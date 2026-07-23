#include "InputGpioButtons.h"

#include <Arduino.h>

using namespace PLAMIOmini;

namespace {

struct MappingEntry
{
    Input::Button button;
    int16_t ButtonMapping::* pin;
};

constexpr MappingEntry MAPPINGS[] =
{
    {Input::UP, &ButtonMapping::UP},
    {Input::DOWN, &ButtonMapping::DOWN},
    {Input::LEFT, &ButtonMapping::LEFT},
    {Input::RIGHT, &ButtonMapping::RIGHT},
    {Input::A, &ButtonMapping::A},
    {Input::B, &ButtonMapping::B},
    {Input::X, &ButtonMapping::X},
    {Input::Y, &ButtonMapping::Y},
    {Input::L, &ButtonMapping::L},
    {Input::R, &ButtonMapping::R},
    {Input::START, &ButtonMapping::START},
    {Input::SELECT, &ButtonMapping::SELECT},
    {Input::VOL_UP, &ButtonMapping::VOL_UP},
    {Input::VOL_DOWN, &ButtonMapping::VOL_DOWN},
    {Input::HOME, &ButtonMapping::HOME},
    {Input::MUTE, &ButtonMapping::MUTE}
};

} // namespace

InputGpioButtons::InputGpioButtons(const InputGpioButtonsConfig& config)
    : buttonMapping(config.buttonMapping)
{
}

bool InputGpioButtons::begin()
{
    for (const auto& entry : MAPPINGS)
    {
        const int16_t pin = buttonMapping.*(entry.pin);
        if (pin >= 0) pinMode(pin, INPUT_PULLUP);
    }

    reset();
    available = true;
    return true;
}

void InputGpioButtons::end()
{
    available = false;
    reset();
}

uint32_t InputGpioButtons::readButtons()
{
    uint32_t result = 0;
    for (const auto& entry : MAPPINGS)
    {
        const int16_t pin = buttonMapping.*(entry.pin);
        if (pin >= 0 && digitalRead(pin) == LOW) result |= entry.button;
    }
    return result;
}
