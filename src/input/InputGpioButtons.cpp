#include "InputGpioButtons.h"

#include <Arduino.h>

using namespace PLAMIOmini;

namespace {

struct MappingEntry
{
    Input::Button button;
    int16_t InputBase::ButtonMapping::* pin;
};

constexpr MappingEntry MAPPINGS[] =
{
    {Input::UP, &InputBase::ButtonMapping::UP},
    {Input::DOWN, &InputBase::ButtonMapping::DOWN},
    {Input::LEFT, &InputBase::ButtonMapping::LEFT},
    {Input::RIGHT, &InputBase::ButtonMapping::RIGHT},
    {Input::A, &InputBase::ButtonMapping::A},
    {Input::B, &InputBase::ButtonMapping::B},
    {Input::X, &InputBase::ButtonMapping::X},
    {Input::Y, &InputBase::ButtonMapping::Y},
    {Input::L, &InputBase::ButtonMapping::L},
    {Input::R, &InputBase::ButtonMapping::R},
    {Input::START, &InputBase::ButtonMapping::START},
    {Input::SELECT, &InputBase::ButtonMapping::SELECT},
    {Input::VOL_UP, &InputBase::ButtonMapping::VOL_UP},
    {Input::VOL_DOWN, &InputBase::ButtonMapping::VOL_DOWN},
    {Input::HOME, &InputBase::ButtonMapping::HOME},
    {Input::MUTE, &InputBase::ButtonMapping::MUTE}
};

} // namespace

InputGpioButtons::InputGpioButtons(const ButtonMapping& _buttonMapping)
    : buttonMapping(_buttonMapping)
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
