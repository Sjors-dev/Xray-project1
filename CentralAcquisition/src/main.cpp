#include <Arduino.h>
#include <Wire.h>
#include <avr/io.h>
//#include "../../Interface_I2C/Protocol_I2C.h"
#include "../../Interface_PatAdmin_CentralAcq/Protocol_PatientAdmin_CentralAcq.h"

// I2C
#define GEO_ADDR   0x10
#define XRAY_ADDR  0x20

// LEDs
#define PIN_PREPARED_LAMP  7
#define PIN_ACQUIRING_LAMP 8
#define RED_LED            9   // exam type set
#define GREEN_LED          10  // NO_EXAM selected

// Buttons
#define PIN_PREPARE_BTN    2
#define PIN_XRAY_BTN       3

// SAN bus (digitale draden)
#define SAN_XRAY_ENABLED   4   // OUTPUT - hoog als xray actief


typedef enum {
    STATE_NOT_CONNECTED,
    STATE_IDLE,
    STATE_PREPARING,
    STATE_PREPARED,
    STATE_ACQUIRING
} CentralAcqState;

//globale variablen
CentralAcqState state = STATE_NOT_CONNECTED;
bool prepBtnPressed = false;
bool xrayBtnPressed = false;
unsigned long preparingStartTime = 0;




void setup() {

    Wire.begin();  // master heeft geen adres
    Wire.setClock(100000); // 100 kHz
    Serial.begin(9600);
    
    Serial.println("Hoi!!");
    DDRD |= (1 << PD7);   // Zet pin 7 as OUTPUT
}

// Schrijf een waarde naar een register op een slave
void writeRegister(uint8_t deviceAddr, uint8_t reg, uint8_t value) {
    Wire.beginTransmission(deviceAddr);
    Wire.write(reg);    // selecteer register
    Wire.write(value);  // schrijf waarde
    Wire.endTransmission();
}

// Lees een register van een slave
uint8_t readRegister(uint8_t deviceAddr, uint8_t reg) {
    Wire.beginTransmission(deviceAddr);
    Wire.write(reg);  // selecteer register
    Wire.endTransmission(false);  // geen stop, repeated start
    
    Wire.requestFrom(deviceAddr, (uint8_t)1);
    if (Wire.available()) {
        return Wire.read();
    }
    return 0xFF;  // fout
}

// Stuur prepare command naar Geo en Xray
void sendPrepare() {
    writeRegister(GEO_ADDR,  0x08, 0x10);  // $8, cmd = 01 in bits 5:4
    writeRegister(XRAY_ADDR, 0x08, 0x10); 
}

void loop() {
    
    // Lees state van Geometry ($9)
    uint8_t geoState = readRegister(GEO_ADDR, 0x09);
    Serial.println(geoState);
    
    // Lees state van XrayGenerator ($9)
    uint8_t xrayState = readRegister(XRAY_ADDR, 0x09);
    Serial.println(xrayState);

    
    // Check of beide prepared zijn (bits 7:6 == 10)
    bool geoPrepared  = ((geoState  >> 6) & 0x03) == 0x02;
    bool xrayPrepared = ((xrayState >> 6) & 0x03) == 0x02;
    
    if (!geoPrepared || !xrayPrepared) {
        sendPrepare();
        PORTD &= ~(1 << 7); // zet pin 7 LOW (LED uit)
    }
    else{
        PORTD |= (1 << 7);  // zet pin 7 HIGH (LED aan)
    }
    
    delay(500);  // alleen in master loop mag delay
}