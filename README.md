# Exposure Fusion for XIAO ESP32-S3 (OV2640)

This project demonstrates exposure fusion on a Seeed XIAO ESP32-S3 using an OV2640 camera. It fuses three images (different exposures) into one well-exposed image using a per-pixel exposure-weighted algorithm.
1. SPIFFS-only demo — reads three pre-captured RGB565 images from SPIFFS and fuses them.
- Location: `examples/XIAO_S3_ExposureFusion/XIAO_S3_ExposureFusion.ino`
- Quick run:
	- Convert images:
		`python tools/to_rgb565.py under.jpg examples\XIAO_S3_ExposureFusion\data\img1.rgb565 --width 160 --height 120`
	- Upload SPIFFS (from the example folder):
		`cd examples\XIAO_S3_ExposureFusion` then `platformio run -e esp32s3 --target uploadfs`
	- Upload sketch: `platformio run -e esp32s3 --target upload`
2. Camera Fusion demo — captures three exposures with the OV2640, fuses them, and serves the result over HTTP.
- Location: `examples/XIAO_S3_ExposureFusion/Camera_Fusion.ino`
- Quick run:
	- Upload sketch: `platformio run -e esp32s3 --target upload`
	- Monitor serial: `platformio device monitor -e esp32s3 -b 115200`
	- After AP starts, download fused image from `http://192.168.4.1/` or run `.\tools\download_fused.ps1 -Out fused_download.rgb565`
3. Matlab exposure-fusion (example)
- Location: `examples/exposure_algorithm/exposure-fusion-shadow-removal-main/exposure-fusion-master/`
- Quick run: open `example.m` in Matlab and run it. The `house/` subfolder contains sample images used by the example.
4. Auto-Exposure Fusion for Shadow Removal 
- Location: `examples/exposure_algorithm/exposure-fusion-shadow-removal-main/`
- Quick steps to reproduce basics:
	- Prepare dataset folders (ISTD/ISTD+/SRD) as described in the original project. For ISTD, place shadow images in `train_A`, masks in `train_B`, and shadow-free images in `train_C_fixed_official`.
	- Run the exposure parameter generator: `data_processing/compute_params.ipynb` (or the equivalent script) to generate `train_params_fixed`.
	- Generate test masks with `data_processing/test_mask_generation.py` if needed.
	- Use `OE_train.sh` to train and `OE_eval.sh` to evaluate (adjust paths and parameters inside the scripts as required).
	- Pretrained models are linked in the original project; set the `model` parameter in `OE_eval.sh` and the training params (`n`, `ks`, `rks`) to match the model you want to load.

---

## Project layout

- `examples/` — STM32/ESP examples (including `XIAO_S3_ExposureFusion` with both SPIFFS and camera variants).
- `src/` — SensorFusion library (Madgwick / Mahony IMU fusion implementations) used for inertial sensor fusion.
- Root `.pde` files and `extras/` — Processing sketches (snake / neural-net genetic algorithm visualization) and related assets; a separate desktop project to implemet the fusion algorithm.
- `tools/` — helper scripts (image converters, downloader).
- `platformio.ini`, `library.properties`, `README.md` — project metadata and build config.
- A Matlab implementation of Tom Mertens' classic "Exposure Fusion" (Mertens et al., 2007/2009). See `examples/exposure_algorithm/exposure-fusion-shadow-removal-main/exposure-fusion-master/` for the original Matlab code and example images.
- A shadow removal of an exposure-fusion problem (paper: "Auto-Exposure Fusion for Single-Image Shadow Removal"). The project, datasets, and training/evaluation scripts are under `examples/exposure_algorithm/exposure-fusion-shadow-removal-main/`.

---

## Requirements

- Hardware: XIAO ESP32-S3, OV2640 camera module, STM32F303K8 MCU
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

## How to Contribute

1. Fork the repo and create your branch.
2. Write clear, well-documented code.
3. Add/modify tests as needed.
4. Open a Pull Request with a detailed description.

---

