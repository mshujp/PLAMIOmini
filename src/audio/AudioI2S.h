#pragma once

#include "AudioBase.h"

namespace PLAMIOmini {

class AudioI2S : public AudioBase
{
private:
    static constexpr uint16_t SAMPLE_RATE = 22050;

    int8_t bclkPin = -1;
    int8_t dataPin = -1;
    void* device = nullptr;
    float phase = 0;
    bool started = false;

    uint32_t sampleRate() const override { return SAMPLE_RATE; }
    bool toneSamples(int from, int to, uint32_t total, uint32_t& written, float startGain, float endGain) override;

public:
    struct Config
    {
        int8_t bclkPin = -1;
        int8_t dataPin = -1;
    };

    explicit AudioI2S(const Config& config) : bclkPin(config.bclkPin), dataPin(config.dataPin) {}
    ~AudioI2S() override;

    uint8_t getVolumeSteps() const override { return 4; }
    bool begin() override;
    void end() override;
};

} // namespace PLAMIOmini
