#pragma once

#include "InputBase.h"

namespace PLAMIOmini {

class InputSnes : public InputBase
{
private:
    int8_t clkPin;
    int8_t latPin;
    int8_t dataPin;
    ButtonMapping buttonMapping;

    uint32_t readButtons() override;

public:
    struct Config {
        int8_t clkPin  = -1;
        int8_t latPin  = -1;
        int8_t dataPin = -1;
        ButtonMapping buttonMapping{};
    };

    explicit InputSnes(const Config& config);

    bool begin() override;
    void end() override;
};

} // namespace PLAMIOmini
