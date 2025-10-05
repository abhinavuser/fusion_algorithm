/*
  XIAO ESP32-S3 Sense - OV2640 camera capture + simple exposure fusion

  Note: This sketch depends on the ESP32 Camera driver being available for ESP32-S3.
  Many ESP32-CAM examples use the esp32/arduino-esp32 camera driver (camera.h). Support
  for S3 variants and Seeed XIAO Sense should be verified with your board package.

  This example captures 3 frames with different exposure/gain, stores them into
  SPIFFS (raw RGB565), runs the same per-pixel fusion as XIAO_S3_ExposureFusion.ino,
  and saves the fused image back to SPIFFS as fused.rgb565.

  BEFORE USING:
  - Ensure your board package supports the camera.h API for ESP32-S3.
  - Check the pin mapping for the XIAO S3 Sense OV2640 (below). If uncertain, provide exact board/pin info.
  - This sketch uses SPIFFS; image sizes must be small to fit RAM.
*/

#include "SPIFFS.h"
#include "esp_camera.h"

// Default image size used for capture & fusion (keep small)
#define WIDTH 160
#define HEIGHT 120

// Filenames
const char* IMG1 = "/cam1.rgb565";
const char* IMG2 = "/cam2.rgb565";
const char* IMG3 = "/cam3.rgb565";
const char* OUT  = "/fused_cam.rgb565";

// Pin configuration for OV2640 on XIAO S3 Sense - THIS MUST BE VERIFIED
// The Seeed XIAO ESP32S3 Sense pin mapping differs across revisions; set these to the correct IO number
// The mapping below is a placeholder. Replace with correct values if needed.
// We'll try multiple candidate configs for different XIAO/OV2640 board revisions.
// If one succeeds, we'll use it. Each candidate maps typical OV2640 signals to GPIO numbers.
struct CamCandidate { camera_config_t cfg; const char* name; };

// Helper macro to create camera_config_t with required fields only
#define BASE_CFG {.xclk_freq_hz = 20000000, .ledc_timer = LEDC_TIMER_0, .ledc_channel = LEDC_CHANNEL_0, .pixel_format = PIXFORMAT_RGB565, .frame_size = FRAMESIZE_QQVGA, .jpeg_quality = 12, .fb_count = 1 }

// Candidate configurations (common mappings). These may need tweaking per board revision.
static CamCandidate candidates[] = {
  // Candidate 0: common OV2640 on many ESP32-S3 modules (example mapping)
  { { .pin_pwdn = -1, .pin_reset = -1, .pin_xclk = 21, .pin_sccb_sda = 26, .pin_sccb_scl = 27, .pin_d7 = 35, .pin_d6 = 34, .pin_d5 = 39, .pin_d4 = 36, .pin_d3 = 19, .pin_d2 = 18, .pin_d1 = 5, .pin_d0 = 4, .pin_vsync = 25, .pin_href = 23, .pin_pclk = 22, BASE_CFG }, "candidate-0" },
  // Candidate 1: alternate mapping used by some Seeed boards
  { { .pin_pwdn = -1, .pin_reset = -1, .pin_xclk = 4, .pin_sccb_sda = 15, .pin_sccb_scl = 14, .pin_d7 = 47, .pin_d6 = 46, .pin_d5 = 45, .pin_d4 = 44, .pin_d3 = 43, .pin_d2 = 42, .pin_d1 = 41, .pin_d0 = 40, .pin_vsync = 39, .pin_href = 38, .pin_pclk = 37, BASE_CFG }, "candidate-1" },
  // Candidate 2: mapping similar to ESP32-CAM modules (for testing)
  { { .pin_pwdn = 32, .pin_reset = -1, .pin_xclk = 0, .pin_sccb_sda = 26, .pin_sccb_scl = 27, .pin_d7 = 35, .pin_d6 = 34, .pin_d5 = 39, .pin_d4 = 36, .pin_d3 = 21, .pin_d2 = 19, .pin_d1 = 18, .pin_d0 = 5, .pin_vsync = 25, .pin_href = 23, .pin_pclk = 22, BASE_CFG }, "candidate-2" }
};
static const int CAND_COUNT = sizeof(candidates)/sizeof(candidates[0]);

float well_exposedness(uint8_t r, uint8_t g, uint8_t b){
  float rf = r / 255.0f;
  float gf = g / 255.0f;
  float bf = b / 255.0f;
  float L = (rf + gf + bf) / 3.0f;
  float sigma = 0.2f;
  float x = (L - 0.5f);
  return expf(-(x*x)/(2.0f*sigma*sigma));
}

