#include <Arduino.h>
#include <Wire.h>
#define SLAVE_ADDR 0x20

uint8_t reg[256];
uint8_t currentReg = 0;  

void receiveEvent(int howMany) {
    if (howMany == 0) return;
    currentReg = Wire.read();   
    if (Wire.available()) {
        uint8_t v = Wire.read();
        reg[currentReg] = v;
        if (currentReg == 0x08 && v == 0x10) {
            reg[0x09] = 0x80;
        }
    }
}

void requestEvent() {
    Wire.write(reg[currentReg]);  
}

void setup() {
    Wire.begin(SLAVE_ADDR);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);
    reg[0x09] = 0x00;
}

void loop() {}