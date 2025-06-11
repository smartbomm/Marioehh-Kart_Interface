#include <Arduino_LSM6DS3.h>
#include <Arduino.h>
#include <MathFunctions.h>

integration_handler_t integration_handler;
int main() 
{

    integration_handler = integration_create();
    Serial.begin(9600);
    while(1)
    {
        integration_update(integration_handler, 1, 1);
        integration_update(integration_handler, 1, 2);
        integration_update(integration_handler, 1, 3);
        uint32_t value = integration_get_value(integration_handler);
        Serial.println(value);
        delay(1000);

    }
}