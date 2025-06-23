#ifndef BarcodeReader_h
#define BarcodeReader_h
#include <Arduino.h>

/**
 * @brief Firmware component to read a barcode linear. 
 * The barcode has to be encoded timebased.
 * 
 * ##Encoding barcode 
 */

 /**
  * @brief Enum to define actual status from the barcode reader unit
  * 
  */
typedef enum
{
    NO_CODE_DETECTED,
    READING_IN_PROGRESS,
    READING_SUCCESSFUL
} barcode_error_t;

/**
 * @brief Configuration struct for the barcode reader
 * @param pin Digital input pin where the barcode reader is connected to
 * @param bitLength Length in mm of 1 bit (seuqence od black and white section)
 * 
 */
typedef struct  {
    uint8_t pin;
    uint32_t bitLength;
} barcodeConfig_t;

/**
 * @brief Read the value and the calculated velocity from a barcode
 * 
 * @param value Pointer variable to store the barcode value to
 * @param velocity Pointer variable to store the velocity value in nm/µs = µm/ms = mm/s
 * @return barcode_error_t returns READING_SUCCESSFULL if barcode has been read and READING_IN_PROGRESS if reading is already in process and NO_CODE_DETCTED if there hasn't been detected a code since the last call of the function
 */
barcode_error_t barcode_get(uint8_t &value, uint32_t &velocity);

/**
 * @brief Initialize the pin configuration, the interrupt and the variables for the barcode reader
 * 
 * @param config config struct for configuration of the barcode reader
 */
void barcode_init(barcodeConfig_t config);

#endif
