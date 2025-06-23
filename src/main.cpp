#include <Arduino.h>
#include <math.h>
#include <Wire.h>
#include <stdint.h>
#include <Arduino_LSM6DS3.h>
#include <stdint.h>

#include "SimpleNET.h"
#include "SimpleGET.h"
#include "OdometerData.h"


constexpr float ACCEL_SCALE = 512.0;              // Custom scaling factor for acceleration
constexpr float GYRO_SCALE = 32768.0 / 2000.0;    // Scale gyroscope data from dps to fixed-point
constexpr unsigned long READ_INTERVAL_MS = 20;     // Interval between readings


data sensorData;

int16_t accelX, accelY, accelZ;
int16_t gyroX, gyroY, gyroZ;

int32_t fixedAccelX, fixedAccelY, fixedAccelZ;
int32_t fixedGyroX, fixedGyroY, fixedGyroZ;
uint16_t banana = 0;

unsigned long previousMillis = 0;
uint8_t tracks[] =         {18  ,   19,   22,   23,  25,   30,  31,   34,   38,    3,    8,    9,   12,   14,   15,  28};
uint16_t track_lengths[] = {2197, 1538, 1037, 1019, 806,  250, 691, 1726, 1381, 2790, 2761, 1492, 3733, 1356, 1424, 796};
uint8_t track_index = 0;


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

    
    if (banana >= track_lengths[track_index]) {
      banana = 0;
      track_index++;
      if (track_index >= (sizeof(tracks) / sizeof(tracks[0]))) {
        track_index = 0;
      }
      sensorData.track_section = tracks[track_index];
      Serial.println("Track: " + String(sensorData.track_section));
    }
    sensorData.track_section = tracks[track_index];
    sensorData.pos_lin = banana;
    banana = banana + 30;

    SUDP_send(sensorData);
  }
}
