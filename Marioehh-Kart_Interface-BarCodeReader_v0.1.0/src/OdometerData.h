#ifndef Data_h
#define Data_h
#include <stdint.h>

#define VERSION 0.1

typedef struct data {
    uint8_t section;
    uint32_t pos_x;
    uint32_t pos_y;
    uint32_t pos_lin;
    uint32_t speed;
    uint32_t accel;
} odometerData_t;


#endif