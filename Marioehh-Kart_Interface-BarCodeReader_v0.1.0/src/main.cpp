#include <Arduino.h>
#include <BarcodeReader.h>
#include <MathFunctions.h>
#include <PinConfig.h>
#define Version 0x34

barcodeConfig_t barcode_config;
uint8_t barcode_value = 0;
uint32_t barcode_velocity = 0;

void EIC_Handler(void) {
    if (EIC->INTFLAG.reg & EIC_INTFLAG_EXTINT7) {
        EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT7;  // Flag löschen
        barcodeIsr();  // Barcode ISR aufrufen
    }
}


void setup()
{
    configure_extint();

    barcode_config.pin = 4;       // Pin where the barcode reader is connected to
    barcode_config.bitLength = 7; // Length in mm of 1 bit (seuqence od black and white section)
    barcode_init(barcode_config);
}

void loop()
{
    switch (barcode_get(barcode_value, barcode_velocity))
    {
    case NO_CODE_DETECTED:
        Serial.println("No code detected");
        break;
    case READING_IN_PROGRESS:
        Serial.println("Reading in progress");
        break;
    case READING_SUCCESSFUL:
        Serial.print("Barcode value: ");
        Serial.print(barcode_value);
        Serial.print(" Velocity: ");
        Serial.println(barcode_velocity);
        break;
    }
    delay(1000);
}