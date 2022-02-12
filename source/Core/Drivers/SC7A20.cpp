/*
 * SC7A20.cpp
 *
 *  Created on: 18 Sep. 2020
 *      Author: Ralim
 */

#include <SC7A20.hpp>
#include <SC7A20_defines.h>
#include <array>

uint8_t SC7A20::activeAddress;

bool SC7A20::detect() {
  if (FRToSI2C::probe(SC7A20_ADDRESS)) {
    // Read chip id to ensure its not an address collision
    uint8_t id = 0;
    if (FRToSI2C::Mem_Read(SC7A20_ADDRESS, SC7A20_WHO_AMI_I, &id, 1)) {
      if (id == SC7A20_WHO_AM_I_VALUE) {
        activeAddress = SC7A20_ADDRESS;
        return true;
      }
    }
  }
  if (FRToSI2C::probe(SC7A20_ADDRESS2)) {
    // Read chip id to ensure its not an address collision
    uint8_t id = 0;
    if (FRToSI2C::Mem_Read(SC7A20_ADDRESS2, SC7A20_WHO_AMI_I, &id, 1)) {
      if (id == SC7A20_WHO_AM_I_VALUE) {
        activeAddress = SC7A20_ADDRESS2;
        return true;
      }
    }
  }
  return false;
}

static const FRToSI2C::I2C_REG i2c_registers[] = {
    //
    //
    {SC7A20_CTRL_REG1, 0b01100111, 0}, // 200Hz, XYZ enabled
    {SC7A20_CTRL_REG2, 0b00000000, 0}, // Setup filter to 0x00 ??
    {SC7A20_CTRL_REG3, 0b00000000, 0}, // int1 off
    {SC7A20_CTRL_REG4, 0b01001000, 0}, // Block mode off,little-endian,2G,High-pres,self test off
    {SC7A20_CTRL_REG5, 0b00000100, 0}, // fifo off, D4D on int1
    {SC7A20_CTRL_REG6, 0x00, 0},       // INT2 off
    // Basically setup the unit to run, and enable 4D orientation detection
    {SC7A20_INT2_CFG, 0b01111110, 0}, // setup for movement detection
    {SC7A20_INT2_THS, 0x28, 0},       //
    {SC7A20_INT2_DURATION, 64, 0},    //
    {SC7A20_INT1_CFG, 0b01111110, 0}, //
    {SC7A20_INT1_THS, 0x28, 0},       //
    {SC7A20_INT1_DURATION, 64, 0}

    //
};
bool SC7A20::initalize() {
  // Setup acceleration readings
  // 2G range
  // bandwidth = 250Hz
  // High pass filter on (Slow compensation)
  // Turn off IRQ output pins
  // Orientation recognition in symmetrical mode
  // Hysteresis is set to ~ 16 counts
  // Theta blocking is set to 0b10

  return FRToSI2C::writeRegistersBulk(activeAddress, i2c_registers, sizeof(i2c_registers) / sizeof(i2c_registers[0]));
}

void SC7A20::getAxisReadings(int16_t &x, int16_t &y, int16_t &z) {
  // We can tell the accelerometer to output in LE mode which makes this simple
  uint16_t sensorData[3] = {0, 0, 0};

  if (FRToSI2C::Mem_Read(SC7A20_ADDRESS, SC7A20_OUT_X_L, (uint8_t *)sensorData, 6) == false) {
    x = y = z = 0;
    return;
  }
  // Shift 6 to make its range ~= the other accelerometers
  x = sensorData[0];
  y = sensorData[1];
  z = sensorData[2];
}
