#pragma once

#include "InputBase.h"
#include <Arduino.h>

namespace PLAMIOmini {
namespace GpioButtons {

inline void init(const ButtonMapping& mapping)
{
    const int16_t pins[] =
    {
        mapping.UP, mapping.DOWN, mapping.LEFT, mapping.RIGHT,
        mapping.A, mapping.B, mapping.X, mapping.Y,
        mapping.L, mapping.R, mapping.START, mapping.SELECT,
        mapping.VOL_UP, mapping.VOL_DOWN, mapping.MUTE
    };

    for (int16_t pin : pins)
    {
        if (pin >= 0) pinMode(pin, INPUT_PULLUP);
    }
}

inline uint32_t read(const ButtonMapping& mapping)
{
    struct Entry
    {
        Input::Button button;
        int16_t pin;
    };

    const Entry entries[] =
    {
        {Input::UP, mapping.UP}, {Input::DOWN, mapping.DOWN},
        {Input::LEFT, mapping.LEFT}, {Input::RIGHT, mapping.RIGHT},
        {Input::A, mapping.A}, {Input::B, mapping.B},
        {Input::X, mapping.X}, {Input::Y, mapping.Y},
        {Input::L, mapping.L}, {Input::R, mapping.R},
        {Input::START, mapping.START}, {Input::SELECT, mapping.SELECT},
        {Input::VOL_UP, mapping.VOL_UP}, {Input::VOL_DOWN, mapping.VOL_DOWN},
        {Input::MUTE, mapping.MUTE}
    };

    uint32_t value = 0;
    for (const auto& entry : entries)
    {
        if (entry.pin >= 0 && digitalRead(entry.pin) == LOW) value |= entry.button;
    }
    return value;
}

} // namespace GpioButtons
} // namespace PLAMIOmini
