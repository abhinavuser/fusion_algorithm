# Exposure Fusion for XIAO ESP32-S3 (OV2640)

This project demonstrates exposure fusion on a Seeed XIAO ESP32-S3 using an OV2640 camera. It fuses three images (different exposures) into one well-exposed image using a per-pixel exposure-weighted algorithm.

Table of Contents
- [Introduction](#introduction)
- [Features](#features)
- [Requirements](#requirements)
- [Installation](#installation)
	- [PlatformIO Setup](#platformio-setup)
	- [Arduino IDE Setup](#arduino-ide-setup)
- [Usage](#usage)
	- [Convert Images to RGB565](#convert-images-to-rgb565)
	- [Upload SPIFFS Data](#upload-spiffs-data)
	- [Upload and Monitor](#upload-and-monitor)
- [Notes](#notes)
- [FAQ](#faq)
- [Contributing](#contributing)

---

## Introduction

Two main examples are included:

1. SPIFFS-only demo — reads three pre-captured RGB565 images from SPIFFS and fuses them.
2. Camera Fusion demo — captures three exposures with the OV2640, fuses them, and serves the result over HTTP.

---

## Features

- Exposure fusion from three input images
- SPIFFS-based demo for offline testing
- Camera capture + fusion + lightweight HTTP server
- Low-memory row-by-row fusion suitable for XIAO without PSRAM

---

## Requirements

- Hardware: XIAO ESP32-S3, OV2640 camera module (or compatible)
- Software: PlatformIO or Arduino IDE, Python 3.x
- Python libs: Pillow (pip install Pillow)

---

## Installation

### PlatformIO Setup

1. Install PlatformIO (if needed):

```powershell
python -m pip install --user platformio
```

2. In VS Code, install the PlatformIO IDE extension and open this folder.

3. Upload SPIFFS data (from the example folder):

```powershell
cd examples\XIAO_S3_ExposureFusion
platformio run -e esp32s3 --target uploadfs
```

4. Upload sketch and monitor:

```powershell
platformio run -e esp32s3 --target upload
platformio device monitor -e esp32s3 -b 115200
```

### Arduino IDE Setup

1. Add ESP32 boards to Boards Manager via:

```
https://dl.espressif.com/dl/package_esp32_index.json
```

2. Select your XIAO ESP32-S3 board and use the ESP32 Filesystem Uploader plugin to upload SPIFFS files.

---

## Usage

### Convert Images to RGB565

Convert source images to raw RGB565 (default 160x120):

```powershell
python tools/to_rgb565.py under.jpg examples\XIAO_S3_ExposureFusion\data\img1.rgb565 --width 160 --height 120
python tools/to_rgb565.py normal.jpg examples\XIAO_S3_ExposureFusion\data\img2.rgb565 --width 160 --height 120
python tools/to_rgb565.py over.jpg examples\XIAO_S3_ExposureFusion\data\img3.rgb565 --width 160 --height 120
```

### Upload SPIFFS Data

From the example folder:

```powershell
platformio run -e esp32s3 --target uploadfs
```

### Upload and Monitor

Upload the sketch and open the serial monitor at 115200 to view progress and results.

If using `Camera_Fusion.ino`, after a successful run the board starts an AP named `XIAO_Fusion` (password `12345678`). Open `http://192.168.4.1/` to download the fused image or run the included PowerShell helper `tools/download_fused.ps1`.

---

## Notes

- Examples use small resolutions to fit in RAM without PSRAM. If your board has PSRAM, enable it in `platformio.ini` and raise the resolution.
- If camera initialization fails in `Camera_Fusion.ino`, save or paste the serial output here and a working candidate mapping can be added.

---

## FAQ

Q: Where do I get the raw files?
A: Use `tools/to_rgb565.py` to convert PNG/JPG to raw RGB565 and put them in `examples/XIAO_S3_ExposureFusion/data/`.

Q: How do I view the fused RGB565 file?
A: Use `tools/rgb565_to_png.py` to convert the raw file back to PNG.

---

## Contributing

Contributions are welcome. If you provide a confirmed camera pin mapping for a specific XIAO S3 Sense board revision I will add it to `Camera_Fusion.ino` and document it here.

---

If you want further trimming or a one-line quickstart, tell me exactly which lines to remove and I'll produce the final tiny README.
