#include "ringbuffer.h"
#include <stdint.h>

 struct common_buffer_data initialize_buffer (void)
  {
    struct common_buffer_data b1;
    b1.index_last_element = 0u;   // Starting with zero; values on this position will be replaced
    b1.ringbuffer_index = RINGBUFFER_SIZE-1;     // Initialized to last Element; needed for Moving average
    memset(b1.ringbuffer,0u,sizeof(b1.ringbuffer));
    b1.kicked_value=0;
    b1.buffer_sum=0;
    b1.buffer_average=0;
    b1.last_time=0;
    b1.current_time=0;
    b1.merker_buffer_sum=0;
    b1.merker_speed=0;
    b1.acc_complete=0;
    b1.merker_accel_complete=0;

    return b1;
}

void push_data_to_buffer (int32_t data, common_buffer_data* buffer)
{
    buffer->kicked_value=buffer->ringbuffer[buffer->index_last_element];
    buffer->ringbuffer[buffer->index_last_element] = -data;
    buffer->index_last_element++;
    buffer->ringbuffer_index++;

    if (buffer -> last_time == 0u) {        // Fixing first Step from 0 to 460000000ms
        buffer -> last_time = accurateMillis() - 1u;
    }
    else {
        buffer->last_time = buffer->current_time;
    }

    buffer->current_time = accurateMillis();
    

    if (buffer->ringbuffer_index >= RINGBUFFER_SIZE) {
        buffer->ringbuffer_index = 0u;
    }
    if (buffer->index_last_element >= RINGBUFFER_SIZE) {
        buffer->index_last_element = 0u;
    }
  
}

int32_t moving_average (common_buffer_data* buffer) 
{   buffer->merker_buffer_sum=buffer->buffer_sum;
    buffer->buffer_sum=buffer->buffer_sum-buffer->kicked_value+buffer->ringbuffer[buffer->ringbuffer_index];
    return buffer->buffer_sum;
}

int32_t integration_32bit(common_buffer_data* buffer,int32_t* speed) 
{
    buffer->merker_accel_complete=buffer->acc_complete;  
    buffer->merker_speed= *speed;                        //merker werden überschrieben
    // folgend ist ein erster versuch einer betrachtung von Kurven 
    //hierzu wird der integration 32 bit accel y mit übergeben

    /*
    if (accel_Y<-ZERO_MOVEMENT_Y)  //rechtskurve
    {
            buffer->acc_complete=accel_linear+(accel_Y*(k/(*speed)));
    }
    else if (accel_Y>=ZERO_MOVEMENT_Y) // linkskurve
    {
            buffer->acc_complete=accel_linear+(-accel_Y*(k/(*speed)));
    }
    else 
        {
        buffer->acc_complete=accel_linear;  // fall für gerade
        }
    */
   //folgend Runge-Kutta 4.Ordnung für bessere numerische Genauigkeit/Dämpfung/Stabilität
    int32_t a1=buffer->merker_accel_complete;
    int32_t a4=buffer->acc_complete;
    int32_t a2=(2*a1+a4)/3;
    int32_t a3=(a1+2*a4)/3;
    int32_t dt = buffer->current_time-buffer->last_time;
    *speed = *speed+((a1+2*a2+2*a3+a4)/6)*dt;
    return buffer->acc_complete;
}

void integration_64bit(common_buffer_data* buffer,uint64_t * position, int32_t speed_linear) 
{
    int32_t dt = buffer->current_time-buffer->last_time;
    int32_t v1=buffer->merker_speed;
    int32_t v4=speed_linear;
    int32_t v2=(2*v1+v4)/3;
    int32_t v3=(v1+2*v4)/3;
    *position = *position+((v1+2*v2+2*v3+v4)/6)*dt;
}








