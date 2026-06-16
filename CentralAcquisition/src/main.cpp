#include "centralAcquisition_internal.h"

// Globals
volatile bool prepareButtonPressed = false;
volatile bool acquireButtonPressed = false;

bool sanXrayEnabled = false;
bool geoPrepared = false;
bool xrayPrepared = false;

unsigned long startTimePrepare = 0;
unsigned long lastDebouncePrep = 0;
unsigned long lastDebounceAcq = 0;

bool lastPrepareButtonState = HIGH;
bool lastAcquireButtonState = HIGH;

State currentState = NOT_CONNECTED;
EXAMINATION_TYPES currentExamType = EXAM_TYPE_NONE;


void prepareISR()
{
    prepareButtonPressed = true;
}

void acquireISR()
{
    acquireButtonPressed = true;
}

// Serial
String readCmd()
{
    if (Serial.available())
    {
        if (Serial.read() == '$')
            return Serial.readStringUntil('#');
    }
    return "";
}

void respondConnect() { Serial.print("$CONNECT#"); }
void respondAck() { Serial.print("$ACK#"); }
void respondNack() { Serial.print("$NACK#"); }

void sendDoseToPatientAdmin(uint8_t dose)
{
    Serial.print("$DOSE:");
    Serial.print(dose);
    Serial.print("#");
}

// I2C
void I2C_sendPrepare(uint8_t deviceAddr)
{
    I2C_writeRegister(deviceAddr, REG_CMD, CMD_PREPARE << CMD_SHIFT);
}

void I2C_sendUnprepare(uint8_t deviceAddr)
{
    I2C_writeRegister(deviceAddr, REG_CMD, CMD_UNPREPARE << CMD_SHIFT);
}

void I2C_sendExamType(uint8_t deviceAddr, EXAMINATION_TYPES type)
{
    uint8_t reg = I2C_readRegister(deviceAddr, REG_STATUS);
    uint8_t val = examTypeToI2C(type);
    reg = (reg & ~EXAM_TYPE_MASK) | ((val << EXAM_TYPE_SHIFT) & EXAM_TYPE_MASK);
    I2C_writeRegister(deviceAddr, REG_STATUS, reg);
}

bool I2C_isPrepared(uint8_t statusReg)
{
    return (statusReg & STATE_MASK) == STATE_PREPARED;
}

uint8_t I2C_getDose()
{
    return I2C_readRegister(XRAY_I2C_ADDRESS, REG_DOSE);
}

// LEDs
void controlLed(uint8_t pin, bool on)
{
    if (pin <= 7)
    {
        if (on)
            PORTD |= (1 << pin);
        else
            PORTD &= ~(1 << pin);
    }
    else if (pin <= 13)
    {
        if (on)
            PORTB |= (1 << (pin - 8));
        else
            PORTB &= ~(1 << (pin - 8));
    }
    else if (pin <= 19)
    {
        if (on)
            PORTC |= (1 << (pin - 14));
        else
            PORTC &= ~(1 << (pin - 14));
    }
}

void allLedsOff()
{
    controlLed(IDLE_LED_PIN, false);
    controlLed(PREPARED_LED_PIN, false);
    controlLed(ACQUIRING_LED_PIN, false);
}

void handleLedStates()
{
    if (currentState == NOT_CONNECTED)
    {
        allLedsOff();
    }
    else if (currentState == IDLE)
    {
        controlLed(IDLE_LED_PIN, currentExamType == EXAM_TYPE_NONE);
        controlLed(PREPARED_LED_PIN, currentExamType != EXAM_TYPE_NONE);
        controlLed(ACQUIRING_LED_PIN, false);
    }
    else if (currentState == PREPARING || currentState == PREPARED)
    {
        controlLed(IDLE_LED_PIN, false);
        controlLed(PREPARED_LED_PIN, true);
        controlLed(ACQUIRING_LED_PIN, false);
    }
    else if (currentState == ACQUIRING)
    {
        controlLed(IDLE_LED_PIN, false);
        controlLed(PREPARED_LED_PIN, false);
        controlLed(ACQUIRING_LED_PIN, true);
    }
}

