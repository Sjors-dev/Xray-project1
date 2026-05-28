#include "centralAcquisition_defines.h"

// globale vars
volatile bool prepareButtonPressed  = false;
volatile bool prepareButtonReleased = false;
volatile bool acquireButtonPressed  = false;

bool geoPrepared  = false;
bool xrayPrepared = false;

bool sanXrayEnabled = false;  // SAN_XRAY_ENABLED 

unsigned long startTimePrepare    = 0;
unsigned long lastDebouncePrep    = 0;
unsigned long lastDebounceAcq     = 0;

bool lastPrepareButtonState = HIGH;
bool lastAcquireButtonState = HIGH;
bool prepareButtonState     = HIGH;
bool acquireButtonState     = HIGH;

State    currentState    = NOT_CONNECTED;
EXAMINATION_TYPES currentExamType = EXAM_TYPE_NONE;

// ISRs
void prepareISR()
{
    bool reading = (PIND & (1 << PREPARE_BUTTON)) != 0;

    if ((millis() - lastDebouncePrep) > DEBOUNCE_DELAY_MS)
    {
        if (reading == LOW && lastPrepareButtonState == HIGH)
        {
            prepareButtonPressed = true;
        }
        else if (reading == HIGH && lastPrepareButtonState == LOW)
        {
            prepareButtonReleased = true;
        }
        lastPrepareButtonState = reading;
        lastDebouncePrep = millis();
    }
}

