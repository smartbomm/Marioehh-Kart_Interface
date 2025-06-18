#include <Arduino.h>
#include <math.h>
#include <Wire.h>
#include <stdint.h>
#include <Arduino_LSM6DS3.h>

#include "SimpleGET.h"
#include "OdometerData.h"
#include "ringbuffer.h"

#include "vectors.h"

// Time intervals
constexpr unsigned long READ_INTERVAL_MS = 1;     // Interval between readings
constexpr unsigned long INTERVAL_STOP_COND = 100;

// Data struct to be sent
data sensorData;

// Incoming data from sensor in 16-bit format
int16_t accelX, accelY, accelZ;
int16_t gyroX, gyroY, gyroZ;

// Other variable definition
int32_t filteredAccelX, filteredAccelY, filteredAccelZ =0;
int32_t filteredGyroZ =0;
int32_t fixedGyroX, fixedGyroY, fixedGyroZ;
int32_t filtered_data_velocity_x = 0;
uint64_t filtered_data_pos_x = 0u;
uint8_t counter_sending = 0u;
int32_t acc_complete_for_debugging = 0;
uint32_t debugCount = 0u;
int32_t didi = 0;

 //Ringbuffer defined in "ringbuffer.h"
struct common_buffer_data Struct_Accel_X  = initialize_buffer();
struct common_buffer_data Struct_Accel_Y  = initialize_buffer();
struct common_buffer_data Struct_Accel_Z  = initialize_buffer();
struct common_buffer_data Struct_Gyro_Z = initialize_buffer();
// Timestamps
unsigned long previousMillis = 0;
uint32_t previousMillis_stop_cond = 0;


void setup()
 {
  Serial.begin(9600);
  uint64_t systemTime = 0;
  while (systemTime == 0) {
    systemTime = bytesToUint64_StringDigits(simpleGET("/t"));
}
  SUDP_beginn(systemTime);
  if (!IMU.begin()) {
    while (true);
  }
// Checking of correct functions for vector addition

}



void loop() 
{
       unsigned long currentMillis = accurateMillis();
debugCount = micros();

  if (currentMillis - previousMillis >= READ_INTERVAL_MS) 
  {
    previousMillis = currentMillis;

    //bool accelAvailable = IMU.accelerationAvailable();
   // bool gyroAvailable = IMU.gyroscopeAvailable();
    if (IMU.readAcceleration(accelX, accelY, accelZ)) 
    {
      if ((accelX > (int32_t)-ZERO_MOVEMENT) & (accelX < (int32_t)ZERO_MOVEMENT))
      {
        accelX =0u;
      }     
      // zero movemnet auch für y
      if ((accelY > (int32_t)-ZERO_MOVEMENT) & (accelY < (int32_t)ZERO_MOVEMENT))
      {
        accelY =0u;
      }     

      // Aktualisierung des Ringpuffers 
      push_data_to_buffer(accelX, &Struct_Accel_X);
      push_data_to_buffer(accelY, &Struct_Accel_Y);
      push_data_to_buffer(accelZ, &Struct_Accel_Z);
      // Auslesen der Filterwerte
      filteredAccelX = moving_average(&Struct_Accel_X) ;
      filteredAccelY = moving_average(&Struct_Accel_Y) ;
      filteredAccelZ = moving_average(&Struct_Accel_Z) ;
    
      // Eintragen für Debugging 
      
  
      
      // Stop recognition
      
      //if ((buffer_sum_merker <= ZERO_MOVEMENT*RINGBUFFER_SIZE) & (Struct_Accel_X.buffer_sum <= ZERO_MOVEMENT*RINGBUFFER_SIZE)){
        //filtered_data_velocity_x = 0u; //Acceleration has been zero for long, therefore the car isn't moving anymore 

//      }
  //    if (currentMillis - previousMillis_stop_cond >= INTERVAL_STOP_COND) {
    //      previousMillis_stop_cond = currentMillis; // Reset timer
      //    buffer_sum_merker = Struct_Accel_X.buffer_sum; // Write current Buffer in Merker
      //}



    }



    if (IMU.readGyroscope(gyroX, gyroY, gyroZ)) 
    {
    push_data_to_buffer(gyroZ,&Struct_Gyro_Z);
    filteredGyroZ=moving_average(&Struct_Gyro_Z);


      // Evtl. auch hier Aktualierung des Ringpuffers für die Gyro-Werte
    }
   
    
      // Implementation of Vectorial acceleration combination

     Struct_Accel_X.acc_complete = calculation_linear_acceleration(filteredAccelX, filteredAccelY,0,filteredGyroZ, filtered_data_velocity_x);
     integration_32bit(&Struct_Accel_X, &filtered_data_velocity_x);

     integration_64bit(&Struct_Accel_X, &filtered_data_pos_x, filtered_data_velocity_x);
       counter_sending++;

    debugCount=micros()-debugCount;
    Serial.println(debugCount);
if (counter_sending>=20) 
{

    sensorData.accel_vec[0] = accelX;
    sensorData.accel_vec[1] = filteredAccelY;
    sensorData.accel_vec[2] = filteredAccelZ;
    sensorData.gyro_vec[0] = Struct_Accel_X.merker_buffer_sum;    
    sensorData.gyro_vec[1] = acc_complete_for_debugging;  
    sensorData.gyro_vec[2] = gyroZ; 

    sensorData.accel_lin = filteredAccelX;
    sensorData.speed_lin = filtered_data_velocity_x/SPEED_SCALER;
    sensorData.pos_lin = (uint32_t)(filtered_data_pos_x/POSITION_SCALER); // account for Integration error
    sensorData.track_section = 1;
    

    SUDP_send(sensorData);
    counter_sending=0U;

}
  }
  
}
