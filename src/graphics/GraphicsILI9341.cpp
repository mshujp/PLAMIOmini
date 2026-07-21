#include "GraphicsILI9341.h"
#include <Arduino.h>

using namespace PLAMIOmini;

GraphicsILI9341::GraphicsILI9341(const Config& config)
    : lcd(config.spiHost, config.spiWriteFreq, config.clkPin, config.dataPin, config.dcPin, config.csPin, config.resetPin),
        lcdRotate(config.lcdRotate), backLightPin(config.backlightPin),
        canvas(&lcd) 
{
}

bool GraphicsILI9341::begin()
{
    lcd.init();
    lcd.setRotation(lcdRotate);

    canvas.setColorDepth(lcd.getColorDepth());
    canvas.setSwapBytes(true);
    canvas.createSprite(Display::ILI9341_SCREEN_W, Display::ILI9341_SCREEN_H);
    canvas.clear();

    if (backLightPin >= 0)
    {
        pinMode(backLightPin, OUTPUT);
        digitalWrite(backLightPin, HIGH);
    }

    viewportX = 0;
    viewportY = 0;
    screenDirty = true;
    return canvas.getBuffer() != nullptr;
}

void GraphicsILI9341::end()
{
    canvas.deleteSprite();
    lcd.clear();
    if (backLightPin >= 0) digitalWrite(backLightPin, LOW);
    screenDirty = false;
}

void GraphicsILI9341::clearScreen()
{
    canvas.clear();
    screenDirty = true;
}

void GraphicsILI9341::fillScreen(Graphics::Color color)
{
    canvas.fillScreen(color);
    screenDirty = true;
}

void GraphicsILI9341::drawPixel(int16_t x, int16_t y, Graphics::Color color)
{
    canvas.drawPixel(x, y, color);
    screenDirty = true;
}

void GraphicsILI9341::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, Graphics::Color color)
{
    canvas.drawLine(x1, y1, x2, y2, color);
    screenDirty = true;
}

void GraphicsILI9341::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, Graphics::Color color)
{
    canvas.drawTriangle(x0, y0, x1, y1, x2, y2, color);
    screenDirty = true;
}

void GraphicsILI9341::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, Graphics::Color color)
{
    canvas.fillTriangle(x0, y0, x1, y1, x2, y2, color);
    screenDirty = true;
}

void GraphicsILI9341::drawRect(int16_t x, int16_t y, uint16_t w, uint16_t h, Graphics::Color color)
{
    canvas.drawRect(x, y, w, h, color);
    screenDirty = true;
}

void GraphicsILI9341::drawRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t radius, Graphics::Color color)
{
    canvas.drawRoundRect(x, y, w, h, radius, color);
    screenDirty = true;
}

void GraphicsILI9341::fillRect(int16_t x, int16_t y, uint16_t w, uint16_t h, Graphics::Color color)
{
    canvas.fillRect(x, y, w, h, color);
    screenDirty = true;
}

void GraphicsILI9341::fillRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, int16_t r, Graphics::Color color)
{
    canvas.fillRoundRect(x, y, w, h, r, color);
    screenDirty = true;
}

void GraphicsILI9341::drawCircle(int16_t x, int16_t y, uint16_t r, Graphics::Color color)
{
    canvas.drawCircle(x, y, r, color);
    screenDirty = true;
}

void GraphicsILI9341::drawCircle(int16_t x, int16_t y, uint16_t rx, uint16_t ry, Graphics::Color color)
{
    canvas.drawEllipse(x, y, rx, ry, color);
    screenDirty = true;
}

void GraphicsILI9341::fillCircle(int16_t x, int16_t y, uint16_t r, Graphics::Color color)
{
    canvas.fillCircle(x, y, r, color);
    screenDirty = true;
}

void GraphicsILI9341::fillCircle(int16_t x, int16_t y, uint16_t rx, uint16_t ry, Graphics::Color color)
{
    canvas.fillEllipse(x, y, rx, ry, color);
    screenDirty = true;
}

