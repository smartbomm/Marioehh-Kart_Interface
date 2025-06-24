#include <Arduino.h>
#include <math.h>
#include <Wire.h>
#include <stdint.h>
#include <Arduino_LSM6DS3.h>

#include <BarcodeReader.h>
#include <PinConfig.h>

#include "SimpleGET.h"
#include "OdometerData.h"
#include "Ringbuffer.h"


// Barcode recognition
        // Interrupt vector for Barcode recognition
        void EIC_Handler(void) {
            if (EIC->INTFLAG.reg & EIC_INTFLAG_EXTINT7) {
                EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT7;  // Flag löschen
                barcodeIsr();  // Barcode ISR aufrufen
            }
        }
        // Define variable for Barcode handling
        barcodeConfig_t barcode_config;
        uint8_t barcode_value = 0;
        uint32_t barcode_velocity = 0;

// Time intervals
constexpr unsigned long READ_INTERVAL_MS = 2;     // Interval between readings
constexpr unsigned long INTERVAL_STOP_COND = 100;

// Data struct to be sent
data sensorData;

// Incoming data from sensor in 16-bit format
int16_t accelX, accelY, accelZ;
int16_t gyroX, gyroY, gyroZ;

// Other variable definition
int32_t filteredAccelX, filteredAccelY, filteredAccelZ;
int32_t fixedGyroX, fixedGyroY, fixedGyroZ;
int32_t filtered_data_velocity_x = 0;
uint64_t filtered_data_pos_x = 0u;
uint8_t counter_sending = 0u;
int32_t dx_for_debugging = 0;
uint32_t debugCount = 0u;
 //Ringbuffer defined in "ringbuffer.h"
struct common_buffer_data Struct_Accel_X  = initialize_buffer();
struct common_buffer_data Struct_Accel_Y  = initialize_buffer();
struct common_buffer_data Struct_Accel_Z  = initialize_buffer();

// Timestamps
unsigned long previousMillis = 0;
uint32_t previousMillis_stop_cond = 0;


void setup()
 {
  //Barcode reader setup
    configure_extint();

    barcode_config.pin = 4;       // Pin where the barcode reader is connected to
    barcode_config.bitLength = 7; // Length in mm of 1 bit (seuqence od black and white section)
    barcode_init(barcode_config);

  // Other configurations
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
      dx_for_debugging = integration_32bit(&Struct_Accel_X, &filtered_data_velocity_x, filteredAccelX,filteredAccelY);
  
      
      // Stop recognition
      
      //if ((buffer_sum_merker <= ZERO_MOVEMENT*RINGBUFFER_SIZE) & (Struct_Accel_X.buffer_sum <= ZERO_MOVEMENT*RINGBUFFER_SIZE)){
        //filtered_data_velocity_x = 0u; //Acceleration has been zero for long, therefore the car isn't moving anymore 

//      }
  //    if (currentMillis - previousMillis_stop_cond >= INTERVAL_STOP_COND) {
    //      previousMillis_stop_cond = currentMillis; // Reset timer
      //    buffer_sum_merker = Struct_Accel_X.buffer_sum; // Write current Buffer in Merker
      //}


       integration_64bit(&Struct_Accel_X, &filtered_data_pos_x, filtered_data_velocity_x);
       counter_sending++;
    }



    if (IMU.readGyroscope(gyroX, gyroY, gyroZ)) 
    {
    


      // Evtl. auch hier Aktualierung des Ringpuffers für die Gyro-Werte
    }
      
    debugCount=micros()-debugCount;
    Serial.println(debugCount);

    // Check against real data gathered from Barcode reader
    switch (barcode_get(barcode_value, barcode_velocity))
    {
    case NO_CODE_DETECTED:
        Serial.println("No code detected");
        break;
    case READING_IN_PROGRESS:
        Serial.println("Reading in progress");
        break;
    case READING_SUCCESSFUL:
        Serial.print("Barcode value: ");
        Serial.print(barcode_value);

        Serial.print(" Velocity: ");
        Serial.println(barcode_velocity);
        //Hard set of Velocity to actual measured velocity by the barcode reader
        filtered_data_velocity_x = barcode_velocity* SPEED_SCALER;      // barcode velocity is in mm/s; needs scaling
        break;
    }

    // Send the gathered data
    if (counter_sending>=20) 
    {

        sensorData.accel_vec[0] = accelX;
        sensorData.accel_vec[1] = filteredAccelY;
        sensorData.accel_vec[2] = filteredAccelZ;
        sensorData.gyro_vec[0] = Struct_Accel_X.merker_buffer_sum;    
        sensorData.gyro_vec[1] = dx_for_debugging;  
        sensorData.gyro_vec[2] = Struct_Accel_X.merker_speed; 

        sensorData.accel_lin = filteredAccelX;
        sensorData.speed_lin = filtered_data_velocity_x/SPEED_SCALER;
        sensorData.pos_lin = (uint32_t)(filtered_data_pos_x/POSITION_SCALER); // account for Integration error
        sensorData.track_section = barcode_value;
        

        SUDP_send(sensorData);
        counter_sending=0U;

    }
  }
  
}
