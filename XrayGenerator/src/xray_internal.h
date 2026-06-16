#ifndef XRAY_INTERNAL_H
#define XRAY_INTERNAL_H

#include <Arduino.h>
#include <Wire.h>
#include "../../Interface_I2C/Protocol_I2C.h"
#include "../../Interface_PatAdmin_CentralAcq/Protocol_PatientAdmin_CentralAcq.h"

// Pin definitions
#define XRAY_LED_PIN         9
#define DOSE_LDR_PIN         A0

// Timing
#define PREPARE_WAIT_MIN_MS     100
#define PREPARE_WAIT_MAX_MS     200
#define UNPREPARE_WAIT_MIN_MS   200
#define UNPREPARE_WAIT_MAX_MS   400

#define PULSE_DURATION_MS       10
#define SERIES_INTERVAL_MS      490
#define FLUORO_INTERVAL_MS      240

#define FLUORO_POWER_PWM        25

// Moving average
#define MA_SIZE 5

// I2C registers
#define XRAY_REG_COUNT  11
#define XRAY_REG_OFFSET  0x08

#define IDX_CMD          0
#define IDX_STATUS       1
#define IDX_DOSE         10

// State machine
typedef enum
{
    XRAY_IDLE = 0,
    XRAY_PREPARING = 1,
    XRAY_PREPARED = 2,
    XRAY_ACQUIRING = 3
} XrayState;

// Globals (defined in cpp)
extern volatile uint8_t registers[XRAY_REG_COUNT];
extern volatile uint8_t regPointer;

extern XrayState currentState;

extern uint8_t reg_status;

extern uint32_t cumulativeDose;

extern unsigned long stateStartTime;
extern unsigned long waitDuration;
extern unsigned long lastPulseTime;

// Pulse engine
extern bool pulseActive;
extern unsigned long pulseStartTime;
extern unsigned long pulseDuration;
extern uint8_t pulsePower;

// Dose
extern int doseSamples[MA_SIZE];
extern int sampleIndex;

// Functions
void updateDose();
void startPulse(uint16_t duration, uint8_t power);
void updatePulseEngine();

void handlePreparingState();
void handlePreparedState(bool xrayEnabled, bool geoMoving, uint8_t examType);
void handleAcquiringState(bool xrayEnabled, bool geoMoving, uint8_t examType);

void handleStateMachine(bool xrayEnabled, bool geoMoving, uint8_t examType);

void processI2CReceive(int howMany);
void processI2CRequest();

#endif