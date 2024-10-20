#include "Arduino.h"
#include "driver/rmt.h"

// Define DALI-specific timing
#define DALI_BIT_PERIOD_US    833
#define DALI_HALF_BIT_US      (DALI_BIT_PERIOD_US / 2)
#define DALI_STOP_CONDITION_US 2450   // Minimum stop condition duration (2.45 ms)

// Define the GPIO pin used for DALI communication
#define DALI_TX_GPIO 7   // Arduino-compatible GPIO pin definition

// RMT configuration
rmt_config_t rmt_tx;

void configure_rmt() {
    // Initialize RMT transmitter
    rmt_tx.channel = RMT_CHANNEL_0;
    rmt_tx.gpio_num = (gpio_num_t)DALI_TX_GPIO; // Cast pin number to gpio_num_t type
    rmt_tx.clk_div = 80;                        // 80 MHz / 80 = 1 MHz
    rmt_tx.mem_block_num = 1;                  // Only need one memory block
    rmt_tx.tx_config.loop_en = false;
    rmt_tx.tx_config.carrier_en = false;       // No carrier modulation for DALI
    rmt_tx.tx_config.idle_level = RMT_IDLE_LEVEL_LOW; // Idle level is LOW
    rmt_tx.tx_config.idle_output_en = true;    // Enable idle output
    rmt_tx.rmt_mode = RMT_MODE_TX;             // Set the mode to TX
    // Apply the configuration and install RMT driver
    rmt_config(&rmt_tx);
    rmt_driver_install(rmt_tx.channel, 0, 0);
}

// Function to prepare a DALI frame with inverted bits
void prepare_dali_frame(uint16_t data, int num_bits, rmt_item32_t* items) {
    int index = 0;
    // Start bit (LOW)
    items[index].level0 = 1; // LOW
    items[index].duration0 = DALI_HALF_BIT_US;
    items[index].level1 = 0; // HIGH
    items[index].duration1 = DALI_HALF_BIT_US;
    index++;
    // Data bits, most significant bit first, inverting the bits
    for (int i = num_bits - 1; i >= 0; i--) {
        bool bit = (data >> i) & 0x01; // Get the bit from the MSB to LSB
        if (bit) {
            // If bit is 1, send HIGH for half bit period, then LOW
            items[index].level0 = 1;  // HIGH
            items[index].duration0 = DALI_HALF_BIT_US;
            items[index].level1 = 0;  // LOW
            items[index].duration1 = DALI_HALF_BIT_US;
        } else {
            // If bit is 0, send LOW for half bit period, then HIGH
            items[index].level0 = 0;  // LOW
            items[index].duration0 = DALI_HALF_BIT_US;
            items[index].level1 = 1;  // HIGH
            items[index].duration1 = DALI_HALF_BIT_US;
        }
        index++;
    }
    // Stop bits (HIGH)
    items[index].level0 = 0; // HIGH
    items[index].duration0 = DALI_STOP_CONDITION_US;
    items[index].level1 = 0; // HIGH
    items[index].duration1 = DALI_STOP_CONDITION_US;
    index++;
}

// Function to send a complete DALI frame
void send_dali_frame(uint16_t data, int num_bits) {
    if (num_bits != 8 && num_bits != 16) {
        Serial.println("Error: num_bits must be 8 or 16");
        return;
    }
    rmt_item32_t items[20]; // Maximum of 20 items (1 start bit + 16 data bits + 2 stop bits)
    prepare_dali_frame(data, num_bits, items);
    // Send the frame over RMT
    rmt_write_items(rmt_tx.channel, items, num_bits + 2, true); // 1 start bit + num_bits data bits + 1 stop bit
    rmt_wait_tx_done(rmt_tx.channel, pdMS_TO_TICKS(10)); // Ensure the frame is transmitted
    // Set the bus to idle
    gpio_set_level((gpio_num_t)DALI_TX_GPIO, RMT_IDLE_LEVEL_LOW);
}

void setup() {
    // Initialize serial communication for printing
    Serial.begin(115200);
    while (!Serial) {
        // Wait for serial to be ready
    }
    // Initialize the RMT for DALI communication
    configure_rmt();

    // Example: Send an 8-bit DALI command (e.g., 0xFF)
    send_dali_frame(0xFF, 8);
    // Example: Send a 16-bit DALI command (e.g., 0x80FE)
    send_dali_frame(0x80FE, 16);
}

void loop() {
    send_dali_frame(0x01, 8);  // Test frame (0x01)
    delay(1000);
    //send_dali_frame(0x80FE, 16);
    //delay(1000);
}