void acquireISR()
{
    bool reading = (PIND & (1 << ACQUIRE_BUTTON)) != 0;
    if ((millis() - lastDebounceAcq) > DEBOUNCE_DELAY_MS)
    {
        acquireButtonPressed = (reading == LOW);
        lastAcquireButtonState = reading;
        lastDebounceAcq = millis();
    }
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

void respondAck()
{
    Serial.print("$ACK#");
}

void respondNack()
{
    Serial.print("$NACK#");
}

void sendDoseToPatientAdmin(uint8_t dose)
{
    Serial.print("$DOSE:");
    Serial.print(dose);
    Serial.print("#");
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

void I2C_sendExamType(uint8_t deviceAddr, EXAMINATION_TYPES type)
{
    uint8_t reg = I2C_readRegister(deviceAddr, REG_STATUS);
    uint8_t val = examTypeToI2C(type);
    reg = (reg & ~EXAM_TYPE_MASK) | ((val << EXAM_TYPE_SHIFT) & EXAM_TYPE_MASK);
    I2C_writeRegister(deviceAddr, REG_STATUS, reg);
}

bool I2C_isPrepared(uint8_t statusReg)
{
    return ((statusReg & STATE_MASK) == STATE_PREPARED);
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

void allLedsOff()
{
    controlLed(IDLE_LED_PIN,     false);
    controlLed(PREPARED_LED_PIN, false);
    controlLed(ACQUIRING_LED_PIN, false);
}

void handleLedStates()
{
    if (currentState == NOT_CONNECTED || currentState == IDLE)
    {
        // green on = no exam type set
        controlLed(IDLE_LED_PIN,      currentExamType == EXAM_TYPE_NONE);
        // red on = exam type IS set
        controlLed(PREPARED_LED_PIN,  currentExamType != EXAM_TYPE_NONE);
        controlLed(ACQUIRING_LED_PIN, false);
    }
    else if (currentState == PREPARING || currentState == PREPARED)
    {
        controlLed(IDLE_LED_PIN,      false);
        controlLed(PREPARED_LED_PIN,  true);   // Prepared Lamp on
        controlLed(ACQUIRING_LED_PIN, false);
    }
    else if (currentState == ACQUIRING)
    {
        controlLed(IDLE_LED_PIN,      false);
        controlLed(PREPARED_LED_PIN,  false);  // Prepared Lamp off
        controlLed(ACQUIRING_LED_PIN, true);   // Acquiring Lamp on
    }
}

EXAMINATION_TYPES readExamType(const String& cmd)
{
    if (cmd == "0") return EXAM_TYPE_SINGLE_SHOT;
    if (cmd == "1") return EXAM_TYPE_SERIES;
    if (cmd == "2") return EXAM_TYPE_SERIES_WITH_MOTION;
    if (cmd == "3") return EXAM_TYPE_FLUORO;
    return EXAM_TYPE_NONE;
}


// state handlers
void handlePrepare()
{
    if (currentExamType != EXAM_TYPE_NONE)
    {
        Serial.println("Prepare button pressed");

        I2C_sendPrepare(GEO_I2C_ADDRESS);
        I2C_sendPrepare(XRAY_I2C_ADDRESS);
        startTimePrepare = millis();

        currentState = PREPARING;
    }
}

void handleAcquire()
{
    if (currentState == PREPARED && currentExamType != EXAM_TYPE_NONE)
    {
        Serial.println("Acquire button pressed");

        // set SAN_XRAY_ENABLED high and track it
        PORTD |= (1 << SAN_XRAY_ENABLED_PIN);
        sanXrayEnabled = true;

        currentState = ACQUIRING;
    }
}

void handleAcquireDone()
{
    Serial.println("Acquire button released");

    // set SAN_XRAY_ENABLED low
    PORTD &= ~(1 << SAN_XRAY_ENABLED_PIN);
    sanXrayEnabled = false;

    // retrieve dose from XrayGenerator and pass to PatientAdministration
    uint8_t dose = I2C_getDose();
    sendDoseToPatientAdmin(dose);

    // reset exam type and pass it to Geometry and XrayGenerator
    currentExamType = EXAM_TYPE_NONE;
    I2C_sendExamType(GEO_I2C_ADDRESS,  currentExamType);
    I2C_sendExamType(XRAY_I2C_ADDRESS, currentExamType);

    currentState = IDLE;
}

void handleUnprepare()
{
    Serial.println("Prepare timeout reached. Unpreparing");

    I2C_sendUnprepare(GEO_I2C_ADDRESS);
    I2C_sendUnprepare(XRAY_I2C_ADDRESS);

    currentState = IDLE;
    PORTD &= ~(1 << SAN_XRAY_ENABLED_PIN);
    sanXrayEnabled = false;
}

void handlePrepared()
{
    // Prepared Lamp is handled in handleLedStates()
    Serial.println("System prepared");
}

void handleDisconnect()
{
    Serial.println("Disconnect received");

    allLedsOff();

    PORTD &= ~(1 << SAN_XRAY_ENABLED_PIN);
    sanXrayEnabled = false;

    currentExamType = EXAM_TYPE_NONE;
    currentState    = NOT_CONNECTED;
}

void handleEvent(const String& cmd)
{
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
        if (cmd == "DISCONNECT")
        {
            handleDisconnect();
            break;
        }

        if (cmd != "")
        {
            // exam type selection only valid in IDLE
            currentExamType = readExamType(cmd);
            I2C_sendExamType(GEO_I2C_ADDRESS,  currentExamType);
            I2C_sendExamType(XRAY_I2C_ADDRESS, currentExamType);
            respondAck();
        }
        break;

    case PREPARING:
    {
        if (cmd == "DISCONNECT")
        {
            handleDisconnect();
            break;
        }

        uint8_t geoState  = I2C_readRegister(GEO_I2C_ADDRESS,  REG_STATUS);
        uint8_t xrayState = I2C_readRegister(XRAY_I2C_ADDRESS, REG_STATUS);

        geoPrepared  = I2C_isPrepared(geoState);
        xrayPrepared = I2C_isPrepared(xrayState);

        if (geoPrepared && xrayPrepared)
        {
            handlePrepared();
            currentState = PREPARED;
        }
        else if (millis() - startTimePrepare >= PREPARE_TIMEOUT_MS)
        {
            handleUnprepare();
        }
        break;
    }

    case PREPARED:
        if (cmd == "DISCONNECT")
        {
            handleDisconnect();
            break;
        }
        break;

    case ACQUIRING:
        if (cmd == "DISCONNECT")
        {
            handleDisconnect();
            break;
        }
        break;
    }
}

// handle non-idle exam type cmd: reply NACK
void handleExamTypeCmdIfNotIdle(const String& cmd)
{
    if (cmd != "" && cmd != "CONNECT" && cmd != "DISCONNECT")
    {
        if (currentState != IDLE)
        {
            respondNack();
        }
    }
}

// setup & loop
void setup()
{
    Wire.begin();

    Serial.begin(9600);
    Serial.println("Hello World");

    // define outputs
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
    // prepare button pressed
    if (prepareButtonPressed == true && currentState == IDLE)
    {
        prepareButtonPressed = false;
        handlePrepare();
    }

    // prepare button released while preparing -> unprepare 
    if (prepareButtonReleased == true && currentState == PREPARING)
    {
        prepareButtonReleased = false;
        handleUnprepare();
    }
    prepareButtonReleased = false;  // clear in all other states

    // acquire button pressed while prepared -> start acquiring
    if (acquireButtonPressed == true && currentState == PREPARED)
    {
        handleAcquire();
    }

    // acquire button released while acquiring -> finish acquiring 
    if (acquireButtonPressed == false && currentState == ACQUIRING)
    {
        handleAcquireDone();
    }

    String cmd = readCmd();

    // if not in idle and an exam type cmd comes in, send NACK
    handleExamTypeCmdIfNotIdle(cmd);

    handleEvent(cmd);

    handleLedStates();
}
