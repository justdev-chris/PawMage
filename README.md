# 🐱 PawMage

**Simple. Fast. Detailed.**  
A lossless image format (`.pawm`) with metadata — and tools to use it everywhere.

---

## The Format

| Feature | Description |
| :--- | :--- |
| **Magic** | `PAWM` (4 bytes) |
| **Header** | Width, Height, Depth (1/8/24/32 bpp), Metadata length, Compressed size |
| **Metadata** | JSON string (author, description, timestamp, custom) |
| **Compression** | RLE (lossless, byte-level) |
| **Pixels** | Raw, row-major, no padding |

---

## Repository Structure

```

pawmage/
├── libpawm/           # C core library
├── pawm-convert/      # CLI converter (C + stb_image)
├── pawm-view/         # Python GUI viewer (ImGui + PyInstaller)
├── pawm-js/           # JavaScript web decoder
├── installers/        # Windows installer script
├── examples/          # Usage examples
├── tests/             # Test files
├── index.html         # Landing page + live demo
├── README.md
└── LICENSE

```

---

## Build

```bash
# Build everything
make

# Build only libpawm
make -C libpawm

# Build only converter
make -C pawm-convert
```

---

Usage

CLI (pawm-convert)

```bash
# Convert to .pawm
pawm-convert image.png output.pawm
pawm-convert photo.jpg output.pawm

# Convert from .pawm
pawm-convert image.pawm output.png
pawm-convert image.pawm output.jpg

# Inspect metadata
pawm-convert --info image.pawm

# Edit metadata
pawm-convert --set-meta image.pawm author=Chris output.pawm
```

C (libpawm)

```c
#include <pawm.h>

PawMageImage* img = pawm_load("cat.pawm");
uint32_t pixel = pawm_get_pixel(img, 10, 20);
pawm_save("output.pawm", img);
pawm_free(img);
```

Python (GUI Viewer)

```bash
pip install -r pawm-view/requirements.txt
python pawm-view/viewer.py image.pawm
```

JavaScript (Web)

```html
<script src="pawm.min.js"></script>
<script>
  const img = await PawMage.loadUrl('cat.pawm');
  PawMage.drawToCanvas(img, document.getElementById('canvas'));
</script>
```

---

Installers

OS Format Command / Action
Windows .exe Double-click PawMage-Setup.exe
Linux .deb / .rpm sudo dpkg -i pawmage.deb or sudo rpm -i pawmage.rpm
macOS .pkg Double-click or sudo installer -pkg pawmage.pkg -target /

---

API Overview

C Library

Function Description
pawm_load(path) Load .pawm from file
pawm_load_from_memory(data, size) Load from memory
pawm_save(path, img) Save to file
pawm_free(img) Free image
pawm_get_pixel(img, x, y) Get pixel (0xRRGGBBAA)
pawm_set_pixel(img, x, y, color) Set pixel
pawm_metadata_get(metadata, key) Get metadata value
pawm_metadata_set(metadata, key, value) Set metadata value

JavaScript

Function Description
PawMage.decode(buffer) Decode .pawm from ArrayBuffer
PawMage.encode(image) Encode to ArrayBuffer
PawMage.drawToCanvas(image, canvas) Render to canvas
PawMage.loadUrl(url) Load from URL
PawMage.loadFile(file) Load from File object

---

License

MIT — use it, remix it, build with it.

---

Author

Chris (justchrisdevmeow) — code by day, exist by night. 🐱

