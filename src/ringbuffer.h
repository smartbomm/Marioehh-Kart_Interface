#include <stdint.h>
#include "SimpleNET.h"
#include <Arduino.h>


// Define length of buffer
#define RINGBUFFER_SIZE 34u
// Define Zero-Border for Accelleration values in x
#define ZERO_MOVEMENT 380
// define Scaler
#define SPEED_SCALER 14196
#define POSITION_SCALER 14196126
//define g
#define G 9.81
// define Zero-Border for contemplation of accel_y
#define ZERO_MOVEMENT_Y 40000 
// define empiric factor for valueing accel_Y in acceleration; vorher 12000
#define k      50
// define empiric factor for valueing accel_Y in breaking
#define kbrak 50000000


struct common_buffer_data
{
    /* data */
    int16_t ringbuffer[RINGBUFFER_SIZE];      //Buffer for sensor Values
    uint8_t ringbuffer_index;                 //ringbuffer index with actual value
    uint8_t index_last_element;               //index with element which is kicked in next call
    int16_t kicked_value;                     //kicked element is stored here for moving average
    int32_t buffer_sum;                       //sum in moving average filter
    int32_t buffer_average;                   //average 
    int32_t last_time;                        //needed for integration
    int32_t current_time;                     //needed for Integration
    int32_t merker_buffer_sum;                //needed for calculation of dx in 32 bit integration
    int32_t merker_speed;                     //needed for calculation of dx in 64 bit integration
    int32_t merker_accel_complete;            //needed for calculation of dx when acc_y is relevant
    int32_t acc_complete;                     // placeholder in 32 bit integration for curve speed

};


 struct common_buffer_data initialize_buffer (void);

void push_data_to_buffer (int32_t data, common_buffer_data* buffer);


int32_t moving_average (common_buffer_data* buffer); 


int32_t integration_32bit(common_buffer_data* buffer,int32_t* speed, int32_t accel_linear,int32_t accel_Y); 


void integration_64bit(common_buffer_data* buffer,uint64_t * position, int32_t speed_linear); 