void setup(){
  Serial.begin(115200);
  while(!Serial){}
  Serial.println("Camera fusion starting...");

  if(!SPIFFS.begin(true)){
    Serial.println("SPIFFS mount failed");
    return;
  }

  // Try autodetecting the right camera pin mapping from candidate list
  int found = -1;
  for(int i=0;i<CAND_COUNT;i++){
    Serial.printf("Trying camera mapping %s (%d/%d)...\n", candidates[i].name, i+1, CAND_COUNT);
    esp_err_t res = esp_camera_init(&candidates[i].cfg);
    if(res == ESP_OK){
      Serial.printf("Camera init succeeded with %s\n", candidates[i].name);
      found = i;
      break;
    } else {
      Serial.printf("Camera init failed (0x%X) for %s\n", res, candidates[i].name);
      esp_camera_deinit();
    }
  }
  if(found < 0){
    Serial.println("Camera autodetect failed - try supplying pin mapping or add more candidates");
    return;
  }

  // Capture three exposures by adjusting sensor settings
  // NOTE: exact sensor register controls depend on the driver. We attempt to use sensor API.
  sensor_t * s = esp_camera_sensor_get();
  if(!s){ Serial.println("Failed to get sensor handle"); }

  // Exposure/gain adjustments: try to reduce exposure, normal, increase
  // Many drivers expose set_gain_ctrl and set_exposure_ctrl; check your sensor_t implementation.

  // Capture 1: darker
  if(s && s->set_gain_ctrl) s->set_gain_ctrl(s, 0); // attempt lower gain
  if(s && s->set_exposure_ctrl) s->set_exposure_ctrl(s, 0);
  capture_and_save(IMG1);

  delay(200);
  // Capture 2: normal (auto)
  if(s && s->set_gain_ctrl) s->set_gain_ctrl(s, 1);
  if(s && s->set_exposure_ctrl) s->set_exposure_ctrl(s, 1);
  capture_and_save(IMG2);

  delay(200);
  // Capture 3: brighter
  if(s && s->set_gain_ctrl) s->set_gain_ctrl(s, 2);
  if(s && s->set_exposure_ctrl) s->set_exposure_ctrl(s, 2);
  capture_and_save(IMG3);

  // Run fusion (we'll reuse the same per-pixel row-by-row approach)
  if(!fuse_from_files(IMG1, IMG2, IMG3, OUT, WIDTH, HEIGHT)){
    Serial.println("Fusion failed");
  } else {
    Serial.println("Fusion completed: ");
    Serial.println(OUT);
  }

}

void loop(){
  delay(10000);
}

bool capture_and_save(const char* filename){
  camera_fb_t * fb = esp_camera_fb_get();
  if(!fb){
    Serial.println("Frame buffer error");
    return false;
  }

  // If fb is in JPEG, convert is needed. We requested RGB565 frame buffer via config.
  if(fb->format != PIXFORMAT_RGB565){
    Serial.println("Frame not RGB565 — driver/framebuffer format mismatch");
    esp_camera_fb_return(fb);
    return false;
  }

  File f = SPIFFS.open(filename, FILE_WRITE);
  if(!f){ Serial.println("Cannot open file to write"); esp_camera_fb_return(fb); return false; }
  f.write(fb->buf, fb->len);
  f.close();
  esp_camera_fb_return(fb);
  Serial.printf("Saved %s (%d bytes)\n", filename, (int)fb->len);
  return true;
}

bool fuse_from_files(const char* f1name, const char* f2name, const char* f3name, const char* outname, int W, int H){
  File f1 = SPIFFS.open(f1name, "r");
  File f2 = SPIFFS.open(f2name, "r");
  File f3 = SPIFFS.open(f3name, "r");
  if(!f1 || !f2 || !f3) return false;
  if(f1.size() != W*H*2 || f2.size() != W*H*2 || f3.size() != W*H*2) return false;

  File out = SPIFFS.open(outname, FILE_WRITE);
  if(!out) return false;

  const size_t ROW_BYTES = W*2;
  uint8_t row1[ROW_BYTES];
  uint8_t row2[ROW_BYTES];
  uint8_t row3[ROW_BYTES];
  uint8_t outrow[ROW_BYTES];

  f1.seek(0); f2.seek(0); f3.seek(0);
  for(int y=0; y<H; y++){
    f1.read(row1, ROW_BYTES);
    f2.read(row2, ROW_BYTES);
    f3.read(row3, ROW_BYTES);
    for(int x=0; x<W; x++){
      int idx = x*2;
      uint16_t p1 = (uint16_t)row1[idx] | ((uint16_t)row1[idx+1] << 8);
      uint16_t p2 = (uint16_t)row2[idx] | ((uint16_t)row2[idx+1] << 8);
      uint16_t p3 = (uint16_t)row3[idx] | ((uint16_t)row3[idx+1] << 8);
      uint8_t r1 = ((p1 >> 11) & 0x1F) << 3;
      uint8_t g1 = ((p1 >> 5) & 0x3F) << 2;
      uint8_t b1 = (p1 & 0x1F) << 3;
      uint8_t r2 = ((p2 >> 11) & 0x1F) << 3;
      uint8_t g2 = ((p2 >> 5) & 0x3F) << 2;
      uint8_t b2 = (p2 & 0x1F) << 3;
      uint8_t r3 = ((p3 >> 11) & 0x1F) << 3;
      uint8_t g3 = ((p3 >> 5) & 0x3F) << 2;
      uint8_t b3 = (p3 & 0x1F) << 3;
      float w1 = well_exposedness(r1,g1,b1);
      float w2 = well_exposedness(r2,g2,b2);
      float w3 = well_exposedness(r3,g3,b3);
      float wsum = w1 + w2 + w3 + 1e-9f;
      float rf = (r1*w1 + r2*w2 + r3*w3) / wsum;
      float gf = (g1*w1 + g2*w2 + g3*w3) / wsum;
      float bf = (b1*w1 + b2*w2 + b3*w3) / wsum;
      uint8_t rr = (uint8_t)rf; uint8_t gg = (uint8_t)gf; uint8_t bb = (uint8_t)bf;
      uint16_t outpix = ((rr >> 3) << 11) | ((gg >> 2) << 5) | (bb >> 3);
      outrow[idx] = outpix & 0xFF; outrow[idx+1] = (outpix>>8)&0xFF;
    }
    out.write(outrow, ROW_BYTES);
  }
  out.close(); f1.close(); f2.close(); f3.close();
  return true;
}
