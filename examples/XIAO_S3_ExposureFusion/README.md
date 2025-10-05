# XIAO_S3_ExposureFusion example

This folder contains two sketches demonstrating a minimal exposure-fusion flow for the Seeed XIAO ESP32-S3 Sense.

Files
- `XIAO_S3_ExposureFusion.ino` — SPIFFS-based demo: reads `/img1.rgb565`, `/img2.rgb565`, `/img3.rgb565` from SPIFFS (in `data/`), performs a per-pixel weighted fusion row-by-row, and writes `/fused.rgb565` back to SPIFFS.
- `Camera_Fusion.ino` — attempts to autodetect OV2640 camera pin mappings, capture three exposures (via sensor gain/exposure changes), run the same fusion, save `/fused_cam.rgb565`, and start a small Wi‑Fi AP + HTTP server so you can download the result.

Quick steps (SPIFFS demo)
1. Convert three images to RGB565 using `tools/to_rgb565.py` (defaults to 160x120). Put them into `examples/XIAO_S3_ExposureFusion/data/` as `img1.rgb565`, `img2.rgb565`, `img3.rgb565`.
2. In PlatformIO: open the example folder, then run `platformio run -e esp32s3 --target uploadfs` to upload SPIFFS.
3. Upload `XIAO_S3_ExposureFusion.ino` to your XIAO and open the serial monitor at 115200. The sketch will report progress and write `/fused.rgb565`.

Quick steps (camera capture + download)
1. Upload `Camera_Fusion.ino` to the board.
2. Open serial monitor to watch autodetect progress. If autodetect finds a working pin mapping, it will save `/camera_map.txt` and start AP `XIAO_Fusion` (password `12345678`).
3. Connect to the AP and go to `http://192.168.4.1/` to download the fused image, or use `tools/download_fused.ps1` after connecting to the AP.

Notes
- Both examples are intentionally small-resolution to reduce RAM usage. If your board has PSRAM, uncomment the `-DBOARD_HAS_PSRAM` build flag in `platformio.ini` and consider increasing resolution.
- Camera autodetect is a pragmatic approach for boards with uncertain pinouts — it's not exhaustive. If autodetect fails, provide the exact pin numbers or serial output and the `camera_map.txt` file will be created on success which we can use for future automated detection.
XIAO S3 Exposure Fusion example

This folder contains a small demo that performs a memory-friendly, on-device exposure fusion on a Seeed XIAO ESP32-S3.

Files
- XIAO_S3_ExposureFusion.ino  -- reads `/img1.rgb565`, `/img2.rgb565`, `/img3.rgb565` from SPIFFS, writes `/fused.rgb565`.

How it works
1. Prepare three small images (WIDTH x HEIGHT, default 160x120) and convert them to raw RGB565 files. Use the Python helper in the top-level README.
2. Put the three `.rgb565` files into the `data/` folder inside this example folder (create it). For PlatformIO, `platformio run --target uploadfs` will upload them to SPIFFS.
3. Upload the sketch to the XIAO S3. Serial output shows progress and final filename.
4. Download `/fused.rgb565` from SPIFFS and convert back to PNG using the inverse Python helper in the top-level README.

Notes
- This is a simple per-pixel fusion (well-exposedness weight only) intended as a starting point. For higher quality, multi-scale pyramid / Laplacian pyramid blending is recommended, but requires more RAM/compute.

Camera capture and S3 specifics
- A camera-enabled variant is provided in `Camera_Fusion.ino`. This attempts to use the ESP32 camera API (`esp_camera.h`) to capture RGB565 frames directly from an OV2640 on the XIAO S3 Sense board and run the same fusion pipeline.
- WARNING: The OV2640 pin mapping on Seeed XIAO ESP32S3 Sense varies by board revision and board package. The `camera_config_t` in `Camera_Fusion.ino` contains placeholder fields. If camera initialization fails, do the following:
	1. Check Seeed's XIAO ESP32S3 Sense documentation for the exact OV2640 pin mapping (SDA, SCL, PCLK, VSYNC, HREF, D0..D7, XCLK, PWDN).
	2. Update `camera_config` in `Camera_Fusion.ino` with the correct GPIO numbers.
	3. Ensure your ESP32-S3 Arduino core / board package includes camera driver support for S3 (some older cores only support classic ESP32).

How to test camera capture only
1. Open `Camera_Fusion.ino` and verify/adjust pin mapping.
2. Upload the sketch to XIAO S3.
3. Open Serial Monitor: you should see messages indicating saved frames `cam1.rgb565`, `cam2.rgb565`, `cam3.rgb565` written to SPIFFS. If frames are JPEG instead of RGB565, you'll see a message and must either request RGB565 frame buffer or decode JPEG frames before applying fusion.

If you share the precise Seeed XIAO ESP32S3 Sense board revision or the OV2640 pin mapping from the Seeed docs, I will update `Camera_Fusion.ino` with the correct `camera_config_t`, test compile, and commit the working config so you can just upload and run.
