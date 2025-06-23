#include <BarcodeReader.h>


#define BARCODE_bits 8
#define BARCODE_edges (BARCODE_bits*2+2)

typedef enum 
{
    PHASE_BLACK,
    PHASE_WHITE
} barcodePhase_t;

typedef struct bcb
{
    uint32_t whiteTime;
    uint32_t blackTime;
} barCodeBit_t;

void printCode(volatile barCodeBit_t code [8])
{
    Serial.println("Bit | White | Black | Value");
    Serial.println("----|-------|-------|-------");
    for (uint8_t i = 0; i < 8; i++)
    {
        Serial.print(i);
        Serial.print("   | ");
        Serial.print(code[i].whiteTime);
        Serial.print(" | ");
        Serial.print(code[i].blackTime);
        Serial.print(" | ");
        if (code[i].whiteTime > code[i].blackTime)
        {
            Serial.println("1");
        }
        else
        {
            Serial.println("0");
        }
    }
}

struct barcodeReader_t
{
    barcodeConfig_t config;
    uint8_t pin;
    volatile uint8_t bitCounter;
    volatile uint32_t lastTime;
    volatile barCodeBit_t barcodeByte[8];
    volatile uint8_t edgeCounter; //every barcode has 18 edges, 9 rising, 9 falling ungerade Zahl: rising, gerade Zahl FALLING
} barcodeReader;



void barcodeIsr()
{
    barcodeReader.edgeCounter++;
    if (1 == barcodeReader.edgeCounter)
    {
        return;
    }
    else if(barcodeReader.edgeCounter<18u) 
    {
        uint32_t actualTime = micros();
        if (0 == barcodeReader.edgeCounter%2)   //White Phase endind
        {
            barcodeReader.barcodeByte[barcodeReader.bitCounter].whiteTime = actualTime - barcodeReader.lastTime;
        }
        else    //Black Phase
        {
            barcodeReader.barcodeByte[barcodeReader.bitCounter].blackTime = actualTime - barcodeReader.lastTime;
            barcodeReader.bitCounter++;
        }
        barcodeReader.lastTime = actualTime;
    }
}

void barcode_init(barcodeConfig_t config)
{
    barcodeReader.config = config;
    barcodeReader.config.bitLength *= 1000000u;
    barcodeReader.bitCounter = 0u;
    barcodeReader.edgeCounter = 0u;
    pinMode(config.pin, INPUT);
    attachInterrupt(digitalPinToInterrupt(config.pin), barcodeIsr, CHANGE);
}


barcode_error_t barcode_get(uint8_t &value, uint32_t &velocity)
{
    if (barcodeReader.edgeCounter >= 18)
    {
        uint8_t barcodeValue = 0u;
        for (uint8_t i = 0u; i < 8u; i++)
        {
            if (barcodeReader.barcodeByte[i].whiteTime > barcodeReader.barcodeByte[i].blackTime)    
            {
                barcodeValue |= (0x80u >> i);   //MSB first
            }
            else
            {
                barcodeValue &= ~(0x80u >> i);  //MSB first
            }
        }
        velocity = (uint32_t) barcodeReader.config.bitLength / (barcodeReader.barcodeByte[7].blackTime + barcodeReader.barcodeByte [7].whiteTime);
        value = barcodeValue;
        barcodeReader.bitCounter = 0;
        barcodeReader.edgeCounter = 0u;
        return READING_SUCCESSFUL;
    }
    else if (barcodeReader.edgeCounter == 0u)   //No code detected
    {
        return NO_CODE_DETECTED;
        
    }
    else    // Not finished yet
    {
        return READING_IN_PROGRESS;
    }
}

