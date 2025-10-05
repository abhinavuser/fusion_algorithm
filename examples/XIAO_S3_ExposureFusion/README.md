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
