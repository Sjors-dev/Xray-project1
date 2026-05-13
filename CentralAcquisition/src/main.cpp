#include "centralAcquisition_defines.h"

// globale vars
volatile bool prepareButtonPressed = false;
volatile bool acquireButtonPressed = false;

bool geoPrepared = false;
bool xrayPrepared = false;

unsigned long startTimePrepare = 0;

State    currentState    = NOT_CONNECTED;
ExamType currentExamType = NONE;

// ISRs
void prepareISR() { prepareButtonPressed = true; }

void acquireISR() {
    acquireButtonPressed = !acquireButtonPressed;
}


// serial
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
}

// I2C
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

// LEDs
void controlLed(uint8_t pin, bool on)
{
    if (pin <= 7)
    {
        if (on) { PORTD |= (1 << pin); }
        else    { PORTD &= ~(1 << pin); }
    }
    else if (pin <= 13)
    {
        if (on) { PORTB |= (1 << (pin - 8)); }
        else    { PORTB &= ~(1 << (pin - 8)); }
    }
    else if (pin <= 19)
    {
        if (on) { PORTC |= (1 << (pin - 14)); }
        else    { PORTC &= ~(1 << (pin - 14)); }
    }
}

void handleLedStates()
{
    if (currentExamType == NONE)
    {
        controlLed(IDLE_LED_PIN, true);
        controlLed(PREPARED_LED_PIN, false);
    }
    else if (currentState == PREPARED || currentState == PREPARING)
    {
        controlLed(PREPARED_LED_PIN, true);
        controlLed(IDLE_LED_PIN, false);
    }
    else if(SAN_XRAY_ENABLED_PIN == HIGH)
    {
        // pin 6 (SAN_XRAY_ENABLED_PIN) hoog is automatisch led aan. 
        controlLed(IDLE_LED_PIN, false);
        controlLed(PREPARED_LED_PIN, false);
        
    }
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
        digitalWrite(SAN_XRAY_ENABLED_PIN, HIGH);
    }
}

void handleAcquire()
{
    if (currentState == PREPARED && currentExamType != NONE)
    {
        Serial.println("Acquire button pressed");
        currentState = ACQUIRING;
    }
}

void handleUnprepare()
{
    Serial.println("Prepare timeout reached. Unpreparing");

    I2C_sendUnprepare(GEO_I2C_ADDRESS);
    I2C_sendUnprepare(XRAY_I2C_ADDRESS);

    currentState = IDLE;
    PORTD |= (1 << SAN_XRAY_ENABLED_PIN);
}

void handlePrepared()
{
    PORTD &= ~(1 << SAN_XRAY_ENABLED_PIN);
}

// setup & loop
void setup()
{
    Wire.begin();

    Serial.begin(9600);
    Serial.println("Hello World");

    //define outputs
    DDRD |= (1 << SAN_XRAY_ENABLED_PIN);
    DDRD |= (1 << PREPARED_LED_PIN);
    DDRD |= (1 << IDLE_LED_PIN);

    pinMode(PREPARE_BUTTON, INPUT_PULLUP);
    pinMode(ACQUIRE_BUTTON, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(PREPARE_BUTTON), prepareISR, FALLING);
    attachInterrupt(digitalPinToInterrupt(ACQUIRE_BUTTON), acquireISR, CHANGE);
}

void loop()
{
    if (prepareButtonPressed == true)
    {
        prepareButtonPressed = false;
        handlePrepare();
    }
    else if (acquireButtonPressed == true && currentState == PREPARED)
    {
        acquireButtonPressed = false;
        handleAcquire();
    }

    else if(acquireButtonPressed == false && currentState == ACQUIRING){
        //LOGIC
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
        uint8_t geoState  = I2C_readRegister(GEO_I2C_ADDRESS,  REG_STATUS);
        uint8_t xrayState = I2C_readRegister(XRAY_I2C_ADDRESS, REG_STATUS);

        geoPrepared  = I2C_isPrepared(geoState);
        xrayPrepared = I2C_isPrepared(xrayState);

        if (geoPrepared && xrayPrepared)
        {
            currentState = PREPARED;
        }
        else if ((!geoPrepared || !xrayPrepared) && (millis() - startTimePrepare >= PREPARE_TIMEOUT_MS))
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

    handleLedStates();
}