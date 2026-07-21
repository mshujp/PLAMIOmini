#pragma once

#include "GraphicsBase.h"
#include <LovyanGFX.hpp>

namespace PLAMIOmini {

class LGFX_ILI9341 : public lgfx::LGFX_Device
{
    lgfx::Panel_ILI9341 panel;
    lgfx::Bus_SPI bus;

public:
    // SPI
    LGFX_ILI9341(uint8_t spiHost, uint32_t spiWriteFreq, int8_t clkPin, int8_t dataPin, int8_t dcPin, int8_t csPin, int8_t rstPin)
    {
        {
            auto cfg = bus.config();
            cfg.spi_host = static_cast<decltype(cfg.spi_host)>(spiHost);
            cfg.spi_mode = 0;
            cfg.freq_write = spiWriteFreq;
            cfg.pin_sclk = clkPin;
            cfg.pin_mosi = dataPin;
            cfg.pin_dc = dcPin;
            bus.config(cfg);
            panel.setBus(&bus);
        }
        {
            auto cfg = panel.config();
            cfg.pin_cs = csPin;
            cfg.pin_rst = rstPin;
            cfg.panel_width = Display::ILI9341_SCREEN_H;
            cfg.panel_height = Display::ILI9341_SCREEN_W;
            cfg.invert = false; 
            panel.config(cfg);
        }
        setPanel(&panel);
    }
};

class GraphicsILI9341 : public GraphicsBase {
private:
    LGFX_ILI9341 lcd;
    uint8_t lcdRotate = 0;
    int8_t backLightPin = -1;

    LGFX_Sprite canvas;
    void setFont(const char* str, Font font);
  
public:
    struct Config
    { 
        uint8_t spiHost = 0;
        uint32_t spiWriteFreq = 60000000;
        int8_t clkPin   = -1;
        int8_t dataPin  = -1;
        int8_t dcPin    = -1;
        int8_t csPin    = -1;
        int8_t resetPin   = -1;
        int8_t backlightPin = -1;
        uint8_t lcdRotate = 0;
    };

    explicit GraphicsILI9341(const Config& config);
 
    bool begin() override;
    void end() override;
 
    void clearScreen() override;
    void fillScreen(Graphics::Color color) override;
    void drawPixel(int16_t x, int16_t y, Graphics::Color color) override;
    void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, Graphics::Color color) override;
    void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, Graphics::Color color) override;
    void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, Graphics::Color color) override;
    void drawRect(int16_t x, int16_t y, uint16_t w, uint16_t h, Graphics::Color color) override;
    void drawRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t radius, Graphics::Color color) override;
    void fillRect(int16_t x, int16_t y, uint16_t w, uint16_t h, Graphics::Color color) override;
    void fillRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, int16_t r, Graphics::Color color) override;
    void drawCircle(int16_t x, int16_t y, uint16_t r, Graphics::Color color) override;
    void drawCircle(int16_t x, int16_t y, uint16_t rx, uint16_t ry, Graphics::Color color) override;
    void fillCircle(int16_t x, int16_t y, uint16_t r, Graphics::Color color) override;
    void fillCircle(int16_t x, int16_t y, uint16_t rx, uint16_t ry, Graphics::Color color) override;
    void drawString(const char* str, int16_t x, int16_t y, Graphics::Color color, Font font) override;
    uint16_t getTextWidth(const char* text, Font font) override;
    void drawSprite(const uint16_t* bitmap, int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t spriteScale,  Color transparentColor, bool flipX = false, bool flipY = false) override;
    void setViewport(int16_t viewportX, int16_t viewportY) override;
    bool readScreenLine(uint16_t y, uint16_t* outPixels, uint16_t pixelCount) override;

    void push() override;
};

} // namespace
