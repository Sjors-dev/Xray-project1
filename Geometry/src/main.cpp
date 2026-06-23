#include "geometry_internal.h"

//  Global variables 
volatile uint8_t registers[GEO_REG_COUNT];
volatile uint8_t regPointer = 0;

GeoState          currentState    = GEO_IDLE;
EXAMINATION_TYPES currentExamType = EXAM_TYPE_NONE;

bool          waitingForPrepare   = false;
bool          waitingForUnprepare = false;
unsigned long waitStartTime       = 0;
unsigned long waitDuration        = 0;

unsigned long motorRunStartMs  = 0;
bool          motorRunning     = false;
uint32_t      motorTotalSec    = 0;
uint8_t       motorPriorityOwner = MOTOR_OWNER_NONE;


//  Helpers 

uint8_t examTypeToI2C(EXAMINATION_TYPES t)
{
    switch (t)
    {
        case EXAM_TYPE_SINGLE_SHOT:        return I2C_EXAM_SINGLE_SHOT;
        case EXAM_TYPE_SERIES:             return I2C_EXAM_SERIES;
        case EXAM_TYPE_SERIES_WITH_MOTION: return I2C_EXAM_SERIES_WITH_MOTION;
        case EXAM_TYPE_FLUORO:             return I2C_EXAM_FLUORO;
        default:                           return I2C_EXAM_NONE;
    }
}

void updateStatusRegister()
{
    uint8_t stateBits = 0;
    switch (currentState)
    {
        case GEO_IDLE:      stateBits = STATE_IDLE;      break;
        case GEO_PREPARING: stateBits = STATE_PREPARING; break;
        case GEO_PREPARED:  stateBits = STATE_PREPARED;  break;
        case GEO_ACQUIRING: stateBits = STATE_ACQUIRING; break;
    }
    uint8_t examBits = (examTypeToI2C(currentExamType) << EXAM_TYPE_SHIFT) & EXAM_TYPE_MASK;
    registers[1] = stateBits | examBits;
}

void updateRuntimeRegisters()
{
    uint32_t totalSec  = motorTotalSec;
    uint8_t  secs      = totalSec % 60;
    uint32_t totalMin  = totalSec / 60;
    uint8_t  minLow    = totalMin & 0x03;
    uint8_t  minHigh   = (totalMin >> 2) & 0x3F;
    uint32_t totalHour = totalMin / 60;
    uint8_t  hourLow   = totalHour & 0x0F;
    uint8_t  hourHigh  = (totalHour >> 4) & 0xFF;

    registers[2] = (secs << 2) | (minLow & 0x03);
    registers[3] = (minHigh << 2) | (hourLow & 0x03);
    registers[4] = hourHigh;
}

void driveMotor(uint8_t speed)
{
    analogWrite(MOTOR_PWM_PIN, speed);

    if (speed > 0 && !motorRunning)
    {
        motorRunStartMs = millis();
        motorRunning    = true;
    }
    else if (speed == 0 && motorRunning)
    {
        motorTotalSec += (millis() - motorRunStartMs) / 1000UL;
        motorRunning   = false;
        updateRuntimeRegisters();
    }
}


//  I2C handlers 

void onReceive(int numBytes)
{
    if (numBytes < 1) return;

    uint8_t firstByte = Wire.read();
    numBytes--;

    regPointer = firstByte - GEO_REG_OFFSET;

    if (numBytes == 0) return;

    while (numBytes-- > 0)
    {
        uint8_t data = Wire.read();

        if (regPointer == 0)   // REG_CMD
        {
            uint8_t cmd = (data >> CMD_SHIFT) & 0x03;

            if (cmd == CMD_PREPARE && currentState == GEO_IDLE)
            {
                uint16_t minMs = PREPARE_WAIT_MIN_MS;
                uint16_t maxMs = PREPARE_WAIT_MAX_MS;
                if (currentExamType == EXAM_TYPE_SERIES_WITH_MOTION)
                {
                    minMs = PREPARE_MOTION_WAIT_MIN_MS;
                    maxMs = PREPARE_MOTION_WAIT_MAX_MS;
                }
                waitDuration        = minMs + random(maxMs - minMs + 1);
                waitStartTime       = millis();
                waitingForPrepare   = true;
                waitingForUnprepare = false;
                currentState        = GEO_PREPARING;
                updateStatusRegister();
            }
            else if (cmd == CMD_UNPREPARE &&
                     (currentState == GEO_PREPARING || currentState == GEO_PREPARED))
            {
                waitDuration        = UNPREPARE_WAIT_MIN_MS + random(UNPREPARE_WAIT_MAX_MS + 1);
                waitStartTime       = millis();
                waitingForUnprepare = true;
                waitingForPrepare   = false;
            }

            registers[0] = data;
        }
        else if (regPointer == 1)   // REG_STATUS 
        {
            uint8_t et = (data & EXAM_TYPE_MASK) >> EXAM_TYPE_SHIFT;
            switch (et)
            {
                case I2C_EXAM_SINGLE_SHOT:        currentExamType = EXAM_TYPE_SINGLE_SHOT;        break;
                case I2C_EXAM_SERIES:             currentExamType = EXAM_TYPE_SERIES;             break;
                case I2C_EXAM_SERIES_WITH_MOTION: currentExamType = EXAM_TYPE_SERIES_WITH_MOTION; break;
                case I2C_EXAM_FLUORO:             currentExamType = EXAM_TYPE_FLUORO;             break;
                default:                          currentExamType = EXAM_TYPE_NONE;               break;
            }
            updateStatusRegister();
        }

        regPointer++;
        if (regPointer >= GEO_REG_COUNT) regPointer = GEO_REG_COUNT - 1;
    }
}

