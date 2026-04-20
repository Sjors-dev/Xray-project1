#ifndef PROTOCOL_I2C_H
#define PROTOCOL_I2C_H

// I2C adressen van de slaves
#define GEO_I2C_ADDRESS  0x10
#define XRAY_I2C_ADDRESS 0x20

// SAN bus pins, zelfde nummer op alle drie de Arduino's
#define SAN_XRAY_ENABLED_PIN 7  // CentralAcq schrijft, Geo+Xray lezen
#define SAN_GEO_MOVING_PIN   8  // Geo schrijft, Xray leest

// Register adressen
#define REG_CMD    0x08
#define REG_STATUS 0x09

// Commands voor in bits van REG_CMD
#define CMD_PREPARE   0x01
#define CMD_UNPREPARE 0x02

// State bits van REG_STATUS
#define STATE_IDLE      0x00
#define STATE_PREPARING 0x40
#define STATE_PREPARED  0x80
#define STATE_ACQUIRING 0xC0
#define STATE_MASK      0xC0

// Examination type in bits van REG_STATUS
#define EXAM_SHIFT 3
#define EXAM_MASK  0x38

// Geometry runtime registers ($A t/m $C)
#define GEO_REG_A 0x0A
#define GEO_REG_B 0x0B
#define GEO_REG_C 0x0C

// XrayGenerator registers
#define XRAY_REG_DOSE 0x12

#endif