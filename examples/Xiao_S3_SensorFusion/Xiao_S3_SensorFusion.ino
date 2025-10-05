#include "SensorFusion.h"
SF fusion;

// This example compiles for ESP32/XIAO S3 boards and demonstrates the API
// without requiring an actual IMU. Replace the simulated sensor code with
// real IMU reads (e.g. MPU9250 library) for a hardware test.

float gx=0, gy=0, gz=0; // gyro in rad/s
float ax=0, ay=0, az=0; // accel in m/s^2
float mx=0, my=0, mz=0; // mag (optional)

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(1); }
  Serial.println("XIAO S3 SensorFusion demo - simulated IMU");
}

unsigned long t0 = 0;

void loop(){
  // Simulate a slow rotation around Z axis
  unsigned long now = millis();
  float seconds = now / 1000.0f;

  // Simulated gyro: rotate 10 deg/s around Z
  const float deg2rad = 3.14159265358979323846f / 180.0f;
  gx = 0.0f;
  gy = 0.0f;
  gz = 10.0f * deg2rad; // 10 deg/s

  // Simulated static accel pointing up (approx gravity)
  ax = 0.0f; ay = 0.0f; az = 9.80665f;

  float deltat = fusion.deltatUpdate();
  fusion.MahonyUpdate(gx, gy, gz, ax, ay, az, deltat);

  float pitch = fusion.getPitch();
  float roll = fusion.getRoll();
  float yaw = fusion.getYaw();

  Serial.print("t="); Serial.print(seconds, 2);
  Serial.print("s \tPitch="); Serial.print(pitch, 2);
  Serial.print(" \tRoll="); Serial.print(roll, 2);
  Serial.print(" \tYaw="); Serial.println(yaw, 2);

  delay(200);
}
