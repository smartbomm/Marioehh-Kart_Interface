#include <BarcodeReader.h>

//Macros

#define BARCODE_bits 8
#define BARCODE_edges (BARCODE_bits * 2 + 2)

#define DISABLE_EXTINT_7() EIC->INTENCLR.reg = EIC_INTENCLR_EXTINT3;
#define ENABLE_EXTINT_7() EIC->INTENSET.reg = EIC_INTENSET_EXTINT3;


#ifdef DEBUG
uint8_t _PHASE_MISMATCH_ERROR = 0; // Variable to store the phase mismatch error code for debugging
#define _PHASE_MISMATCH_ERROR(no) _PHASE_MISMATCH_ERROR = no; // Variable to store the phase mismatch error code for debugging
#define DEBUG_PHASE_MISMATCH_ERROR _PHASE_MISMATCH_ERROR // Macro to get the phase mismatch error code for debugging

#else
#define _PHASE_MISMATCH_ERROR(no)
#define DEBUG_PHASE_MISMATCH_ERROR 
#endif



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

// Private ENUMS


// Private Variables
barcode_error_t barcode_error = NO_CODE_DETECTED; // Variable to store the error code from the barcode reader

// Private Functions Prototypes
void _reset_counters();

void printCode(volatile barCodeBit_t code[8])
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
    volatile uint8_t edgeCounter; // every barcode has 18 edges, 9 rising, 9 falling ungerade Zahl: rising, gerade Zahl FALLING
    uint16_t readingTimeout;
} barcodeReader;

void barcodeIsr()
{

    barcodeReader.edgeCounter++;
    uint32_t actualTime = micros();

    // Validate Phase
    bool pinPhase = PORT->Group[PORTA].IN.reg & (1u << 7);
    bool counterPhase = barcodeReader.edgeCounter % 2; // 1 = White Phase, 0 = Black Phase
    if (pinPhase != counterPhase)
    {
        _PHASE_MISMATCH_ERROR(barcodeReader.edgeCounter);
        // Phase mismatch, reset edge counter
        _reset_counters();
        barcode_error = PHASE_MISMATCH_ERROR;
        
        //DISABLE_EXTINT_7();
        return;
    }

    // First edge
    if (1 == barcodeReader.edgeCounter)
    {
        barcodeReader.lastTime = micros();
        return;
    }
    // All other edges
    else if (barcodeReader.edgeCounter < 18u)
    {
        if (pinPhase) // Black Phase ending
        {
            barcodeReader.barcodeByte[barcodeReader.bitCounter].blackTime = actualTime - barcodeReader.lastTime;
            barcodeReader.bitCounter++;
        }
        else // White Phase ending
        {
            barcodeReader.barcodeByte[barcodeReader.bitCounter].whiteTime = actualTime - barcodeReader.lastTime;
        }
        barcodeReader.lastTime = actualTime;
        if (barcodeReader.edgeCounter == 18u)
        {
            DISABLE_EXTINT_7();
        }
    }
}

void barcode_init(barcodeConfig_t config)
{
    barcodeReader.config = config;
    barcodeReader.config.bitLength *= 1000u;
    barcodeReader.pin = config.pin;
    barcodeReader.readingTimeout = config.readingTimeout;
    barcodeReader.bitCounter = 0u;
    barcodeReader.edgeCounter = 0u;

    ENABLE_EXTINT_7();
}

barcode_error_t barcode_get(uint8_t &value, uint32_t &velocity)
{
    barcode_error_t reading_status;
    if (barcode_error != NO_CODE_DETECTED) // All errors
    {
        // Error has been detected, reset counters
        _reset_counters();
        reading_status = barcode_error;
        barcode_error = NO_CODE_DETECTED; // Reset error code
        ENABLE_EXTINT_7();
    }
    else if (barcodeReader.edgeCounter >= 18)
    {
        uint8_t barcodeValue = 0u;
        for (uint8_t i = 0u; i < 8u; i++)
        {
            if (barcodeReader.barcodeByte[i].whiteTime > (barcodeReader.barcodeByte[i].blackTime))
            {
                barcodeValue |= (0x80u >> i); // MSB first
            }
            else
            {
                barcodeValue &= ~(0x80u >> i); // MSB first
            }
        }
        velocity = (uint32_t)barcodeReader.config.bitLength / (barcodeReader.barcodeByte[7].blackTime + barcodeReader.barcodeByte[7].whiteTime);
        value = barcodeValue;
        barcodeReader.bitCounter = 0;
        barcodeReader.edgeCounter = 0u;
        reading_status = READING_SUCCESSFUL; // Reading successful, reset counters and return value
    }
    else if (barcodeReader.edgeCounter == 0u) // No code detected
    {
        reading_status = NO_CODE_DETECTED;
    }
    else // Not finished yet
    {
        //Check for timeouts
        uint32_t actualTime = micros();
        if (actualTime - barcodeReader.lastTime > barcodeReader.readingTimeout)
        {
            _reset_counters();
            reading_status = TIMEOUT_ERROR;

        }
        else
        {
            reading_status = READING_IN_PROGRESS;
        }
        
    }
#ifdef DEBUG
    switch (reading_status)
    {
    case NO_CODE_DETECTED:
        DEBUG_PRINTLN("NO_CODE_DETECTED");
        break;
    case READING_IN_PROGRESS:
        DEBUG_PRINTLN("READING_IN_PROGRESS");
        break;
    case READING_SUCCESSFUL:
        DEBUG_PRINT("READING_SUCCESSFUL: ");
        DEBUG_PRINT("Barcode value: ");
        DEBUG_PRINT(value);
        DEBUG_PRINT(", Velocity: ");
        DEBUG_PRINTLN(velocity);
        break;
    case PHASE_MISMATCH_ERROR:
        DEBUG_PRINT("PHASE_MISMATCH_ERROR, edge no. ");
        DEBUG_PRINT(DEBUG_PHASE_MISMATCH_ERROR);
        DEBUG_PRINTLN();
        break;
    case TIMEOUT_ERROR:
        DEBUG_PRINTLN("TIMEOUT_ERROR");
        break;
    }
#endif
    return reading_status;
}

/*********************************************************************************** */
// Private Functions
/*********************************************************************************** */
void _reset_counters()
{
    barcodeReader.bitCounter = 0u;
    barcodeReader.edgeCounter = 0u;
}