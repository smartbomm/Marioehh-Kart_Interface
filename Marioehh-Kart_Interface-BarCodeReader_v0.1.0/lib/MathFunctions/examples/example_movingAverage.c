#include "MathFunctions.h"
#include <stdio.h>
#include <time.h>

//This example was written for C on PC and not for Arduino but if modified it can be used on Arduino as well.

void delay(int number_of_seconds);

//Create a new integration handler
moving_average_handler_t moving_average_handler;
int main() 
{
    //
    moving_average_handler = moving_average_create(20);
    clock_t current_time = 0;
    int16_t myValue = -200;
    
    moving_average_init(moving_average_handler, myValue);
    while(current_time < 60000000)
    {
        current_time = clock();
        myValue += 10;
        
        moving_average_write(moving_average_handler, myValue);

        //Get the current value of the integration handler and print it
        int16_t value = moving_average_read(moving_average_handler);
        printf("Wert: %d\n", value);
        
        delay(1000);

    }
    
    moving_average_delete(moving_average_handler);
    return 0;
}



void delay(int number_of_seconds) {
    int milli_seconds = 1000 * number_of_seconds;
    clock_t start_time = clock();
    while (clock() < start_time + milli_seconds) ;
}