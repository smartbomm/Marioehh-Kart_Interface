/*
  This file is part of the Arduino_LSM6DS3 library.
  Copyright (c) 2019 Arduino SA. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "LSM6DS3.h"

LSM6DS3Class::LSM6DS3Class(TwoWire& wire, uint8_t slaveAddress) :
  _wire(&wire),
  _spi(NULL),
  _slaveAddress(slaveAddress)
{
}

LSM6DS3Class::LSM6DS3Class(SPIClass& spi, int csPin, int irqPin) :
  _wire(NULL),
  _spi(&spi),
  _csPin(csPin),
  _irqPin(irqPin),
  _spiSettings(10E6, MSBFIRST, SPI_MODE0)
{
}

LSM6DS3Class::~LSM6DS3Class()
{
}

int LSM6DS3Class::begin()
{
  if (_spi != NULL) {
    pinMode(_csPin, OUTPUT);
    digitalWrite(_csPin, HIGH);
    _spi->begin();
  } else {
    _wire->begin();
  }

  if (!(readRegister(LSM6DS3_WHO_AM_I_REG) == 0x6C || readRegister(LSM6DS3_WHO_AM_I_REG) == 0x69)) {
    end();
    return 0;
  }

  // Set the Accelerometer control register
  // Output Data Rate: 3,33 kHz 
  // Full-Scale Selection: 8g
  // Low pass filter enabled
  // Bandwidth Selection: 400Hz
  writeRegister(LSM6DS3_CTRL1_XL, 0x9E);

  // Set the Gyroscope control register
  // Output data rate: 104Hz
  // Full-Scale Selection: 500dps
  writeRegister(LSM6DS3_CTRL2_G, 0x89);

  // Hardware configurations
  // No need for changes
  // writeRegister(LSM6DS3_CTRL3_C, 0x00);

  // Enable digital LPF1 for Gyroscope 
  writeRegister(LSM6DS3_CTRL4_C, 0x01);

  // Activate rounding on the accelerometer (0x20) or the Gyroscope (0x40)
  writeRegister(LSM6DS3_CTRL5_C, 0x20);

  // Select the bandwidth for the low-pass filter of the Gyroscope
  // Just from ODR = 800Hz
  writeRegister(LSM6DS3_CTRL6_C, 0x00);

  // Enable Modes for Gyro
  // High Performance mode enabled
  // HighPassFilter disabled
  writeRegister(LSM6DS3_CTRL7_G, 0x00);

  // LPF2 Selection at ODR/400
  // Configuration for Filters: 
  // Reference mode for HP Filter: off
  // Composite filter: ODR/4 low pass filtered gets sent
  writeRegister(LSM6DS3_CTRL8_XL, 0xA8);

  // Something with DEN Values
  writeRegister(LSM6DS3_CTRL9_XL, 0x00);

  // Pedometer functions etc. 
  writeRegister(LSM6DS3_CTRL10_C, 0x00);

  return 1;
}

void LSM6DS3Class::end()
{
  if (_spi != NULL) {
    _spi->end();
    digitalWrite(_csPin, LOW);
    pinMode(_csPin, INPUT);
  } else {
    writeRegister(LSM6DS3_CTRL2_G, 0x00);
    writeRegister(LSM6DS3_CTRL1_XL, 0x00);
    _wire->end();
  }
}

int LSM6DS3Class::readAcceleration(int16_t& x, int16_t& y, int16_t& z)
{
  int16_t data[3];

  if (!readRegisters(LSM6DS3_OUTX_L_XL, (uint8_t*)data, sizeof(data))) {
    x = 0;
    y = 0;
    z = 0;

    return 0;
  }

  x = data[0];
  y = data[1];
  z = data[2];

  return 1;
}

int LSM6DS3Class::accelerationAvailable()
{
  if (readRegister(LSM6DS3_STATUS_REG) & 0x01) {
    return 1;
  }

  return 0;
}

uint8_t LSM6DS3Class::accelerationSampleRate()
{
  return 104u;
}

int LSM6DS3Class::readGyroscope(int16_t& x, int16_t& y, int16_t& z)
{
  int16_t data[3];

  if (!readRegisters(LSM6DS3_OUTX_L_G, (uint8_t*)data, sizeof(data))) {
    x = 0;
    y = 0;
    z = 0;

    return 0;
  }

  x = data[0];
  y = data[1];
  z = data[2];

  return 1;
}

int LSM6DS3Class::gyroscopeAvailable()
{
  if (readRegister(LSM6DS3_STATUS_REG) & 0x02) {
    return 1;
  }

  return 0;
}

uint8_t LSM6DS3Class::gyroscopeSampleRate()
{
  return 104;
}

int LSM6DS3Class::readTemperature(int16_t& t)
{
  int16_t data;

  if (!readRegisters(LSM6DS3_OUT_TEMP_L, (uint8_t*)data, sizeof(data))) {
    t = 0;

    return 0;
  }

  t = (data / 16u) + 25;

  return 1;
}

int LSM6DS3Class::temperatureAvailable()
{
  if (readRegister(LSM6DS3_STATUS_REG) & 0x04) {
    return 1;
  }

  return 0;
}

uint8_t LSM6DS3Class::temperatureSampleRate()
{
  return 52u;
}

int LSM6DS3Class::readRegister(uint8_t address)
{
  uint8_t value;

  if (readRegisters(address, &value, sizeof(value)) != 1) {
    return -1;
  }

  return value;
}

int LSM6DS3Class::readRegisters(uint8_t address, uint8_t* data, size_t length)
{
  if (_spi != NULL) {
    _spi->beginTransaction(_spiSettings);
    digitalWrite(_csPin, LOW);
    _spi->transfer(0x80 | address);
    _spi->transfer(data, length);
    digitalWrite(_csPin, HIGH);
    _spi->endTransaction();
  } else {
    _wire->beginTransmission(_slaveAddress);
    _wire->write(address);

    if (_wire->endTransmission(false) != 0) {
      return -1;
    }

    if (_wire->requestFrom(_slaveAddress, length) != length) {
      return 0;
    }

    for (size_t i = 0; i < length; i++) {
      *data++ = _wire->read();
    }
  }
  return 1;
}

int LSM6DS3Class::writeRegister(uint8_t address, uint8_t value)
{
  if (_spi != NULL) {
    _spi->beginTransaction(_spiSettings);
    digitalWrite(_csPin, LOW);
    _spi->transfer(address);
    _spi->transfer(value);
    digitalWrite(_csPin, HIGH);
    _spi->endTransaction();
  } else {
    _wire->beginTransmission(_slaveAddress);
    _wire->write(address);
    _wire->write(value);
    if (_wire->endTransmission() != 0) {
      return 0;
    }
  }
  return 1;
}

#ifdef ARDUINO_AVR_UNO_WIFI_REV2
  LSM6DS3Class IMU(SPI, SPIIMU_SS, SPIIMU_INT);
#else
  LSM6DS3Class IMU_LSM6DS3(Wire, LSM6DS3_ADDRESS);
#endif

