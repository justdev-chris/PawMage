// pawm-js/pawm.js
// PawMage JavaScript Decoder - Browser + Node.js compatible

(function(root, factory) {
    if (typeof module !== 'undefined' && module.exports) {
        module.exports = factory();
    } else {
        root.PawMage = factory();
    }
}(typeof self !== 'undefined' ? self : this, function() {

    'use strict';

    // ========== MAGIC ==========
    const MAGIC = 0x4D574150; // "PAWM" little-endian
    const VERSION = 1;

    // ========== DECODE ==========
    function decode(buffer) {
        const dv = new DataView(buffer);
        const magic = dv.getUint32(0, true);
        if (magic !== MAGIC) {
            throw new Error('Invalid .pawm file: bad magic');
        }

        let offset = 4;
        const width = dv.getUint32(offset, true); offset += 4;
        const height = dv.getUint32(offset, true); offset += 4;
        const depth = dv.getUint8(offset); offset += 1;
        const metadataLen = dv.getUint32(offset, true); offset += 4;
        const compressedLen = dv.getUint32(offset, true); offset += 4;

        // Read metadata
        const metadataBytes = new Uint8Array(buffer, offset, metadataLen);
        const metadata = new TextDecoder().decode(metadataBytes);
        offset += metadataLen;

        // Decompress pixels
        const compressed = new Uint8Array(buffer, offset, compressedLen);
        const pixels = rleDecompress(compressed);

        const bpp = depth / 8;
        const expectedPixels = width * height * bpp;
        if (pixels.length !== expectedPixels) {
            throw new Error('Pixel data size mismatch');
        }

        return {
            width: width,
            height: height,
            depth: depth,
            metadata: JSON.parse(metadata || '{}'),
            pixels: pixels // Uint8Array
        };
    }

    // ========== RLE DECOMPRESS ==========
    function rleDecompress(compressed) {
        let total = 0;
        for (let i = 0; i < compressed.length; i += 2) {
            if (i + 1 >= compressed.length) break;
            total += compressed[i];
        }

        const output = new Uint8Array(total);
        let outIdx = 0;
        for (let i = 0; i < compressed.length; i += 2) {
            if (i + 1 >= compressed.length) break;
            const count = compressed[i];
            const val = compressed[i + 1];
            for (let j = 0; j < count; j++) {
                output[outIdx++] = val;
            }
        }
        return output;
    }

    // ========== RLE COMPRESS ==========
    function rleCompress(input) {
        const chunks = [];
        let i = 0;
        while (i < input.length) {
            const val = input[i];
            let count = 1;
            while (i + count < input.length && count < 255 && input[i + count] === val) {
                count++;
            }
            chunks.push(count, val);
            i += count;
        }
        return new Uint8Array(chunks);
    }

    // ========== ENCODE ==========
    function encode(image) {
        const bpp = image.depth / 8;
        const pixelBytes = image.width * image.height * bpp;
        const compressed = rleCompress(image.pixels);

        const metadataStr = JSON.stringify(image.metadata || {});
        const metadataBytes = new TextEncoder().encode(metadataStr);

        // Header: 4 + 4+4+1+4+4 = 21 bytes
        const headerSize = 21;
        const totalSize = headerSize + metadataBytes.length + compressed.length;
        const buffer = new ArrayBuffer(totalSize);
        const dv = new DataView(buffer);
        let offset = 0;

        dv.setUint32(offset, MAGIC, true); offset += 4;
        dv.setUint32(offset, image.width, true); offset += 4;
        dv.setUint32(offset, image.height, true); offset += 4;
        dv.setUint8(offset, image.depth); offset += 1;
        dv.setUint32(offset, metadataBytes.length, true); offset += 4;
        dv.setUint32(offset, compressed.length, true); offset += 4;

        // Write metadata
        const metaView = new Uint8Array(buffer, offset, metadataBytes.length);
        metaView.set(metadataBytes);
        offset += metadataBytes.length;

        // Write compressed pixels
        const pixView = new Uint8Array(buffer, offset, compressed.length);
        pixView.set(compressed);

        return buffer;
    }

    // ========== DRAW TO CANVAS ==========
    function drawToCanvas(image, canvas) {
        const ctx = canvas.getContext('2d');
        canvas.width = image.width;
        canvas.height = image.height;

        const bpp = image.depth / 8;
        const imageData = ctx.createImageData(image.width, image.height);
        const data = imageData.data;

        for (let y = 0; y < image.height; y++) {
            for (let x = 0; x < image.width; x++) {
                const srcIdx = (y * image.width + x) * bpp;
                const dstIdx = (y * image.width + x) * 4;

                if (bpp === 4) {
                    // RGBA
                    data[dstIdx] = image.pixels[srcIdx];
                    data[dstIdx + 1] = image.pixels[srcIdx + 1];
                    data[dstIdx + 2] = image.pixels[srcIdx + 2];
                    data[dstIdx + 3] = image.pixels[srcIdx + 3];
                } else if (bpp === 3) {
                    // RGB -> RGBA
                    data[dstIdx] = image.pixels[srcIdx];
                    data[dstIdx + 1] = image.pixels[srcIdx + 1];
                    data[dstIdx + 2] = image.pixels[srcIdx + 2];
                    data[dstIdx + 3] = 255;
                } else if (bpp === 1) {
                    // Grayscale -> RGBA
                    const val = image.pixels[srcIdx];
                    data[dstIdx] = val;
                    data[dstIdx + 1] = val;
                    data[dstIdx + 2] = val;
                    data[dstIdx + 3] = 255;
                }
            }
        }

        ctx.putImageData(imageData, 0, 0);
    }

    // ========== LOAD FROM URL ==========
    async function loadUrl(url) {
        const response = await fetch(url);
        const buffer = await response.arrayBuffer();
        return decode(buffer);
    }

    // ========== LOAD FROM FILE ==========
    function loadFile(file) {
        return new Promise((resolve, reject) => {
            const reader = new FileReader();
            reader.onload = (e) => {
                try {
                    const img = decode(e.target.result);
                    resolve(img);
                } catch (err) {
                    reject(err);
                }
            };
            reader.onerror = reject;
            reader.readAsArrayBuffer(file);
        });
    }

    // ========== PUBLIC API ==========
    return {
        decode: decode,
        encode: encode,
        drawToCanvas: drawToCanvas,
        loadUrl: loadUrl,
        loadFile: loadFile,
        VERSION: VERSION,
        MAGIC: MAGIC
    };

}));
