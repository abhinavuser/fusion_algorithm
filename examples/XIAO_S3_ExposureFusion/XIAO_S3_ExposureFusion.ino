/*
  XIAO S3 Exposure Fusion (SPIFFS-based demo)

  This sketch reads three raw RGB565 images from SPIFFS named:
    /img1.rgb565
    /img2.rgb565
    /img3.rgb565

  Each image must be WIDTH x HEIGHT, 2 bytes per pixel (RGB565). The sketch performs
  a simple per-pixel weighted fusion using a well-exposedness weight and writes
  /fused.rgb565 back to SPIFFS.

  Purpose: demonstration-only. Replace SPIFFS reads with camera captures when you have a camera.
*/

#include <SPIFFS.h>

// Image geometry - choose small values so it fits in XIAO memory
#define WIDTH 160
#define HEIGHT 120
#define PIXELS (WIDTH * HEIGHT)
#define IMG_SIZE (PIXELS * 2) // RGB565

// Filenames in SPIFFS
const char* IMG1 = "/img1.rgb565";
const char* IMG2 = "/img2.rgb565";
const char* IMG3 = "/img3.rgb565";
const char* OUT = "/fused.rgb565";

// Small helper: compute "well-exposedness" weight per pixel from RGB565
// We'll convert RGB565 -> [0..255] RGB and compute weight = exp(-((L-0.5)^2)/(2*sigma^2))
// Use integer-friendly approximation: L in 0..255, sigma ~ 0.25 -> mapped accordingly

float well_exposedness(uint8_t r, uint8_t g, uint8_t b){
  float rf = r / 255.0f;
  float gf = g / 255.0f;
  float bf = b / 255.0f;
  float L = (rf + gf + bf) / 3.0f; // luminance proxy
  float sigma = 0.2f;
  float x = (L - 0.5f);
  return expf(-(x*x)/(2.0f*sigma*sigma));
}

void setup(){
  Serial.begin(115200);
  while(!Serial){}
  Serial.println("XIAO S3 Exposure Fusion demo starting...");

  if(!SPIFFS.begin(true)){
    Serial.println("SPIFFS mount failed");
    return;
  }

  // Quick file size checks
  File f1 = SPIFFS.open(IMG1, "r");
  File f2 = SPIFFS.open(IMG2, "r");
  File f3 = SPIFFS.open(IMG3, "r");
  if(!f1 || !f2 || !f3){
    Serial.println("Missing image files in SPIFFS. Put img1.rgb565, img2.rgb565 and img3.rgb565 in SPIFFS data folder and upload.");
    return;
  }
  if(f1.size() != IMG_SIZE || f2.size() != IMG_SIZE || f3.size() != IMG_SIZE){
    Serial.println("Image size mismatch. Ensure images are WIDTH x HEIGHT RGB565 raw files.");
    Serial.printf("Expected %d bytes. Found: %d, %d, %d\n", IMG_SIZE, (int)f1.size(), (int)f2.size(), (int)f3.size());
    return;
  }

  Serial.println("Files OK — starting fusion (row-by-row to limit RAM)...");

  // Open output file for writing
  File out = SPIFFS.open(OUT, "w");
  if(!out){
    Serial.println("Unable to open output file for writing");
    return;
  }

  // Process row-by-row: read one row from each input and write output row
  const size_t ROW_BYTES = WIDTH * 2; // RGB565
  uint8_t row1[ROW_BYTES];
  uint8_t row2[ROW_BYTES];
  uint8_t row3[ROW_BYTES];
  uint8_t outrow[ROW_BYTES];

  // Rewind
  f1.seek(0);
  f2.seek(0);
  f3.seek(0);

  for(int y=0; y<HEIGHT; y++){
    size_t r = f1.read(row1, ROW_BYTES);
    f2.read(row2, ROW_BYTES);
    f3.read(row3, ROW_BYTES);
    if(r != ROW_BYTES){
      Serial.println("Read error");
      break;
    }

    // Process each pixel in row
    for(int x=0; x<WIDTH; x++){
      // read RGB565 little-endian (low, high)
      int idx = x*2;
      uint16_t p1 = (uint16_t)row1[idx] | ((uint16_t)row1[idx+1] << 8);
      uint16_t p2 = (uint16_t)row2[idx] | ((uint16_t)row2[idx+1] << 8);
      uint16_t p3 = (uint16_t)row3[idx] | ((uint16_t)row3[idx+1] << 8);

      // expand to 8-bit per channel
      uint8_t r1 = ((p1 >> 11) & 0x1F) << 3;
      uint8_t g1 = ((p1 >> 5) & 0x3F) << 2;
      uint8_t b1 = (p1 & 0x1F) << 3;

      uint8_t r2 = ((p2 >> 11) & 0x1F) << 3;
      uint8_t g2 = ((p2 >> 5) & 0x3F) << 2;
      uint8_t b2 = (p2 & 0x1F) << 3;

      uint8_t r3 = ((p3 >> 11) & 0x1F) << 3;
      uint8_t g3 = ((p3 >> 5) & 0x3F) << 2;
      uint8_t b3 = (p3 & 0x1F) << 3;

      // compute well-exposedness weights
      float w1 = well_exposedness(r1, g1, b1);
      float w2 = well_exposedness(r2, g2, b2);
      float w3 = well_exposedness(r3, g3, b3);
      float wsum = w1 + w2 + w3 + 1e-9f;

      // normalized weighted average (float intermediate, small row size so it's OK)
      float rf = (r1 * w1 + r2 * w2 + r3 * w3) / wsum;
      float gf = (g1 * w1 + g2 * w2 + g3 * w3) / wsum;
      float bf = (b1 * w1 + b2 * w2 + b3 * w3) / wsum;

      // convert back to RGB565
      uint8_t rr = (uint8_t)rf;
      uint8_t gg = (uint8_t)gf;
      uint8_t bb = (uint8_t)bf;
      uint16_t outpix = ((rr >> 3) << 11) | ((gg >> 2) << 5) | (bb >> 3);
      outrow[idx] = outpix & 0xFF;
      outrow[idx+1] = (outpix >> 8) & 0xFF;
    }

    // write output row
    out.write(outrow, ROW_BYTES);
    if((y & 15) == 0) Serial.printf("Processed row %d/%d\n", y, HEIGHT);
  }

  Serial.println("Fusion completed — closing files.");
  f1.close(); f2.close(); f3.close(); out.close();
  Serial.printf("Wrote %s (raw RGB565 %dx%d)\n", OUT, WIDTH, HEIGHT);
}

void loop(){
  // nothing else to do
  delay(10000);
}
