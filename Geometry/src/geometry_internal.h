#ifndef GEOMETRY_INTERNAL_H
#define GEOMETRY_INTERNAL_H

#include <Arduino.h>
#include <Wire.h>
#include "../../Interface_I2C/Protocol_I2C.h"
#include "../../Interface_PatAdmin_CentralAcq/Protocol_PatientAdmin_CentralAcq.h"


//  Pin definitions 
#define MOTOR_PWM_PIN        9
#define JOYSTICK_PIN         A0
#define FIXED_SPEED_BUTTON   2
#define SAN_XRAY_ENABLED_PIN    4


// SAN_XRAY_ENABLED_PIN 7  – defined in Protocol_I2C.h
// SAN_GEO_MOVING_PIN   8  – defined in Protocol_I2C.h

//  Timing 
#define JOYSTICK_DEADZONE           30   

#define PREPARE_WAIT_MIN_MS         100
#define PREPARE_WAIT_MAX_MS         400
#define PREPARE_MOTION_WAIT_MIN_MS  300
#define PREPARE_MOTION_WAIT_MAX_MS  800
#define UNPREPARE_WAIT_MIN_MS       0
#define UNPREPARE_WAIT_MAX_MS       200

//  I2C register file 
// reg 0x08 → index 0, reg 0x0C → index 4
#define GEO_REG_COUNT   5
#define GEO_REG_OFFSET  0x08   

//  State machine 
typedef enum
{
    GEO_IDLE      = 0,
    GEO_PREPARING = 1,
    GEO_PREPARED  = 2,
    GEO_ACQUIRING = 3
} GeoState;

//  Motor priority owner 
// 0 = none, 1 = fixed-speed button, 2 = joystick
#define MOTOR_OWNER_NONE    0
#define MOTOR_OWNER_BUTTON  1
#define MOTOR_OWNER_JOY     2

//  Global variables (defined in geometry.cpp) 
extern volatile uint8_t registers[GEO_REG_COUNT];
extern volatile uint8_t regPointer;

extern GeoState           currentState;
extern EXAMINATION_TYPES  currentExamType;

extern bool           waitingForPrepare;
extern bool           waitingForUnprepare;
extern unsigned long  waitStartTime;
extern unsigned long  waitDuration;

extern unsigned long  motorRunStartMs;
extern bool           motorRunning;
extern uint32_t       motorTotalSec;
extern uint8_t        motorPriorityOwner;

//  Function prototypes 
uint8_t      examTypeToI2C(EXAMINATION_TYPES t);
void         updateStatusRegister();
void         updateRuntimeRegisters();
void         driveMotor(uint8_t speed);

void         onReceive(int numBytes);
void         onRequest();

void         handlePreparingState();
void         handleUnprepareState();
void         handleAcquiringState();
void         handleMotorControl();

#endif // GEOMETRY_INTERNAL_H