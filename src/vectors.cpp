#include <vectors.h>
#include <math.h>
#include <stdint.h>

// Physically correct model

/*
Ax, Vehicle = Linear acceleration needed for integration
Ay, Vehicle = Lateral acceleration of vehicle; always senkrecht to a corner

Ax,Sensor, Ay,Sensor, Az,Sensor = Measurement of sensor; 
*/

int32_t vector_addition (int32_t vector_x, int32_t vector_y, int32_t vector_z)
    {
        int32_t vector_combination = 0;
        vector_combination = sqrt((vector_x^2) + (vector_y^2) + (vector_z^2));
        return vector_combination;
    }



int32_t vector_subtraction (int32_t vector_combination, int32_t vector_x, int32_t vector_y)
    {
        int32_t vector_z = 0;
        vector_z = sqrt((vector_combination^2) - (vector_x^2) - (vector_y^2));
        return vector_z;
    }



int32_t calculation_linear_acceleration (int32_t a_x, int32_t a_y, int32_t a_z, int32_t yaw_rate, int32_t velocity)
    {
            int32_t calculated_accel_linear = 0;
            if (a_y >= 0 && a_y > THRESHOLD_LATERAL_ACCEL)
            {

            calculated_accel_linear = vector_subtraction(vector_addition(a_x, a_y, a_z-OFFSET_G_VERTICAL)*ACCELERATION_FACTOR, velocity*yaw_rate*GYRO_FACTOR,0);
            }
            else if (a_y < 0 && a_y < -THRESHOLD_LATERAL_ACCEL)
            {
            calculated_accel_linear = vector_subtraction(vector_addition(a_x, a_y, a_z-OFFSET_G_VERTICAL)*ACCELERATION_FACTOR, velocity*yaw_rate*GYRO_FACTOR,0);
            }
            else
            {
            calculated_accel_linear = vector_subtraction(vector_addition(a_x, 0, a_z-OFFSET_G_VERTICAL)*ACCELERATION_FACTOR, velocity*yaw_rate*GYRO_FACTOR,0);
            } 
            
            return calculated_accel_linear;
    }