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
        vector_combination = sqrt((vector_x/10 * vector_x/10) + (vector_y/10 * vector_y/10) + (vector_z/10 * vector_z/10)); // Ohne Verrechnung overflow bei ca. 3,13 m/s^2, deswegen Genauigkeitsverlust in Kauf nehmen
        return vector_combination * 10;
    }



int32_t vector_subtraction (int32_t vector_combination, int32_t vector_x, int32_t vector_y)
    {
        int32_t vector_z = 0;
        vector_z = sqrt((vector_combination * vector_combination) - (vector_x * vector_x) - (vector_y * vector_y));
        return vector_z;
    }



int32_t calculation_linear_acceleration (int32_t a_x, int32_t a_y, int32_t a_z, int32_t yaw_rate, int32_t velocity)
    {
            int32_t combined_acceleration = 0;
            int32_t calculated_acc_linear = 0;

            if (a_x >= 0)  // Beschleunigen
            {
                if (a_y >= 0 && a_y > THRESHOLD_LATERAL_ACCEL)
                {
                // Debug
                calculated_acc_linear = vector_addition(a_x, a_y,0)*ACCELERATION_FACTOR;
                //PLanned solution
                /*
                combined_acceleration = vector_addition(a_x, a_y,0)*ACCELERATION_FACTOR;
                calculated_acc_linear = vector_subtraction(combined_acceleration,velocity * yaw_rate * GYRO_FACTOR,0);
                */
                }
                else if (a_y < 0 && a_y < -THRESHOLD_LATERAL_ACCEL)
                {
                // Debug
                calculated_acc_linear = vector_addition(a_x, a_y,0)*ACCELERATION_FACTOR;
                //PLanned solution
                /*
                combined_acceleration = vector_addition(a_x, a_y,0)*ACCELERATION_FACTOR;
                calculated_acc_linear = vector_subtraction(combined_acceleration,velocity * yaw_rate * GYRO_FACTOR,0);
                */
                }
                else
                {
                // Debug
                calculated_acc_linear = vector_addition(a_x,0,0)*ACCELERATION_FACTOR;
                //PLanned solution
                /*
                combined_acceleration = vector_addition(a_x, a_y,0)*ACCELERATION_FACTOR;
                calculated_acc_linear = vector_subtraction(combined_acceleration,velocity * yaw_rate * GYRO_FACTOR,0);
                */
                }
            }
            else // Bremsen
            {
                if (a_y >= 0 && a_y > THRESHOLD_LATERAL_ACCEL)
                {
                // Debug
                calculated_acc_linear = -vector_addition(a_x, a_y,0)*ACCELERATION_FACTOR;
                //PLanned solution
                /*
                combined_acceleration = vector_addition(a_x, a_y,0)*ACCELERATION_FACTOR;
                calculated_acc_linear = vector_subtraction(combined_acceleration,velocity * yaw_rate * GYRO_FACTOR,0);
                */;
                }
                else if (a_y < 0 && a_y < -THRESHOLD_LATERAL_ACCEL)
                {
                // Debug
                calculated_acc_linear = -vector_addition(a_x, a_y,0)*ACCELERATION_FACTOR;
                //PLanned solution
                /*
                combined_acceleration = vector_addition(a_x, a_y,0)*ACCELERATION_FACTOR;
                calculated_acc_linear = vector_subtraction(combined_acceleration,velocity * yaw_rate * GYRO_FACTOR,0);
                */
                }
                else
                {
                // Debug
                calculated_acc_linear = -vector_addition(a_x,0,0)*ACCELERATION_FACTOR;
                //PLanned solution
                /*
                combined_acceleration = vector_addition(a_x, a_y,0)*ACCELERATION_FACTOR;
                calculated_acc_linear = vector_subtraction(combined_acceleration,velocity * yaw_rate * GYRO_FACTOR,0);
                */
                }                
            } 
            
            return calculated_acc_linear;
    }