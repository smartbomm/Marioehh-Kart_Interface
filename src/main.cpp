

#include <Arduino.h>
#include <math.h>
#include <Wire.h>
#include <stdint.h>
#include <Arduino_LSM6DS3.h>

#include "SimpleGET.h"
#include "OdometerData.h"
#include "ringbuffer.h"
#include "Hardware_Config.h"
#include <BarcodeReader.h>
/**
 * @file main.cpp
 * @brief Beschreibung
 * @author JGesell
 * @date 2025-07-13
 */
/// Time intervals
constexpr unsigned long READ_INTERVAL_MS = 2;     // Interval between readings
constexpr unsigned long INTERVAL_STOP_COND = 100;

/// Data struct to be sent
data sensorData;

/// Incoming data from sensor in 16-bit format
int16_t accelX, accelY, accelZ;
int16_t gyroX, gyroY, gyroZ;

///filtered data after moving average
int32_t filteredAccelX, filteredAccelY, filteredAccelZ=0; 
///calculated speed after integration
int32_t filtered_data_velocity_x = 0;
///calculated position after integration
uint64_t filtered_data_pos_x = 0u;
///counter for cycles before sending 
uint8_t counter_sending = 0u;
/// placeholder for debugging return types
int32_t acc_complete_for_debugging = 0;
/// counter for cycles of motor voltage =0;
uint8_t counter_standstill = 0u;
/// track section beeing read from barcode reader
uint8_t barcode_value = 0u;
/// speed beeing read from barcode reader
uint32_t barcode_debug_velocity = 0u;

///initialization of ringbuffer for 3 axis acceleration and motor voltage defined in "ringbuffer.h"
struct common_buffer_data Struct_Accel_X  = initialize_buffer();
struct common_buffer_data Struct_Accel_Y  = initialize_buffer();
struct common_buffer_data Struct_Accel_Z  = initialize_buffer();
struct common_buffer_data Struct_Motor_Voltage = initialize_buffer();

/// Timestamps
unsigned long previousMillis = 0;
uint32_t previousMillis_stop_cond = 0;

/// Barcode 
void EIC_Handler(void) {
    if (EIC->INTFLAG.reg & EIC_INTFLAG_EXTINT7) {
        EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT7;  // Flag löschen
        barcodeIsr();  // Barcode ISR aufrufen
    }
}

barcodeConfig_t barcode_config = {
    .pin = PIN_PA07,          /// Pin where the barcode reader is connected to
    .bitLength = 9489,    /// Length in µm of 1 bit (sequence of black and white section)
    .readingTimeout = 40000  /// Timeout in µs for the reading process
};


void setup()
 {
  uint64_t systemTime = 0;
  while (systemTime == 0) {
    systemTime = bytesToUint64_StringDigits(simpleGET("/t"));
  }
  SUDP_beginn(systemTime);
  if (!IMU.begin()) {
    while (true);
  }
  configure_extint();
  barcode_init(barcode_config);
}

void loop() 
{
       unsigned long currentMillis = accurateMillis();


  if (currentMillis - previousMillis >= READ_INTERVAL_MS) 
  {
    previousMillis = currentMillis;

    if (IMU.readAcceleration(accelX, accelY, accelZ)) 
    {
      ///zero movement in x direction, filters noise; sets all values below the threshold to zero
      if (abs(accelX)<ZERO_MOVEMENT)
      {
        accelX =0u;
      }     
      /// zero movement auch für y
      if (abs(accelY)<ZERO_MOVEMENT)
      {
        accelY =0u;
      }     

      /// putting the actual values to ringbuffer 
      push_data_to_buffer(accelX, &Struct_Accel_X);
      push_data_to_buffer(accelY, &Struct_Accel_Y);
      push_data_to_buffer(accelZ, &Struct_Accel_Z);
      push_data_to_buffer(analogRead(Motor), &Struct_Motor_Voltage);

      ///filtering the values with moving average
      filteredAccelX = moving_average(&Struct_Accel_X) ;
      filteredAccelY = moving_average(&Struct_Accel_Y) ;
      filteredAccelZ = moving_average(&Struct_Accel_Z) ;
      motor_voltage = moving_average(&Struct_Motor_Voltage);

    
      // Speed processing with check against hardwired logic over a short time intervall
      if ((motor_voltage > DEADZONE_MOTOR) && (filteredAccelX == 0)) 
      {
          counter_standstill++;
      }
      else 
      {
        counter_standstill = 0;
      }
      
      if (counter_standstill >= STANDSTILL) 
      {
        filtered_data_velocity_x = 0;
        counter_standstill = 0u;
      }
      else
      {
        ///integration of speed out of filtered x and y acceleration
         integration_32bit(&Struct_Accel_X, &filtered_data_velocity_x, filteredAccelX,filteredAccelY);
      }
      
    
      ///integration of position from intgrated speed; incrementation of send counter
       integration_64bit(Struct_Accel_X, &filtered_data_pos_x, filtered_data_velocity_x);
       counter_sending++;
    }


///reading of Gyroscope values and store them in gyroX,gyroY,gyroZ
    if (IMU.readGyroscope(gyroX, gyroY, gyroZ)) 
    {
    
    }
      

  /// Barcode recognition
    barcode_error_t error = barcode_get(barcode_value, barcode_debug_velocity);
    if (error == READING_SUCCESSFUL){
      /// set back position to zero when entering a new track section
      filtered_data_pos_x = 0;
      ///take the velocity from the barcode reader to prevent integration problems
      filtered_data_velocity_x=(int32_t)(barcode_debug_velocity*SPEED_SCALER);

    }
    /// after 10 programcycles actual values are send via udp to the mqtt bridge
if (counter_sending>=10) 
{

    sensorData.accel_vec[0] = accelX;  ///unfiltered acceleration x
    sensorData.accel_vec[1] = accelY;  ///unfiltered acceleration y
    sensorData.accel_vec[2] = accelZ;  ///unfiltered acceleration z
    sensorData.gyro_vec[0] = gyroX;    ///unfiltered gyro x
    sensorData.gyro_vec[1] = gyroY;    ///unfiltered gyro y
    sensorData.gyro_vec[2] = gyroZ;    ///unfiltered gyro z
    sensorData.error_code = error; /// 0 as standard --> machts was ihr wollt damit


    sensorData.accel_lin = filteredAccelX;   ///acceleration linear 
    sensorData.speed_lin = filtered_data_velocity_x/SPEED_SCALER;    ///speed linear in mm/s
    sensorData.pos_lin = (uint32_t)(filtered_data_pos_x/POSITION_SCALER); /// account for Integration error

    sensorData.track_section = barcode_value;
   

    // Debug Motor Voltage
    Serial.print(motor_voltage);
    

    SUDP_send(sensorData);
    counter_sending=0U;

}
  }
  
}
