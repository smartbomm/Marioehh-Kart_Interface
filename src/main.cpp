#include <Arduino.h>
#include <math.h>
#include <Wire.h>
#include <stdint.h>
#include <Arduino_LSM6DS3.h>

#include "SimpleNET.h"
#include "SimpleGET.h"
#include "OdometerData.h"


constexpr float ACCEL_SCALE = 512.0;              // Custom scaling factor for acceleration
constexpr float GYRO_SCALE = 32768.0 / 2000.0;    // Scale gyroscope data from dps to fixed-point
constexpr unsigned long READ_INTERVAL_MS = 0;     // Interval between readings


data sensorData;

float accelX, accelY, accelZ;
float gyroX, gyroY, gyroZ;

int32_t fixedAccelX, fixedAccelY, fixedAccelZ;
int32_t fixedGyroX, fixedGyroY, fixedGyroZ;
uint16_t banana = 0;

unsigned long previousMillis = 0;

void setup() {
  Serial.begin(9600);
  uint64_t systemTime = 0;
  while (systemTime == 0) {
    systemTime = bytesToUint64_StringDigits(simpleGET("/t"));
}
  SUDP_beginn(systemTime);

  if (!IMU.begin()) {
    while (true);
  }

}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= READ_INTERVAL_MS) {
    previousMillis = currentMillis;

  //  bool accelAvailable = IMU.accelerationAvailable();
   // bool gyroAvailable = IMU.gyroscopeAvailable();

    if (IMU.readAcceleration(accelX, accelY, accelZ)) {
      


      fixedAccelX = (int32_t)(accelX * ACCEL_SCALE);
      fixedAccelY = (int32_t)(accelY * ACCEL_SCALE);
      fixedAccelZ = (int32_t)(accelZ * ACCEL_SCALE);
    }
/*
    if (gyroAvailable) {
      IMU.readGyroscope(gyroX, gyroY, gyroZ);

      fixedGyroX = (int32_t)(gyroX * GYRO_SCALE);
      fixedGyroY = (int32_t)(gyroY * GYRO_SCALE);
      fixedGyroZ = (int32_t)(gyroZ * GYRO_SCALE);
    }
*/
    sensorData.accel_vec[0] = fixedAccelX;
    sensorData.accel_vec[1] = fixedAccelY;
    sensorData.accel_vec[2] = fixedAccelZ;

    sensorData.gyro_vec[0] = fixedGyroX;
    sensorData.gyro_vec[1] = fixedGyroY;
    sensorData.gyro_vec[2] = fixedGyroZ;
    if (banana > 350) {
      banana = 0;
      sensorData.track_section++;
      if (sensorData.track_section > 3) {
        sensorData.track_section = 1;
      }
      Serial.println("Track: " + String(sensorData.track_section));
    }
    banana = banana + 1;
    sensorData.pos_lin = banana;

    SUDP_send(sensorData);
  }
}
