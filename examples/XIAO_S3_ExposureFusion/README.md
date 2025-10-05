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
- This is a simple per-pixel fusion (well-exposedness weight only) intended as a starting point. For higher quality, multi-scale pyramid blending is recommended, but requires more RAM/compute.
- Camera capture is not implemented here. Replace SPIFFS reads with camera capture code for live capture once you supply camera details.
