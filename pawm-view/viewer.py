# pawm-view/viewer.py
import sys
import os
import ctypes
import ctypes.util
from pathlib import Path

try:
    import glfw
    import OpenGL.GL as gl
    import imgui
    from imgui.integrations.glfw import GlfwRenderer
    import numpy as np
except ImportError:
    print("Missing dependencies. Run: pip install glfw PyOpenGL imgui numpy")
    sys.exit(1)

# ========== LOAD LIBPAWM ==========
def load_libpawm():
    # Try to find libpawm
    paths = [
        "../libpawm/libpawm.so",
        "../libpawm/libpawm.dylib",
        "./libpawm.so",
        ctypes.util.find_library("pawm")
    ]
    
    for p in paths:
        if p and Path(p).exists():
            return ctypes.CDLL(p)
    raise FileNotFoundError("libpawm not found. Build it first.")

lib = load_libpawm()

# ========== CTYPES BINDINGS ==========
class PawMageImage(ctypes.Structure):
    _fields_ = [
        ("width", ctypes.c_uint32),
        ("height", ctypes.c_uint32),
        ("depth", ctypes.c_uint8),
        ("metadata_len", ctypes.c_uint32),
        ("metadata", ctypes.POINTER(ctypes.c_char)),
        ("pixels", ctypes.POINTER(ctypes.c_uint8))
    ]

lib.pawm_load.argtypes = [ctypes.c_char_p]
lib.pawm_load.restype = ctypes.POINTER(PawMageImage)

lib.pawm_load_from_memory.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.c_size_t]
lib.pawm_load_from_memory.restype = ctypes.POINTER(PawMageImage)

lib.pawm_save.argtypes = [ctypes.c_char_p, ctypes.POINTER(PawMageImage)]
lib.pawm_save.restype = ctypes.c_int

lib.pawm_free.argtypes = [ctypes.POINTER(PawMageImage)]
lib.pawm_free.restype = None

lib.pawm_get_pixel.argtypes = [ctypes.POINTER(PawMageImage), ctypes.c_uint32, ctypes.c_uint32]
lib.pawm_get_pixel.restype = ctypes.c_uint32

# ========== IMAGE LOADER ==========
def load_pawm(path):
    img_ptr = lib.pawm_load(path.encode())
    if not img_ptr:
        return None
    return img_ptr

def img_to_numpy(img_ptr):
    img = img_ptr.contents
    w, h, d = img.width, img.height, img.depth
    bpp = d // 8
    pixel_bytes = w * h * bpp
    
    # Copy pixel data to numpy array
    pixels = ctypes.string_at(img.pixels, pixel_bytes)
    arr = np.frombuffer(pixels, dtype=np.uint8).reshape((h, w, bpp))
    
    # Convert BGR/RGB based on depth
    if d == 24:
        # BGR -> RGB
        arr = arr[:, :, ::-1]
    elif d == 32:
        # BGRA -> RGBA
        arr = arr[:, :, [2, 1, 0, 3]]
    
    return arr

