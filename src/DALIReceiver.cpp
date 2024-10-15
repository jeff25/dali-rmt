#include "Arduino.h"
#include "DALIReceiver.h"

// Constructor
DALIReceiver::DALIReceiver(int rx_pin, int rmt_channel)
    : _rx_pin(rx_pin), _rmt_channel((rmt_channel_t)rmt_channel) {
}

// Destructor
DALIReceiver::~DALIReceiver() {
    rmt_driver_uninstall(_rmt_channel);
}

// Initialize the RMT receiver
void DALIReceiver::begin() {
    unsigned long startTime = millis();
    
    _rmt_rx_config.rmt_mode = RMT_MODE_RX;
    _rmt_rx_config.channel = _rmt_channel;
    _rmt_rx_config.gpio_num = (gpio_num_t)_rx_pin;
    _rmt_rx_config.clk_div = 80; // Clock divider (1 µs per tick with 80 MHz clock)
    _rmt_rx_config.mem_block_num = 1;
    _rmt_rx_config.rx_config.filter_en = true;
    _rmt_rx_config.rx_config.filter_ticks_thresh = 100; // filter out noise
    _rmt_rx_config.rx_config.idle_threshold = DALI_RECEIVER_BIT_TIME * 10;

    // Configure RMT
    esp_err_t err = rmt_config(&_rmt_rx_config);
    unsigned long configTime = millis() - startTime;
    Serial.printf("RMT Config Time: %lu ms\n", configTime);
    
    if (err != ESP_OK) {
        Serial.println("Failed to configure RMT");
        return;
    }

    // Install RMT driver
    startTime = millis();
    err = rmt_driver_install(_rmt_channel, 1000, 0);
    unsigned long installTime = millis() - startTime;
    Serial.printf("RMT Driver Install Time: %lu ms\n", installTime);

    if (err != ESP_OK) {
        Serial.println("Failed to install RMT driver");
        return;
    }

    // Start receiving on the RMT channel
    startTime = millis();
    err = rmt_rx_start(_rmt_channel, 1);
    unsigned long startTimeDuration = millis() - startTime;
    Serial.printf("RMT Start RX Time: %lu ms\n", startTimeDuration);

    if (err != ESP_OK) {
        Serial.println("Failed to start RMT reception");
    }
}

// Receive DALI command
bool DALIReceiver::receiveCommand(uint16_t &command) {
    size_t rx_size = 0;
    RingbufHandle_t rb = NULL;

    // Get the RMT ring buffer handle
    esp_err_t err = rmt_get_ringbuf_handle(_rmt_channel, &rb);
    if (err != ESP_OK || rb == NULL) {
        Serial.println("Failed to get RMT ring buffer handle");
        return false;
    }

    unsigned long startTime = millis();
    
    // Receive items from the ring buffer
    rmt_item32_t *items = (rmt_item32_t *)xRingbufferReceive(rb, &rx_size, 1000);
    unsigned long receiveTime = millis() - startTime;

    if (items) {
        Serial.printf("RMT Item Receive Time: %lu ms\n", receiveTime);

        // Check if the received size is sufficient for decoding
        if (rx_size >= sizeof(rmt_item32_t) * DALI_RECEIVER_BITS) {
            bool result = decodeRMTData(items, command);
            vRingbufferReturnItem(rb, (void *)items); // Return the item back to the buffer
            return result;
        } else {
            Serial.println("Received insufficient data for DALI command");
        }
        vRingbufferReturnItem(rb, (void *)items); // Return the item back to the buffer
    } else {
        Serial.println("No items received");
    }
    return false;
}

// Decode RMT data into a command
bool DALIReceiver::decodeRMTData(rmt_item32_t *items, uint16_t &command) {
    command = 0;
    if (items == nullptr) {
        return false;
    }

    unsigned long startTime = millis();
    
    // Decode the received items
    for (int i = 0; i < DALI_RECEIVER_BITS; ++i) {
        Serial.printf("Bit %d: Duration0: %d, Expected Bit Time: %d\n", i, items[i].duration0, DALI_RECEIVER_BIT_TIME);
        
        if (items[i].duration0 < DALI_RECEIVER_BIT_TIME) {
            command <<= 1;
            command |= 1; // 1 bit
        } else {
            command <<= 1;
            command |= 0; // 0 bit
        }
    }
    unsigned long decodeTime = millis() - startTime;
    Serial.printf("RMT Decode Time: %lu ms\n", decodeTime);
    
    return true;
}
