#include <Arduino.h>
#include <math.h>
#include <Wire.h>
#include <stdint.h>
#include <Arduino_LSM6DS3.h>

#include "SimpleGET.h"
#include "OdometerData.h"
#include "Ringbuffer.h"


constexpr unsigned long READ_INTERVAL_MS = 0;     // Interval between readings


data sensorData;


int16_t fixedAccelX, fixedAccelY, fixedAccelZ;
int16_t fixedGyroX, fixedGyroY, fixedGyroZ;
int16_t filtered_data_accel_x, filtered_data_velocity_x, filtered_data_pos_x;
uint16_t banana = 0;

 //Ringbuffer defined in "ringbuffer.h"
struct common_buffer_data Struct_Accel_X  = initialize_buffer();

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

    bool accelAvailable = IMU.accelerationAvailable();
    bool gyroAvailable = IMU.gyroscopeAvailable();

    if (accelAvailable) {
      IMU.readAcceleration(fixedAccelX, fixedAccelY, fixedAccelZ);

    
      // Aktualisierung des Ringpuffers 
      push_data_to_buffer(fixedAccelX, &Struct_Accel_X);

      // Auslesen der Filterwerte
      filtered_data_accel_x = moving_average(&Struct_Accel_X);
     // Serial.println(filtered_data_accel_x);

    }

    if (gyroAvailable) {
      IMU.readGyroscope(fixedGyroX, fixedGyroY, fixedGyroZ);

      // Evtl. auch hier Aktualierung des Ringpuffers für die Gyro-Werte
    }

  


    sensorData.accel_vec[0] = filtered_data_accel_x;
    sensorData.accel_vec[1] = filtered_data_velocity_x;
    sensorData.accel_vec[2] = filtered_data_pos_x;

    sensorData.speed_vec[0] = filtered_data_accel_x;
    sensorData.speed_vec[1] = filtered_data_velocity_x;
    sensorData.speed_vec[2] = filtered_data_pos_x;
    sensorData.track_section = 1;
    if (banana > 1000) {
      banana = 0;
    }
    banana = banana+1;
    sensorData.pos_lin = banana;

    SUDP_send(sensorData);
  }
  
}
