#include <Arduino.h>
#include <math.h>
#include <Wire.h>
#include <stdint.h>
#include <Arduino_LSM6DS3.h>

#include "SimpleGET.h"
#include "OdometerData.h"
#include "Ringbuffer.h"

// Time intervals
constexpr unsigned long READ_INTERVAL_MS = 0;     // Interval between readings
constexpr unsigned long INTERVAL_STOP_COND = 100;

// Data struct to be sent
data sensorData;

// Incoming data from sensor in 16-bit format
int16_t accelX, accelY, accelZ;
int16_t gyroX, gyroY, gyroZ;

// Other variable definition
int32_t filteredAccelX, filteredAccelY, filteredAccelZ;
int32_t fixedGyroX, fixedGyroY, fixedGyroZ;
uint32_t filtered_data_velocity_x = 0u;
uint32_t filtered_data_accel_x = 0u;
uint64_t filtered_data_pos_x = 0;
int32_t merker_x=0; 
uint32_t merker_velocity_x = 0;
uint32_t buffer_sum_merker = 0;
uint8_t counter_sending = 0u;

 //Ringbuffer defined in "ringbuffer.h"
struct common_buffer_data Struct_Accel_X  = initialize_buffer();
struct common_buffer_data Struct_Accel_Y  = initialize_buffer();
struct common_buffer_data Struct_Accel_Z  = initialize_buffer();

// Timestamps
unsigned long previousMillis = 0;
uint32_t previousMillis_stop_cond = 0;


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
      IMU.readAcceleration(accelX, accelY, accelZ);
      
      if ((accelX > (int32_t)0) & (accelX < (int32_t)ZERO_MOVEMENT)) {
        accelX =0u;
      }


    
      // Aktualisierung des Ringpuffers 
      push_data_to_buffer(accelX, &Struct_Accel_X);
      push_data_to_buffer(accelY, &Struct_Accel_Y);
      push_data_to_buffer(accelZ, &Struct_Accel_Z);
            
      // Auslesen der Filterwerte
      merker_x = filteredAccelX;
      merker_velocity_x = filtered_data_velocity_x;
      filteredAccelX = moving_average(&Struct_Accel_X) ;
      filteredAccelY = moving_average(&Struct_Accel_Y) ;
      filteredAccelZ = moving_average(&Struct_Accel_Z) ;
    
  
       sensorData.gyro_vec[0] = merker_x;       
       sensorData.gyro_vec[1] = integration_32bit(Struct_Accel_X, &filtered_data_velocity_x,filteredAccelX,merker_x);

      /*
      // Stop recognition
      
      if ((buffer_sum_merker <= ZERO_MOVEMENT*RINGBUFFER_SIZE) & (Struct_Accel_X.buffer_sum <= ZERO_MOVEMENT*RINGBUFFER_SIZE)){
        filtered_data_velocity_x = 0u; //Acceleration has been zero for long, therefore the car isn't moving anymore 

      }
      if (currentMillis - previousMillis_stop_cond >= INTERVAL_STOP_COND) {
          previousMillis_stop_cond = currentMillis; // Reset timer
          buffer_sum_merker = Struct_Accel_X.buffer_sum; // Write current Buffer in Merker
      }
*/


      
       sensorData.gyro_vec[2] = integration_64bit(Struct_Accel_X, &filtered_data_pos_x, filtered_data_velocity_x, merker_velocity_x);
      counter_sending++;
    }



    if (gyroAvailable) {
      IMU.readGyroscope(gyroX, gyroY, gyroZ);


      // Evtl. auch hier Aktualierung des Ringpuffers für die Gyro-Werte
    }

  



    if (counter_sending >=20) {
    sensorData.accel_vec[0] = accelX;
    sensorData.accel_vec[1] = accelY;
    sensorData.accel_vec[2] = accelZ;

    sensorData.accel_lin = filteredAccelX;
    sensorData.speed_lin = filtered_data_velocity_x/SPEED_SCALER;
    sensorData.pos_lin = (uint32_t)(filtered_data_pos_x/POSITION_SCALER); // STIMMT DIE BUFFERSIZE????
    sensorData.track_section = 1;
    SUDP_send(sensorData);
    counter_sending = 0u;
    }
  }
  
}
