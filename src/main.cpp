#include <Arduino.h>
#include "DALIReceiver.h"

#define DALI_RX_PIN 20
#define RMT_CHANNEL RMT_CHANNEL_1

DALIReceiver daliReceiver(DALI_RX_PIN, RMT_CHANNEL);

void setup() {
    Serial.begin(115200);
    daliReceiver.begin();
}

void loop() {
    uint16_t command;
    
    // Get the current time before trying to receive a command
    unsigned long startTime = millis();
    
    if (daliReceiver.receiveCommand(command)) {
        unsigned long duration = millis() - startTime;  // Calculate the time taken to receive the command
        Serial.printf("Received DALI Command: 0x%04X in %lu ms\n", command, duration);
    } else {
        unsigned long duration = millis() - startTime;  // Time taken even if no command is received
        //Serial.printf("No valid DALI command received (Duration: %lu ms)\n", duration);
         Serial.printf("Received DALI Command: 0x%04X in %lu ms\n", command, duration);
    }
    
    delay(1000); // Delay for demonstration
}
