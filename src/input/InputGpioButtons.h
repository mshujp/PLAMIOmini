#pragma once

#include "InputBase.h"

namespace PLAMIOmini {

// Input implementation for buttons individually connected to GPIO pins.
class InputGpioButtons : public InputBase
{
private:
    ButtonMapping buttonMapping;

    uint32_t readButtons() override;

public:
    explicit InputGpioButtons(const ButtonMapping& buttonMapping);

    bool begin() override;
    void end() override;
};

} // namespace PLAMIOmini