# ========== MAIN APP ==========
class PawMageViewer:
    def __init__(self):
        self.img_ptr = None
        self.texture_id = None
        self.image_data = None
        self.zoom = 1.0
        self.pan_x = 0
        self.pan_y = 0
        self.show_metadata = True
        self.filename = "No image loaded"
        
        self._init_window()
        self._init_imgui()
        
    def _init_window(self):
        if not glfw.init():
            raise RuntimeError("GLFW init failed")
        
        glfw.window_hint(glfw.RESIZABLE, glfw.TRUE)
        self.window = glfw.create_window(800, 600, "PawMage Viewer", None, None)
        if not self.window:
            glfw.terminate()
            raise RuntimeError("Window creation failed")
        
        glfw.make_context_current(self.window)
        glfw.swap_interval(1)
        
    def _init_imgui(self):
        imgui.create_context()
        self.impl = GlfwRenderer(self.window)
        
    def load_image(self, path):
        self.filename = Path(path).name
        self.img_ptr = load_pawm(path)
        if self.img_ptr:
            self.image_data = img_to_numpy(self.img_ptr)
            self._update_texture()
            self.zoom = 1.0
            self.pan_x = 0
            self.pan_y = 0
            return True
        return False
    
    def _update_texture(self):
        if self.image_data is None:
            return
        
        h, w, c = self.image_data.shape
        if self.texture_id:
            gl.glDeleteTextures(1, [self.texture_id])
        
        self.texture_id = gl.glGenTextures(1)
        gl.glBindTexture(gl.GL_TEXTURE_2D, self.texture_id)
        
        gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MIN_FILTER, gl.GL_LINEAR)
        gl.glTexParameteri(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MAG_FILTER, gl.GL_LINEAR)
        
        fmt = gl.GL_RGBA if c == 4 else gl.GL_RGB
        gl.glTexImage2D(gl.GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, gl.GL_UNSIGNED_BYTE, self.image_data)
    
    def _draw_image(self):
        if not self.texture_id:
            imgui.text("No image loaded. Drag and drop or use File -> Open")
            return
        
        win_w = imgui.get_window_width()
        win_h = imgui.get_window_height()
        
        # Calculate image size with zoom
        h, w = self.image_data.shape[:2]
        aspect = w / h
        disp_w = min(win_w * self.zoom, w * self.zoom)
        disp_h = disp_w / aspect
        
        if disp_h > win_h * self.zoom:
            disp_h = win_h * self.zoom
            disp_w = disp_h * aspect
        
        # Pan offset
        pos_x = (win_w - disp_w) / 2 + self.pan_x
        pos_y = (win_h - disp_h) / 2 + self.pan_y
        
        imgui.image(self.texture_id, disp_w, disp_h)
        
        # Mouse drag to pan
        if imgui.is_mouse_dragging(0):
            dx, dy = imgui.get_mouse_drag_delta()
            self.pan_x += dx
            self.pan_y += dy
            imgui.reset_mouse_drag_delta()
        
        # Scroll to zoom
        scroll = imgui.get_io().mouse_wheel
        if scroll != 0:
            self.zoom *= (1.0 + scroll * 0.1)
            self.zoom = max(0.1, min(10.0, self.zoom))
    
    def _draw_metadata(self):
        if not self.img_ptr:
            return
        
        img = self.img_ptr.contents
        if self.show_metadata:
            imgui.begin("Metadata", True)
            imgui.text(f"File: {self.filename}")
            imgui.text(f"Width: {img.width}  Height: {img.height}")
            imgui.text(f"Depth: {img.depth} bpp")
            imgui.separator()
            imgui.text("Metadata JSON:")
            try:
                meta = img.metadata.decode() if img.metadata else "{}"
                imgui.text_wrapped(meta)
            except:
                imgui.text_wrapped("[invalid metadata]")
            imgui.end()
    
    def _draw_menu(self):
        if imgui.begin_main_menu_bar():
            if imgui.begin_menu("File", True):
                clicked, path = imgui.menu_item("Open", "Ctrl+O")
                if clicked:
                    # Simple file open via dialog would go here
                    # For now, just print
                    print("File -> Open")
                
                imgui.menu_item("Save As", "Ctrl+S")
                imgui.separator()
                clicked, _ = imgui.menu_item("Exit", "Ctrl+Q")
                if clicked:
                    sys.exit(0)
                imgui.end_menu()
            
            if imgui.begin_menu("View", True):
                _, self.show_metadata = imgui.menu_item("Metadata", None, self.show_metadata)
                imgui.end_menu()
            
            imgui.end_main_menu_bar()
    
    def run(self):
        while not glfw.window_should_close(self.window):
            glfw.poll_events()
            self.impl.process_inputs()
            
            imgui.new_frame()
            
            self._draw_menu()
            self._draw_image()
            self._draw_metadata()
            
            imgui.render()
            gl.glClearColor(0.1, 0.1, 0.12, 1.0)
            gl.glClear(gl.GL_COLOR_BUFFER_BIT)
            self.impl.render(imgui.get_draw_data())
            
            glfw.swap_buffers(self.window)
        
        self.impl.shutdown()
        glfw.terminate()

# ========== ENTRY ==========
if __name__ == "__main__":
    viewer = PawMageViewer()
    
    # Load image from command line if provided
    if len(sys.argv) > 1:
        viewer.load_image(sys.argv[1])
    
    viewer.run()
