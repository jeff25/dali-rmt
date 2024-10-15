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
    if (daliReceiver.receiveCommand(command)) {
        Serial.printf("Received DALI Command: 0x%04X\n", command);
    } else {
        Serial.println("No valid DALI command received");
    }
    delay(1000); // Delay for demonstration
}
