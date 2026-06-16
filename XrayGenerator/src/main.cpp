#include <Arduino.h>
#include <Wire.h>
#include "xray_internal.h"

// ---------------- STATE ----------------
XrayState currentState = XRAY_IDLE;

uint8_t reg_status = 0;

uint32_t cumulativeDose = 0;

unsigned long stateStartTime = 0;
unsigned long waitDuration = 0;
unsigned long lastPulseTime = 0;

// ---------------- PULSE ENGINE ----------------
bool pulseActive = false;
unsigned long pulseStartTime = 0;
unsigned long pulseDuration = 0;
uint8_t pulsePower = 0;

// ---------------- DOSE ----------------
int doseSamples[MA_SIZE] = {0};
int sampleIndex = 0;

// ---------------- DOSE UPDATE ----------------
void updateDose()
{
    int val = analogRead(DOSE_LDR_PIN);

    doseSamples[sampleIndex] = val;
    sampleIndex = (sampleIndex + 1) % MA_SIZE;

    long sum = 0;
    for (int i = 0; i < MA_SIZE; i++)
        sum += doseSamples[i];

    cumulativeDose += (sum / MA_SIZE);
}

// ---------------- PULSE ENGINE ----------------
void startPulse(uint16_t duration, uint8_t power)
{
    pulseActive = true;
    pulseStartTime = millis();
    pulseDuration = duration;
    pulsePower = power;
}

void updatePulseEngine()
{
    if (!pulseActive) return;

    analogWrite(XRAY_LED_PIN, pulsePower);

    updateDose();

    if (millis() - pulseStartTime >= pulseDuration)
    {
        analogWrite(XRAY_LED_PIN, 0);
        pulseActive = false;
    }
}

// ---------------- STATES ----------------
void handlePreparingState()
{
    if (millis() - stateStartTime >= waitDuration)
    {
        currentState = (waitDuration > 200) ? XRAY_IDLE : XRAY_PREPARED;
    }
}

void handlePreparedState(bool xrayEnabled, bool geoMoving, uint8_t examType)
{
    if (!xrayEnabled) return;

    if (examType == I2C_EXAM_SERIES_WITH_MOTION)
    {
        if (geoMoving) currentState = XRAY_ACQUIRING;
    }
    else if (examType != I2C_EXAM_NONE)
    {
        currentState = XRAY_ACQUIRING;
    }
}

void handleAcquiringState(bool xrayEnabled, bool geoMoving, uint8_t examType)
{
    if (!xrayEnabled || (examType == I2C_EXAM_SERIES_WITH_MOTION && !geoMoving))
    {
        currentState = XRAY_IDLE;
        return;
    }

    unsigned long now = millis();

    if (pulseActive) return;

    switch (examType)
    {
        case I2C_EXAM_SINGLE_SHOT:
            startPulse(10, 255);
            currentState = XRAY_IDLE;
            break;

        case I2C_EXAM_SERIES:
        case I2C_EXAM_SERIES_WITH_MOTION:
            if (now - lastPulseTime >= 500)
            {
                startPulse(10, 255);
                lastPulseTime = now;
            }
            break;

        case I2C_EXAM_FLUORO:
            if (now - lastPulseTime >= 250)
            {
                startPulse(10, 25);
                lastPulseTime = now;
            }
            break;
    }
}

// ---------------- STATE MACHINE ----------------
void handleStateMachine(bool xrayEnabled, bool geoMoving, uint8_t examType)
{
    switch (currentState)
    {
        case XRAY_PREPARING:
            handlePreparingState();
            break;

        case XRAY_PREPARED:
            handlePreparedState(xrayEnabled, geoMoving, examType);
            break;

        case XRAY_ACQUIRING:
            handleAcquiringState(xrayEnabled, geoMoving, examType);
            break;

        default:
            break;
    }
}

// ---------------- I2C ----------------
void processI2CReceive(int howMany)
{
    while (Wire.available())
    {
        uint8_t reg = Wire.read();

        if (reg == REG_CMD)
        {
            uint8_t val = Wire.read();
            uint8_t cmd = (val >> CMD_SHIFT) & 0x03;

            if (cmd == CMD_PREPARE && currentState == XRAY_IDLE)
            {
                currentState = XRAY_PREPARING;
                cumulativeDose = 0;
                stateStartTime = millis();
                waitDuration = random(100, 201);
            }
            else if (cmd == CMD_UNPREPARE)
            {
                currentState = XRAY_PREPARING;
                stateStartTime = millis();
                waitDuration = random(200, 401);
            }
        }
        else if (reg == REG_STATUS)
        {
            reg_status = Wire.read();
        }
    }
}

void processI2CRequest()
{
    reg_status = (reg_status & ~STATE_MASK) | currentState;
    Wire.write(reg_status);
}

// ---------------- SETUP ----------------
void setup()
{
    Wire.begin(XRAY_I2C_ADDRESS);
    Wire.onReceive(processI2CReceive);
    Wire.onRequest(processI2CRequest);
}

// ---------------- LOOP ----------------
void loop()
{
    bool xrayEnabled = (I2C_readRegister(GEO_I2C_ADDRESS, REG_STATUS) & 0x01);
    bool geoMoving   = I2C_readRegister(GEO_I2C_ADDRESS, SAN_GEO_MOVING_PIN);

    uint8_t examType = (reg_status & EXAM_TYPE_MASK) >> EXAM_TYPE_SHIFT;

    handleStateMachine(xrayEnabled, geoMoving, examType);
    updatePulseEngine();
}