#ifndef CENTRALACHQUISITION_DEFINES_H
#define CENTRALACHQUISITION_DEFINES_H

#include <Arduino.h>
#include <Wire.h>
#include <avr/io.h>
#include "../../Interface_I2C/Protocol_I2C.h"
#include "../../Interface_PatAdmin_CentralAcq/Protocol_PatientAdmin_CentralAcq.h"

// CentralAcquisition-specific pins
#define SAN_XRAY_ENABLED_PIN    4
#define PREPARE_BUTTON          2
#define ACQUIRE_BUTTON          3
#define IDLE_LED_PIN            5
#define PREPARED_LED_PIN        6
#define ACQUIRING_LED_PIN       7

// Timing
#define DEBOUNCE_DELAY_MS       20
#define PREPARE_TIMEOUT_MS      1000

// State machine
enum State
{
    NOT_CONNECTED,
    IDLE,
    PREPARING,
    PREPARED,
    ACQUIRING
};

// Convert EXAMINATION_TYPES enum value to the I2C register 
inline uint8_t examTypeToI2C(EXAMINATION_TYPES type)
{
    switch (type)
    {
        case EXAM_TYPE_SINGLE_SHOT:         return I2C_EXAM_SINGLE_SHOT;
        case EXAM_TYPE_SERIES:              return I2C_EXAM_SERIES;
        case EXAM_TYPE_SERIES_WITH_MOTION:  return I2C_EXAM_SERIES_WITH_MOTION;
        case EXAM_TYPE_FLUORO:              return I2C_EXAM_FLUORO;
        case EXAM_TYPE_NONE:
        default:                            return I2C_EXAM_NONE;
    }
}

// Globals
extern volatile bool prepareButtonPressed;
extern volatile bool prepareButtonReleased;
extern volatile bool acquireButtonPressed;
extern bool sanXrayEnabled;
extern bool geoPrepared;
extern bool xrayPrepared;
extern unsigned long startTimePrepare;
extern unsigned long lastDebouncePrep;
extern unsigned long lastDebounceAcq;
extern bool lastPrepareButtonState;
extern bool lastAcquireButtonState;
extern bool prepareButtonState;
extern bool acquireButtonState;
extern State             currentState;
extern EXAMINATION_TYPES currentExamType;

// ISRs
void prepareISR();
void acquireISR();

// Serial / protocol
String readCmd();
void respondConnect();
void respondAck();
void respondNack();
void sendDoseToPatientAdmin(uint8_t dose);

// I2C helpers (CentralAcq level)
void I2C_sendPrepare(uint8_t deviceAddr);
void I2C_sendUnprepare(uint8_t deviceAddr);
bool I2C_isPrepared(uint8_t statusReg);
void I2C_sendExamType(uint8_t deviceAddr, EXAMINATION_TYPES type);
uint8_t I2C_getDose();

// State handlers
void handlePrepare();
void handleAcquire();
void handleAcquireDone();
void handleUnprepare();
void handlePrepared();
void handleDisconnect();

// LEDs
void controlLed(uint8_t pin, bool on);
void allLedsOff();
void handleLedStates();

// Event dispatcher
void handleEvent(const String& cmd);
void handleExamTypeCmdIfNotIdle(const String& cmd);

#endif // CENTRALACHQUISITION_DEFINES_H