// State action handlers
void startPreparing()
{
    I2C_sendPrepare(GEO_I2C_ADDRESS);
    I2C_sendPrepare(XRAY_I2C_ADDRESS);
    startTimePrepare = millis();
}

void handleAcquire()
{
    Serial.println("Acquire button pressed");
    controlLed(IDLE_LED_PIN, false);
    controlLed(PREPARED_LED_PIN, false);
    controlLed(ACQUIRING_LED_PIN, true);
    PORTD |= (1 << SAN_XRAY_ENABLED_PIN);
    sanXrayEnabled = true;
}

void handleAcquireDone()
{
    Serial.println("Acquire button released");
    PORTD &= ~(1 << SAN_XRAY_ENABLED_PIN);
    sanXrayEnabled = false;

    uint8_t dose = I2C_getDose();
    sendDoseToPatientAdmin(dose);

    currentExamType = EXAM_TYPE_NONE;
    I2C_sendExamType(GEO_I2C_ADDRESS, currentExamType);
    I2C_sendExamType(XRAY_I2C_ADDRESS, currentExamType);

}

void cancelPreparing()
{
    Serial.println("Unpreparing");
    I2C_sendUnprepare(GEO_I2C_ADDRESS);
    I2C_sendUnprepare(XRAY_I2C_ADDRESS);
    PORTD &= ~(1 << SAN_XRAY_ENABLED_PIN);
    sanXrayEnabled = false;
}

void onPrepareComplete()
{
    Serial.println("System prepared");
    controlLed(IDLE_LED_PIN, false);
    controlLed(PREPARED_LED_PIN, true);
    controlLed(ACQUIRING_LED_PIN, false);
}

void handleDisconnect()
{
    Serial.println("Disconnect received");
    allLedsOff();
    PORTD &= ~(1 << SAN_XRAY_ENABLED_PIN);
    sanXrayEnabled = false;
    currentExamType = EXAM_TYPE_NONE;
}

void handleButtonEvents()
{
    //  Prepare button
    if (prepareButtonPressed)
    {
        prepareButtonPressed = false;

        bool reading = (PIND & (1 << PREPARE_BUTTON)) != 0;
        if ((millis() - lastDebouncePrep) > DEBOUNCE_DELAY_MS)
        {
            lastDebouncePrep = millis();

            if (reading == LOW && lastPrepareButtonState == HIGH)
            {
                lastPrepareButtonState = LOW;
                handleEvent(EVT_PREPARE_PRESSED);
            }
            else if (reading == HIGH && lastPrepareButtonState == LOW)
            {
                lastPrepareButtonState = HIGH;
                handleEvent(EVT_PREPARE_RELEASED);
            }
        }
    }

    // Acquire button
    if (acquireButtonPressed)
    {
        acquireButtonPressed = false;

        bool reading = (PIND & (1 << ACQUIRE_BUTTON)) != 0;
        if ((millis() - lastDebounceAcq) > DEBOUNCE_DELAY_MS)
        {
            lastDebounceAcq = millis();

            if (reading == LOW && lastAcquireButtonState == HIGH)
            {
                lastAcquireButtonState = LOW;
                handleEvent(EVT_ACQUIRE_PRESSED);
            }
            else if (reading == HIGH && lastAcquireButtonState == LOW)
            {
                lastAcquireButtonState = HIGH;
                handleEvent(EVT_ACQUIRE_RELEASED);
            }
        }
    }
}

