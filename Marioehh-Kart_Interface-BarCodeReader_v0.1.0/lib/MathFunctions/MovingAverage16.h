#ifndef MovingAverage16_h
#define MovingAverage16_h
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif



struct _moving_average16_t;
typedef struct _moving_average16_t* moving_average_handler_t;

moving_average_handler_t moving_average_create(uint8_t size);
void moving_average_init(moving_average_handler_t f, int16_t value);
void moving_average_delete(moving_average_handler_t f);
void moving_average_write(moving_average_handler_t f, int16_t value);
int16_t moving_average_read(moving_average_handler_t f);


#ifdef __cplusplus
}
#endif
#endif