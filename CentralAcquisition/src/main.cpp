#include <Arduino.h>
#include <Wire.h>
#include <avr/io.h>
#include <time.h>

#include "../../Interface_I2C/Protocol_I2C.h"
#include "../../Interface_PatAdmin_CentralAcq/Protocol_PatientAdmin_CentralAcq.h"

// buttons
#define PREPARED_BUTTON 2
volatile bool prepareButtonPressed = false;

// preparing
bool geoPrepared = false;
bool xrayPrepared = false;

unsigned long startTimePrepare = 0;
unsigned long intervalPrepare = 1000; // 1 seconde

enum State
{
    NOT_CONNECTED,
    IDLE,
    PREPARING,
    PREPARED,
    ACQUIRING
};

enum ExamType
{
    SINGLE_SHOT,
    SERIES,
    SERIES_WITH_MOTION,
    FLUORO,
    NONE
};

ExamType currentExamType = NONE;
State currentState = NOT_CONNECTED;

void buttonISR()
{
    prepareButtonPressed = true;
}

String readCmd()
{
    if (Serial.available())
    {
        if (Serial.read() == '$')
        {
            String cmd = Serial.readStringUntil('#');
            return cmd;
        }
    }
    return "";
}

void respondConnect()
{
    Serial.print("$CONNECT#");
    return;
}

// I2C helpers
void I2C_sendPrepare(uint8_t deviceAddr)
{
    uint8_t cmd = (CMD_PREPARE << CMD_SHIFT);
    I2C_writeRegister(deviceAddr, REG_CMD, cmd);
}

void I2C_sendUnprepare(uint8_t deviceAddr)
{
    uint8_t cmd = (CMD_UNPREPARE << CMD_SHIFT);
    I2C_writeRegister(deviceAddr, REG_CMD, cmd);
}

bool I2C_isPrepared(uint8_t statusReg)
{
    return ((statusReg & STATE_MASK) == STATE_PREPARED);
}

// state handlers
void handlePrepare()
{
    if (currentExamType != NONE)
    {
        Serial.println("Prepare button pressed");

        I2C_sendPrepare(GEO_I2C_ADDRESS);
        I2C_sendPrepare(XRAY_I2C_ADDRESS);
        startTimePrepare = millis();

        currentState = PREPARING;
    }
}

void handleUnprepare()
{
    if (currentState == PREPARING)
    {
        Serial.println("Prepare timeout reached. Unpreparing");

        I2C_sendUnprepare(GEO_I2C_ADDRESS);
        I2C_sendUnprepare(XRAY_I2C_ADDRESS);

        currentState = IDLE;
        PORTD |= (1 << 7); // LED ON
    }
}

void handlePrepared()
{
    PORTD &= ~(1 << 7); // LED OFF
}

void handleLeds()
{
    if (currentExamType == NONE)
    {
        PORTD |= (1 << 7);
        PORTD &= ~(1 << 6);
    }
    else
    {
        PORTD |= (1 << 6);
        PORTD &= ~(1 << 7);
    }
}

void setup()
{
    Wire.begin();

    Serial.begin(9600);
    Serial.println("Hello World");

    DDRD |= (1 << PD7); // pin 7 output
    DDRD |= (1 << PD6); // pin 6 output

    pinMode(PREPARED_BUTTON, INPUT_PULLUP);

    // interrupt op FALLING edge (knop naar GND)
    attachInterrupt(digitalPinToInterrupt(PREPARED_BUTTON), buttonISR, FALLING);
}

void loop()
{
    if (prepareButtonPressed == true)
    {
        prepareButtonPressed = false;
        handlePrepare();
    }

    String cmd = readCmd();

    switch (currentState)
    {
    case NOT_CONNECTED:
        if (cmd == "CONNECT")
        {
            respondConnect();
            currentState = IDLE;
        }
        break;

    case IDLE:
        if (cmd == "0")
        {
            currentExamType = SINGLE_SHOT;
        }
        else if (cmd == "1")
        {
            currentExamType = SERIES;
        }
        else if (cmd == "2")
        {
            currentExamType = SERIES_WITH_MOTION;
        }
        else if (cmd == "3")
        {
            currentExamType = FLUORO;
        }
        else if (cmd == "4")
        {
            currentExamType = NONE;
        }
        break;

    case PREPARING:
    {
        uint8_t geoState = I2C_readRegister(GEO_I2C_ADDRESS, REG_STATUS);
        uint8_t xrayState = I2C_readRegister(XRAY_I2C_ADDRESS, REG_STATUS);

        geoPrepared = I2C_isPrepared(geoState);
        xrayPrepared = I2C_isPrepared(xrayState);

        if (geoPrepared && xrayPrepared)
        {
            currentState = PREPARED;
        }
        else if ((!geoPrepared || !xrayPrepared) && (millis() - startTimePrepare >= intervalPrepare))
        {
            handleUnprepare();
        }
        break;
    }

    case PREPARED:
    {
        handlePrepared();
        break;
    }

    case ACQUIRING:
        break;
    }

    handleLeds();
}