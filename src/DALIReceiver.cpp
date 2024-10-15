#include "DALIReceiver.h"

DALIReceiver::DALIReceiver(int rx_pin, int rmt_channel)
    : _rx_pin(rx_pin), _rmt_channel((rmt_channel_t)rmt_channel) {
}

DALIReceiver::~DALIReceiver() {
    rmt_driver_uninstall(_rmt_channel);
}

void DALIReceiver::begin() {
    _rmt_rx_config.rmt_mode = RMT_MODE_RX;
    _rmt_rx_config.channel = _rmt_channel;
    _rmt_rx_config.gpio_num = (gpio_num_t)_rx_pin;
    _rmt_rx_config.clk_div = 80; // Clock divider (1 µs per tick with 80 MHz clock)
    _rmt_rx_config.mem_block_num = 1;
    _rmt_rx_config.rx_config.filter_en = true;
    _rmt_rx_config.rx_config.filter_ticks_thresh = 100; // filter out noise
    _rmt_rx_config.rx_config.idle_threshold = DALI_RECEIVER_BIT_TIME * 10;

    rmt_config(&_rmt_rx_config);
    rmt_driver_install(_rmt_channel, 1000, 0);
}

bool DALIReceiver::receiveCommand(uint16_t &command) {
    size_t rx_size = 0;
    rmt_item32_t *items = NULL;

    esp_err_t err = rmt_get_ringbuf_handle(_rmt_channel, (RingbufHandle_t *)&items);
    if (err != ESP_OK) {
        return false;
    }

    items = (rmt_item32_t *)xRingbufferReceive((RingbufHandle_t)items, &rx_size, 1000);
    if (items) {
        bool result = decodeRMTData(items, command);
        vRingbufferReturnItem((RingbufHandle_t)items, items);
        return result;
    }
    return false;
}

bool DALIReceiver::decodeRMTData(rmt_item32_t *items, uint16_t &command) {
    command = 0;
    if (items == nullptr) {
        return false;
    }

    for (int i = 0; i < DALI_RECEIVER_BITS; ++i) {
        if (items[i].duration0 < DALI_RECEIVER_BIT_TIME) {
            command <<= 1;
            command |= 1; // 1 bit
        } else {
            command <<= 1;
            command |= 0; // 0 bit
        }
    }
    return true;
}