void GraphicsILI9341::setFont(const char* str, Font font)
{
    const lgfx::IFont* targetFont = &fonts::DejaVu12;
    float scaleS = 1.0;

    switch (font)
    {
        case Font::SIZE_10:  targetFont = &fonts::DejaVu9; break;
        case Font::SIZE_13:  targetFont = &fonts::DejaVu12; break;
        case Font::SIZE_18:  targetFont = &fonts::DejaVu18; break;
        case Font::SIZE_22B: targetFont = &fonts::FreeSansBold9pt7b; break;
        case Font::SIZE_25:  targetFont = &fonts::DejaVu24; break;
        case Font::SIZE_25B: targetFont = &fonts::FreeSansBold9pt7b; scaleS = 1.13; break;
        case Font::SIZE_32:  targetFont = &fonts::DejaVu24; scaleS = 1.33; break;
        case Font::SIZE_32B: targetFont = &fonts::FreeSansBold12pt7b; scaleS = 1.10; break;
        case Font::SIZE_42:  targetFont = &fonts::DejaVu40; break;
        case Font::SIZE_42B: targetFont = &fonts::FreeSansBold18pt7b; break;
#ifdef PLAMIO_JAPANESE_FONT
        case Font::SIZE_16J: targetFont = &fonts::efontJA_16; break;
        case Font::SIZE_20J: targetFont = &fonts::efontJA_16; scaleS = 1.25; break;
        case Font::SIZE_32J: targetFont = &fonts::efontJA_16; scaleS = 2; break;
#endif
        default: targetFont = &fonts::DejaVu9; break;
    }
 
    canvas.setFont(targetFont);
    canvas.setTextSize(scaleS);
}

void GraphicsILI9341::drawString(const char* str, int16_t x, int16_t y, Graphics::Color color, Font font)
{
    if (str == nullptr) return;
    setFont(str, font);
    canvas.setTextColor(color);
    canvas.drawString(str, x, y);
    screenDirty = true;
}

uint16_t GraphicsILI9341::getTextWidth(const char* text, Font font)
{
    if (text == nullptr) return 0;
    setFont(text, font);
    return canvas.textWidth(text);
}

void GraphicsILI9341::drawSprite(const uint16_t* bitmap, int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t spriteScale,  Color transparentColor, bool flipX, bool flipY)
{
    if (bitmap == nullptr || spriteScale == 0) return;

    if (spriteScale == 1 && !flipX && !flipY)
    {
        canvas.pushImage(x, y, w, h, bitmap, transparentColor);
    }
    else
    {
        const float zoomX = flipX ? -static_cast<float>(spriteScale)
                                  :  static_cast<float>(spriteScale);
        const float zoomY = flipY ? -static_cast<float>(spriteScale)
                                  :  static_cast<float>(spriteScale);

        canvas.pushImageRotateZoom(
            x + static_cast<int16_t>((w * spriteScale) / 2), y + static_cast<int16_t>((h * spriteScale) / 2),
            w / 2, h / 2,
            0.0f,
            zoomX, zoomY,
            w, h,
            bitmap,
            transparentColor
        );
    }

    screenDirty = true;
}

void GraphicsILI9341::setViewport(int16_t x, int16_t y)
{
    if (viewportX == x && viewportY == y) return;
    viewportX = x;
    viewportY = y;
    screenDirty = true;
}

void GraphicsILI9341::push()
{
    if (!screenDirty) return;

    if (viewportX != 0 || viewportY != 0) lcd.fillScreen(Graphics::BLACK);
    canvas.pushSprite(&lcd, -viewportX, -viewportY);

    screenDirty = false;
}

bool GraphicsILI9341::readScreenLine(uint16_t y, uint16_t* outPixels, uint16_t pixelCount)
{
    if (outPixels == nullptr) return false;

    if (y >= Display::ILI9341_SCREEN_H || pixelCount < Display::ILI9341_SCREEN_W) return false;

    const int32_t sourceY = static_cast<int32_t>(y) + viewportY;
    for (uint16_t x = 0; x < Display::ILI9341_SCREEN_W; ++x)
    {
        const int32_t sourceX = static_cast<int32_t>(x) + viewportX;
        if (sourceX < 0 || sourceY < 0 ||
            sourceX >= Display::ILI9341_SCREEN_W || sourceY >= Display::ILI9341_SCREEN_H)
        {
            outPixels[x] = static_cast<uint16_t>(Graphics::BLACK);
        }
        else
        {
            outPixels[x] = static_cast<uint16_t>(canvas.readPixel(sourceX, sourceY));
        }
    }
    return true;
}
