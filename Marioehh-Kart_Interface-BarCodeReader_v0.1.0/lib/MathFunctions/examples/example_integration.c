#include "MathFunctions.h"
#include <stdio.h>
#include <time.h>

//This example was written for C on PC and not for Arduino but if modified it can be used on Arduino as well.

void delay(int number_of_seconds);

//Create a new integration handler
integration_handler_t integration_handler;
int main() 
{
    //Initialize memory for the integration handler
    integration_handler = integration_create();
    clock_t current_time = 0;
    int16_t myValue = -300;
    //Initialize the integration handler with a value of 0 and the current time
    integration_init(integration_handler, myValue, current_time/1000000);
    delay(1000);
    printf("Aktueller Wert: %d\n", myValue);
    printf("Aktuelle Zeit: %ld\n", current_time/1000000);
    printf("Wert: %d\n", integration_get_value(integration_handler));
    while(current_time < 60000000)
    {
        current_time = clock();
        myValue += 10;
        printf("Aktueller Wert: %d\n", myValue);
        printf("Aktuelle Zeit: %ld\n", current_time/1000000);
        //Update the integration handler with a value of 1 and the current time
        integration_update(integration_handler, myValue, current_time/1000000);

        //Get the current value of the integration handler and print it
        uint32_t value = integration_get_value(integration_handler);
        printf("Wert: %d\n", value);
        
        delay(1000);

    }
    //Free the memory allocated for the integration handler
    integration_delete(integration_handler);
    return 0;
}



void delay(int number_of_seconds) {
    int milli_seconds = 1000 * number_of_seconds;
    clock_t start_time = clock();
    while (clock() < start_time + milli_seconds) ;
}