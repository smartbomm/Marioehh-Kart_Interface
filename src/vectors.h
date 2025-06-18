#ifndef VECTORS_H
#define VECTORS_H

#include <stdint.h>

#define ACCELERATION_FACTOR 1
#define GYRO_FACTOR 1
#define OFFSET_G_VERTICAL 0 //müsste unser faktor 14196 sein bis jetzt null da keine Z Betrachtung
#define THRESHOLD_LATERAL_ACCEL 40000


int32_t vector_addition (int32_t vector_x, int32_t vector_y, int32_t vector_z); // if only input arguments are needed, set vector_z to zero
int32_t vector_subtraction (int32_t vector_combination, int32_t vector_x, int32_t vector_y); // in only one vector needs to be subtractet, set vector_y to zero
int32_t calculation_linear_acceleration (int32_t a_x, int32_t a_y, int32_t a_z, int32_t yaw_rate, int32_t velocity);

#endif

