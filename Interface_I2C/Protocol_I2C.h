#ifndef PROTOCOL_I2C_H
#define PROTOCOL_I2C_H

#include <Arduino.h>
#include <Wire.h>

// I2C addresses
#define GEO_I2C_ADDRESS     0x10
#define XRAY_I2C_ADDRESS    0x20

// SAN bus pins
#define SAN_GEO_MOVING_PIN  8

// Registers
#define REG_CMD             0x08
#define REG_STATUS          0x09
#define REG_DOSE            0x12

// Commands written into bits [5:4] of REG_CMD
#define CMD_PREPARE         0x01
#define CMD_UNPREPARE       0x02
#define CMD_SHIFT           4

// State field bits [7:6] of REG_STATUS
#define STATE_IDLE          0x00
#define STATE_PREPARING     0x40
#define STATE_PREPARED      0x80
#define STATE_ACQUIRING     0xC0
#define STATE_MASK          0xC0

// Exam type field bits [4:2] of REG_STATUS
#define EXAM_TYPE_SHIFT     2
#define EXAM_TYPE_MASK      0x1C

// Exam type register values (I2C layer only no name clash with the enum)
#define I2C_EXAM_SINGLE_SHOT        0x00
#define I2C_EXAM_SERIES             0x01
#define I2C_EXAM_SERIES_WITH_MOTION 0x02
#define I2C_EXAM_FLUORO             0x03
#define I2C_EXAM_NONE               0x04

// Geometry registers
#define GEO_REG_A           0x0A
#define GEO_REG_B           0x0B
#define GEO_REG_C           0x0C

// XRay pulse count registers
#define XRAY_REG_PULSE_MAX_A    0x0A
#define XRAY_REG_PULSE_MAX_B    0x0B
#define XRAY_REG_PULSE_MAX_C    0x0C
#define XRAY_REG_PULSE_MAX_D    0x0D
#define XRAY_REG_PULSE_LOW_E    0x0E
#define XRAY_REG_PULSE_LOW_F    0x0F
#define XRAY_REG_PULSE_LOW_10   0x10
#define XRAY_REG_PULSE_LOW_11   0x11

// I2C helpers
inline void I2C_writeRegister(uint8_t deviceAddr, uint8_t reg, uint8_t value)
{
    Wire.beginTransmission(deviceAddr);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

inline uint8_t I2C_readRegister(uint8_t deviceAddr, uint8_t reg)
{
    Wire.beginTransmission(deviceAddr);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(deviceAddr, (uint8_t)1);
    if (Wire.available())
        return Wire.read();
    return 0xFF;
}

#endif // PROTOCOL_I2C_H