void onRequest()
{
    if (regPointer < GEO_REG_COUNT)
        Wire.write(registers[regPointer]);
    else
        Wire.write(0xFF);
}


//  State handlers 

void handlePreparingState()
{
    if (!waitingForPrepare) return;
    if (millis() - waitStartTime >= waitDuration)
    {
        waitingForPrepare = false;
        currentState      = GEO_PREPARED;
        updateStatusRegister();
    }
}

void handleUnprepareState()
{
    if (!waitingForUnprepare) return;
    if (millis() - waitStartTime >= waitDuration)
    {
        waitingForUnprepare = false;
        currentState        = GEO_IDLE;
        driveMotor(0);
        PORTB &= ~(1 << (SAN_GEO_MOVING_PIN - 8));
        updateStatusRegister();
    }
}

void handleAcquiringState()
{
    if (currentExamType != EXAM_TYPE_SERIES_WITH_MOTION) return;

    bool xrayEnabled = (PIND & (1 << SAN_XRAY_ENABLED_PIN)) != 0;
    bool geoMoving   = (PORTB & (1 << (SAN_GEO_MOVING_PIN - 8))) != 0;

    if (currentState == GEO_PREPARED && xrayEnabled)
    {
        bool fixedPressed = (digitalRead(FIXED_SPEED_BUTTON) == LOW);
        bool joyActive    = abs(analogRead(JOYSTICK_PIN) - 512) > JOYSTICK_DEADZONE;
        bool enableMove   = fixedPressed || joyActive;

        if (enableMove && !geoMoving)
        {
            PORTB |= (1 << (SAN_GEO_MOVING_PIN - 8));
            currentState = GEO_ACQUIRING;
            updateStatusRegister();
        }
    }
    else if (currentState == GEO_ACQUIRING)
    {
        if (!xrayEnabled)
        {
            PORTB &= ~(1 << (SAN_GEO_MOVING_PIN - 8));
            currentState = GEO_IDLE;
            updateStatusRegister();
        }
    }
}

void handleMotorControl()
{
    bool fixedPressed = (digitalRead(FIXED_SPEED_BUTTON) == LOW);
    int  joyRaw       = analogRead(JOYSTICK_PIN);
    bool joyActive    = abs(joyRaw - 512) > JOYSTICK_DEADZONE;

    if (!fixedPressed && !joyActive)
    {
        motorPriorityOwner = MOTOR_OWNER_NONE;
        driveMotor(0);

        if (currentState == GEO_ACQUIRING)
        {
            PORTB &= ~(1 << (SAN_GEO_MOVING_PIN - 8));
            currentState = GEO_IDLE;
            updateStatusRegister();
        }
        return;
    }

    if (motorPriorityOwner == MOTOR_OWNER_NONE)
        motorPriorityOwner = fixedPressed ? MOTOR_OWNER_BUTTON : MOTOR_OWNER_JOY;

    if (motorPriorityOwner == MOTOR_OWNER_BUTTON)
    {
        if (!fixedPressed) { motorPriorityOwner = MOTOR_OWNER_NONE; driveMotor(0); return; }
        driveMotor(127);
    }
    else
    {
        if (!joyActive) { motorPriorityOwner = MOTOR_OWNER_NONE; driveMotor(0); return; }
        uint8_t speed = map(abs(joyRaw - 512), 0, 512, 0, 255);
        driveMotor(speed);
    }

    if (motorRunning)
        updateRuntimeRegisters();
}


//  Setup & Loop 

void setup()
{
    Serial.begin(9600);
    Serial.println("Geometry slave starting");

    Wire.begin(GEO_I2C_ADDRESS);
    Wire.onReceive(onReceive);
    Wire.onRequest(onRequest);

    for (uint8_t i = 0; i < GEO_REG_COUNT; i++) registers[i] = 0x00;
    registers[1] = 0x80;

    pinMode(SAN_XRAY_ENABLED_PIN, INPUT);
    pinMode(SAN_GEO_MOVING_PIN,   OUTPUT);
    digitalWrite(SAN_GEO_MOVING_PIN, LOW);

    pinMode(MOTOR_PWM_PIN,      OUTPUT);
    pinMode(FIXED_SPEED_BUTTON, INPUT_PULLUP);

    randomSeed(analogRead(A1));
}
    
void loop()
{
    handlePreparingState();
    handleUnprepareState();
    handleMotorControl();
    handleAcquiringState();
}