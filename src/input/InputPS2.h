#pragma once

#include "InputBase.h"

namespace PLAMIOmini {

// Experimental PlayStation 2 controller input implementation.
// Compile-tested only; communication with real hardware has not been verified.
// Supports digital buttons only. ACK, analog axes, vibration, and mode setup
// are intentionally not implemented.
class InputPS2 : public InputBase
{
private:
    int8_t clockPin;
    int8_t commandPin;
    int8_t attentionPin;
    int8_t dataPin;
    ButtonMapping buttonMapping;

    uint8_t transferByte(uint8_t command);
    bool pollController(uint32_t& buttons);
    uint32_t readButtons() override;

public:
    struct Config
    {
        int8_t clockPin = -1;
        int8_t commandPin = -1;
        int8_t attentionPin = -1;
        int8_t dataPin = -1;
        ButtonMapping buttonMapping{};
    };

    explicit InputPS2(const Config& config);

    bool begin() override;
    void end() override;
};

} // namespace PLAMIOmini
