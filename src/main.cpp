//#include <samd21/include/samd21g18a.h>
#include <Arduino.h>
#include <Arduino_LSM6DS3.h>
#include <MathFunctions.h>

uint32_t lastTime = 0;

struct accelerationData {
    int16_t x;
    int16_t y;
    int16_t z;
}raw, filtered, velocity, position;
int32_t accel_xy = 0;
int32_t velocity_xy = 0;

// Low-pass filter parameters
moving_average_handler_t lowPassFilterX;
moving_average_handler_t lowPassFilterY;
moving_average_handler_t lowPassFilterZ;

//Integration parameters
integration_handler_t integrationX;
integration_handler_t integrationY;
integration_handler_t integrationZ;

void setup() {
    lowPassFilterX = moving_average_create(10);
    lowPassFilterY = moving_average_create(10);
    lowPassFilterZ = moving_average_create(10);

    integrationX = integration_create();
    integrationY = integration_create();
    integrationZ = integration_create();

    Serial.begin(9600);
    while (!Serial) {
        delay(10);
    }
    Serial.println("Hello, world!");
    if (!IMU.begin()) {
        Serial.println("Failed to initialize IMU!");
        while (1);
    }
    while (!IMU.accelerationAvailable()) {
        IMU.readAcceleration(raw.x, raw.y, raw.z);
        raw.x = raw.x / 1000;
        raw.y = raw.y / 1000;
        raw.z = raw.z / 1000;
        moving_average_init(lowPassFilterX, raw.x);
        moving_average_init(lowPassFilterY, raw.y);
        moving_average_init(lowPassFilterZ, raw.z);
    }


    uint32_t time = micros()/1000000;
    integration_init(integrationX, raw.x, time);
    integration_init(integrationY, raw.y, time);
    integration_init(integrationZ, raw.z, time);
}
void loop() {

    
    if (IMU.accelerationAvailable()) {
        IMU.readAcceleration(raw.x, raw.y, raw.z);
        raw.x = raw.x / 1000;
        raw.y = raw.y / 1000;
        raw.z = raw.z / 1000;
    }
    // Apply a low-pass filter to the raw acceleration data
    moving_average_write(lowPassFilterX, raw.x);
    moving_average_write(lowPassFilterY, raw.y);
    moving_average_write(lowPassFilterZ, raw.z);
    filtered.x = moving_average_read(lowPassFilterX);
    filtered.y = moving_average_read(lowPassFilterY);
    filtered.z = moving_average_read(lowPassFilterZ);
    // Calculate velocity and position

    uint32_t time = micros()/1000000;
    integration_update(integrationX, filtered.x, time);
    integration_update(integrationY, filtered.y, time);
    integration_update(integrationZ, filtered.z, time);
    velocity.x = integration_get_value(integrationX);
    velocity.y = integration_get_value(integrationY);
    velocity.z = integration_get_value(integrationZ);

    // Print the results
    if(time - lastTime > 1) {
        Serial.print("Raw Acceleration: ");
        Serial.print("X: "+String(raw.x));
        Serial.print(" Y: "+String(raw.y));
        Serial.print(" Z: "+String(raw.z));
        Serial.println();
        Serial.print("X: "+String(velocity.x));
    Serial.print(" Y: "+String(velocity.y));
    Serial.print(" Z: "+String(velocity.z));
    Serial.println();

        lastTime = time;
    }
    


}