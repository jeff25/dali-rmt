#ifndef DALIReceiver_h
#define DALIReceiver_h

#include <driver/rmt.h>

class DALIReceiver {
public:
    DALIReceiver(int rx_pin, int rmt_channel);
    ~DALIReceiver();
    void begin();
    bool receiveCommand(uint16_t &command);

private:
    int _rx_pin;
    rmt_channel_t _rmt_channel;
    rmt_config_t _rmt_rx_config;
    
    static const int DALI_RECEIVER_BITS = 16; // Number of bits in a DALI command
    static const int DALI_RECEIVER_BIT_TIME = 416; // DALI bit duration in microseconds

    bool decodeRMTData(rmt_item32_t *items, uint16_t &command);
};

#endif
