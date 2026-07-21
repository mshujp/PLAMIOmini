#pragma once

#include "AudioBase.h"

namespace PLAMIOmini {

class AudioPWM : public AudioBase
{
private:
    static constexpr uint16_t SAMPLE_RATE = 22050;

    int8_t pin = -1;
    bool started = false;

    uint32_t sampleRate() const override { return SAMPLE_RATE; }
    bool toneSamples(int from, int to, uint32_t total, uint32_t& written, float startGain, float endGain) override;

public:
    struct Config
    {
        int8_t pwmPin = -1;
    };

    explicit AudioPWM(const Config& config)
        : pin(config.pwmPin) {}

    uint8_t getVolumeSteps() const override { return 2; }
    bool begin() override;
    void end() override;
};

} // namespace PLAMIOmini