void handleSerialEvent(const String &cmd)
{
    if (cmd == "")
        return;

    if (cmd == "CONNECT")
    {
        handleEvent(EVT_CMD_CONNECT);
    }
    else if (cmd == "DISCONNECT")
    {
        handleEvent(EVT_CMD_DISCONNECT);
    }
    else
    {
        if (cmd == "0")
            currentExamType = EXAM_TYPE_SINGLE_SHOT;
        else if (cmd == "1")
            currentExamType = EXAM_TYPE_SERIES;
        else if (cmd == "2")
            currentExamType = EXAM_TYPE_SERIES_WITH_MOTION;
        else if (cmd == "3")
            currentExamType = EXAM_TYPE_FLUORO;
        else
            currentExamType = EXAM_TYPE_NONE;

        handleEvent(EVT_CMD_EXAM_TYPE);
    }
}

void handleEvent(Event evt)
{
    switch (currentState)
    {
    case NOT_CONNECTED:
        if (evt == EVT_CMD_CONNECT)
        {
            respondConnect();
            currentState = IDLE;
        }
        break;

    case IDLE:
        if (evt == EVT_CMD_DISCONNECT)
        {
            handleDisconnect();
            currentState = NOT_CONNECTED;
        }
        else if (evt == EVT_CMD_EXAM_TYPE)
        {
            I2C_sendExamType(GEO_I2C_ADDRESS, currentExamType);
            I2C_sendExamType(XRAY_I2C_ADDRESS, currentExamType);
            respondAck();
        }
        else if (evt == EVT_PREPARE_PRESSED)
        {
            if (currentExamType != EXAM_TYPE_NONE)
                startPreparing();
                currentState = PREPARING;
        }
        break;

    case PREPARING:
        if (evt == EVT_PREPARE_RELEASED)
            cancelPreparing();
            currentState = IDLE;
        else if (evt == EVT_PREPARE_PRESSED)
            checkPreparingStatus(); 
        else if (evt == EVT_CMD_DISCONNECT)
            handleDisconnect();
        else if (evt == EVT_CMD_EXAM_TYPE)
            respondNack();
        break;

    case PREPARED:
        if (evt == EVT_CMD_DISCONNECT)
            handleDisconnect();
        else if (evt == EVT_CMD_EXAM_TYPE)
            respondNack();
        else if (evt == EVT_ACQUIRE_PRESSED)
        {
            if (currentExamType != EXAM_TYPE_NONE)
                handleAcquire();
                currentState = ACQUIRING;
        }
        break;

    case ACQUIRING:
        if (evt == EVT_CMD_DISCONNECT)
            handleDisconnect();
        else if (evt == EVT_CMD_EXAM_TYPE)
            respondNack();
        else if (evt == EVT_ACQUIRE_RELEASED)
            handleAcquireDone();
            currentState = IDLE;
        break;
    }
}

void checkPreparingStatus()
{
    if (currentState != PREPARING)
        return;

    uint8_t geoState = I2C_readRegister(GEO_I2C_ADDRESS, REG_STATUS);
    uint8_t xrayState = I2C_readRegister(XRAY_I2C_ADDRESS, REG_STATUS);

    geoPrepared = I2C_isPrepared(geoState);
    xrayPrepared = I2C_isPrepared(xrayState);

    if (geoPrepared && xrayPrepared)
        onPrepareComplete();
        currentState = PREPARED;
    else if (millis() - startTimePrepare >= PREPARE_TIMEOUT_MS)
        cancelPreparing();
}

// Setup & loop
void setup()
{
    Wire.begin();
    Serial.begin(9600);
    Serial.println("Hello World");

    DDRD |= (1 << SAN_XRAY_ENABLED_PIN);
    DDRD |= (1 << PREPARED_LED_PIN);
    DDRD |= (1 << IDLE_LED_PIN);
    DDRD |= (1 << ACQUIRING_LED_PIN);

    pinMode(PREPARE_BUTTON, INPUT_PULLUP);
    pinMode(ACQUIRE_BUTTON, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(PREPARE_BUTTON), prepareISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ACQUIRE_BUTTON), acquireISR, CHANGE);
}

void loop()
{
    handleButtonEvents();
    handleSerialEvent(readCmd());
    handleLedStates();


    //handleEvent(getEvent());
}