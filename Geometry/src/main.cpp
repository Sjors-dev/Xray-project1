#include <Arduino.h>
#include <Wire.h>

#define SLAVE_ADDR 0x10

uint8_t reg[256];
uint8_t currentReg = 0;

void receiveEvent(int howMany) {
    if (howMany == 0) return;

    currentReg = Wire.read();   // register address

    if (Wire.available()) {
        uint8_t value = Wire.read();
        reg[currentReg] = value;

        if (currentReg == 0x08 && value == 0x10) {
            reg[0x09] = 0x80;   // PREPARED
        }
    }
}

void requestEvent() {
    Wire.write(reg[currentReg]); // return last selected register
}

void setup() {
    Wire.begin(SLAVE_ADDR);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);

    reg[0x09] = 0x00;
}

void loop() {}