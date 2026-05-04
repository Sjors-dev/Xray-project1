#ifndef PROTOCOL_I2C_H
#define PROTOCOL_I2C_H

#include <Arduino.h>
#include <Wire.h>

// I2C addresses
#define GEO_I2C_ADDRESS 0x10
#define XRAY_I2C_ADDRESS 0x20

// SAN bus pins
#define SAN_XRAY_ENABLED_PIN 7
#define SAN_GEO_MOVING_PIN 8

// Registers
#define REG_CMD 0x08
#define REG_STATUS 0x09

// Commands (bits 5:4 of REG_CMD)
#define CMD_PREPARE 0x01
#define CMD_UNPREPARE 0x02

#define CMD_SHIFT 4

// State bits (REG_STATUS)
#define STATE_IDLE 0x00
#define STATE_PREPARING 0x40
#define STATE_PREPARED 0x80
#define STATE_ACQUIRING 0xC0
#define STATE_MASK 0xC0

// Exam type bits
#define EXAM_SHIFT 3
#define EXAM_MASK 0x38

// Geometry registers
#define GEO_REG_A 0x0A
#define GEO_REG_B 0x0B
#define GEO_REG_C 0x0C

// Xray registers
#define XRAY_REG_DOSE 0x12


// Write value to register
void I2C_writeRegister(uint8_t deviceAddr, uint8_t reg, uint8_t value)
{
    Wire.beginTransmission(deviceAddr);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

// Read value from register
uint8_t I2C_readRegister(uint8_t deviceAddr, uint8_t reg)
{
    Wire.beginTransmission(deviceAddr);
    Wire.write(reg);
    Wire.endTransmission(false);

    Wire.requestFrom(deviceAddr, (uint8_t)1);
    if (Wire.available())
    {
        return Wire.read();
    }
    return 0xFF;
}


#endif