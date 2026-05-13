#include <Arduino.h>
#include <Wire.h>
#include <avr/io.h>
#include "../../Interface_I2C/Protocol_I2C.h"
#include "../../Interface_PatAdmin_CentralAcq/Protocol_PatientAdmin_CentralAcq.h"


// pins
#define IDLE_LED_PIN        5
#define PREPARED_LED_PIN    6
#define PREPARE_BUTTON      2
#define ACQUIRE_BUTTON      3


// timing
#define PREPARE_TIMEOUT_MS  1000



// enums
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


// globals
extern volatile bool prepareButtonPressed;
extern volatile bool acquireButtonPressed;
extern bool geoPrepared;
extern bool xrayPrepared;
extern unsigned long startTimePrepare;
extern State currentState;
extern ExamType currentExamType;

// ISRs
void prepareISR();
void acquireISR();
void stopAcquireISR();

// serial
String readCmd();
void respondConnect();

// I2C
void I2C_sendPrepare(uint8_t deviceAddr);
void I2C_sendUnprepare(uint8_t deviceAddr);
bool I2C_isPrepared(uint8_t statusReg);


// state handlers
void handlePrepare();
void handleAcquire();
void handleUnprepare();
void handlePrepared();
void handleAcquireRelease();           

// LEDs
void handleLedStates();
void controlLed(uint8_t pin, bool on);