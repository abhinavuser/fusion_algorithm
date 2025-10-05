# exposure-fusion for XIAO ESP32-S3

Minimal example code to perform a small exposure-fusion on a Seeed XIAO ESP32-S3 (OV2640).

What this repo provides
- `examples/XIAO_S3_ExposureFusion/XIAO_S3_ExposureFusion.ino` — SPIFFS-only demo: reads three raw RGB565 images from SPIFFS (`/img1.rgb565`, `/img2.rgb565`, `/img3.rgb565`), fuses them using a per-pixel well‑exposedness weight (row-by-row to save RAM), and writes `/fused.rgb565`.
- `examples/XIAO_S3_ExposureFusion/Camera_Fusion.ino` — optional: attempts to autodetect OV2640 pin mappings, capture three exposures, fuse them, and serve the result over a tiny HTTP server.
- `tools/to_rgb565.py` and `tools/rgb565_to_png.py` — helpers to convert between PNG/JPG and raw RGB565.
- `tools/download_fused.ps1` — PowerShell helper to download the fused result from the device AP.
- `platformio.ini` — basic PlatformIO environment for ESP32-S3 (Arduino framework).

Quick usage (concise)
1. Convert three images to raw RGB565 (default 160x120):
```powershell
python tools/to_rgb565.py under.jpg examples\XIAO_S3_ExposureFusion\data\img1.rgb565 --width 160 --height 120
python tools/to_rgb565.py normal.jpg examples\XIAO_S3_ExposureFusion\data\img2.rgb565 --width 160 --height 120
python tools/to_rgb565.py over.jpg examples\XIAO_S3_ExposureFusion\data\img3.rgb565 --width 160 --height 120
```
2. Upload SPIFFS data (PlatformIO recommended):
```powershell
cd examples\XIAO_S3_ExposureFusion
platformio run -e esp32s3 --target uploadfs
```
3. Upload sketch and view serial output:
```powershell
platformio run -e esp32s3 --target upload
platformio device monitor -e esp32s3 -b 115200
```
4. If you're using `Camera_Fusion.ino`: after a successful run the board starts an AP `XIAO_Fusion` (password `12345678`). Visit `http://192.168.4.1/` or use the PowerShell helper to download `/fused_cam.rgb565`.

Notes
- The examples target small resolutions to fit RAM on XIAO without PSRAM. If your board has PSRAM, enable it via `platformio.ini` and increase resolution.
- If you see camera init failures in `Camera_Fusion.ino`, provide the serial output and I can add or adjust candidate mappings.

That's it — the README is intentionally minimal. If you want me to add back a longer explanation or a web UI, tell me which part to expand.

If PlatformIO is not installed on your machine
---------------------------------------------
If running `platformio` failed ("command not found"), you have two easy options:

- Use the PlatformIO extension for Visual Studio Code (recommended): open VS Code, install the "PlatformIO IDE" extension, then open this folder and the PlatformIO toolbar will provide build/upload/uploadfs actions.
- Or install PlatformIO CLI directly (Windows PowerShell):

```powershell
python -m pip install --user platformio
# then add %USERPROFILE%\AppData\Roaming\Python\Python3X\Scripts to your PATH or run platformio from that folder
```

If you don't want to install PlatformIO, you can still test the SPIFFS fusion workflow by:

1. Using the `tools/to_rgb565.py` script to convert images on your PC to raw `.rgb565` files.
2. Place the three files (`img1.rgb565`, `img2.rgb565`, `img3.rgb565`) inside the `examples/XIAO_S3_ExposureFusion/data/` folder and upload via the Arduino ESP32 Filesystem Uploader plugin in Arduino IDE (or use PlatformIO `uploadfs` later).

Both the SPIFFS-only demo (`XIAO_S3_ExposureFusion.ino`) and the camera capture variant (`Camera_Fusion.ino`) are present in `examples/XIAO_S3_ExposureFusion/`. The example-level README there has quick per-example notes.
