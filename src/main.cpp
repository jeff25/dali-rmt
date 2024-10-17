#include "Arduino.h"
#include "driver/rmt.h"

// Define DALI-specific timing
#define DALI_BIT_PERIOD_US    833
#define DALI_HALF_BIT_US      (DALI_BIT_PERIOD_US / 2)

// Define the GPIO pin used for DALI communication
#define DALI_TX_GPIO 7   // Arduino-compatible GPIO pin definition

// RMT configuration
rmt_config_t rmt_tx;

void configure_rmt() {
    // Initialize RMT transmitter
    rmt_tx.channel = RMT_CHANNEL_0;
    rmt_tx.gpio_num = (gpio_num_t)DALI_TX_GPIO; // Cast pin number to gpio_num_t type
    rmt_tx.clk_div = 80;                       // 1 MHz clock (80 MHz / 80)
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

// Function to send a single DALI bit (0 or 1)
void send_dali_bit(bool bit) {
    rmt_item32_t item;

    if (bit) {
        item.level0 = 0; // Start with LOW
        item.duration0 = DALI_HALF_BIT_US;  // Half-bit period for LOW
        item.level1 = 1; // Then HIGH
        item.duration1 = DALI_HALF_BIT_US;  // Half-bit period for HIGH
    } else {
        item.level0 = 1; // Start with HIGH
        item.duration0 = DALI_HALF_BIT_US;  // Half-bit period for HIGH
        item.level1 = 0; // Then LOW
        item.duration1 = DALI_HALF_BIT_US;  // Half-bit period for LOW
    }

     // Print the bit being sent
    //Serial.print("Sending bit: ");
    //Serial.println(bit);

    // Send the bit over RMT
    rmt_write_items(rmt_tx.channel, &item, 1, true);
}

// Function to send a complete DALI frame
void send_dali_frame(uint16_t data, int num_bits) {
    if (num_bits != 8 && num_bits != 16) {
        Serial.println("Error: num_bits must be 8 or 16");
        return;
    }

    send_dali_bit(0); // Start bit

    // Send data bits, least significant bit first
    for (int i = 0; i < num_bits; i++) {
        bool bit = (data >> i) & 0x01;
        Serial.print(bit ? '1' : '0'); // Print the bit
        send_dali_bit(bit);
    }

    send_dali_bit(1); // Stop bits
    send_dali_bit(1);
}

void setup() {
    // Initialize serial communication for printing
    Serial.begin(115200);
      while (!Serial) {
        // Wait for serial to be ready
    }

    // Initialize the RMT for DALI communication
    configure_rmt();
    // Example: Send an 8-bit DALI command (e.g., 0xA6)
    send_dali_frame(0xA6, 8);

    // Example: Send a 16-bit DALI command (e.g., 0xA6F0)
    send_dali_frame(0xA6F0, 16);
}

void loop() {
   
}
