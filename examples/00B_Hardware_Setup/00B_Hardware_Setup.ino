/*
===============================================================================
 PLAMIOmini Example
 00B_Hardware_Setup
===============================================================================

Copy the configuration for the hardware you want to use.
Change each value to match your hardware.

Only one configuration from each section should be used.

This file is a configuration reference and an interface compile check.
Select one configuration from each category near the end of the file.
*/

#include <PLAMIOmini.h>
using namespace PLAMIOmini;

/*
===============================================================================
 Graphics
===============================================================================
*/

// -----------------------------------------------------------------------------
// GraphicsILI9341
// -----------------------------------------------------------------------------

GraphicsILI9341Config ili9341Config = {
    .spiHost = 0,  // 0 or 1. Select the SPI host that matches the pins.
                   // When using an SD card, SPI0 for SD and SPI1 for the display is recommended.
    .spiWriteFreq    = 62500000,
    .clkPin          = -1,      // or SCK
    .dataPin         = -1,      // or MOSI
    .dcPin           = -1,
    .csPin           = -1,
    .resetPin        = -1,
    .backlightPin    = -1,
    .lcdRotate  = 0,  // 0: Normal  3: Rotated 180 degrees
};

// -----------------------------------------------------------------------------
// GraphicsSSD1306
// -----------------------------------------------------------------------------

GraphicsSSD1306Config ssd1306Config = {
    .i2cPort    = 0,            // 0 or 1
    .i2cAddr    = 0x3C,         // 0x3C or 0x3D, depending on the module
    .sdaPin     = -1,
    .sclPin     = -1,
    .resetPin   = -1,
    .oledRotate = 0,  // 0: Normal  2: Rotated 180 degrees
};

/*
===============================================================================
 Input
===============================================================================
*/

// -----------------------------------------------------------------------------
// InputGpioButtons
// -----------------------------------------------------------------------------

ButtonMapping buttonMapping = {
    .UP       = -1,
    .DOWN     = -1,
    .LEFT     = -1,
    .RIGHT    = -1,
    .A        = -1,
    .B        = -1,
    .X        = -1,
    .Y        = -1,
    .L        = -1,
    .R        = -1,
    .START    = -1,
    .SELECT   = -1,
    .VOL_UP   = -1,
    .VOL_DOWN = -1,
    .HOME     = -1,
    .MUTE     = -1,
};

InputGpioButtonsConfig gpioButtonsConfig = {
    .buttonMapping = buttonMapping,
};


// -----------------------------------------------------------------------------
// InputSnes
// -----------------------------------------------------------------------------

InputSnesConfig snesConfig = {
    .clkPin  = -1,
    .latPin  = -1,
    .dataPin = -1,

    .buttonMapping = {
        .UP       = -1,
        .DOWN     = -1,
        .LEFT     = -1,
        .RIGHT    = -1,
        .A        = -1,
        .B        = -1,
        .X        = -1,
        .Y        = -1,
        .L        = -1,
        .R        = -1,
        .START    = -1,
        .SELECT   = -1,
        .VOL_UP   = -1,
        .VOL_DOWN = -1,
        .HOME     = -1,
        .MUTE     = -1,
    },
};

/*
===============================================================================
 Audio
===============================================================================
*/

// -----------------------------------------------------------------------------
// AudioI2S
// -----------------------------------------------------------------------------

AudioI2SConfig i2sConfig = {
    .bclkPin = -1,
    .wsPin = -1, // LRCLK/WS; on Pico(rp2040/rp2350), wsPin must equal bclkPin + 1.
    .dataPin = -1,
};

// -----------------------------------------------------------------------------
// AudioPWM
// -----------------------------------------------------------------------------

AudioPWMConfig pwmConfig = {
    .pwmPin = -1,
};

// -----------------------------------------------------------------------------
// AudioStub
//
// Use when audio hardware is not supported or not used.
// -----------------------------------------------------------------------------

AudioStubConfig stubAudioConfig;


/*
===============================================================================
 Storage
===============================================================================
*/

// -----------------------------------------------------------------------------
// StorageSD
// -----------------------------------------------------------------------------

StorageSDConfig sdConfig = {
    .spiHost = 0, // 0 or 1. Select the SPI host that matches the pins.
                  // When using a display, SPI0 for SD and SPI1 for the display is recommended.
    .misoPin  = -1,
    .sckPin   = -1,
    .mosiPin  = -1,
    .csPin    = -1,
    .baudRate = 12000000,
};

// -----------------------------------------------------------------------------
// StorageEEPROM  Default values are recommended
// -----------------------------------------------------------------------------

StorageEEPROMConfig eepromConfig = {
    .magic      = 0x504d,
    .version    = 1,
    .eepromSize = 4096,
};

// -----------------------------------------------------------------------------
// StorageStub
//
// Use when storage hardware is not supported or not used.
// -----------------------------------------------------------------------------

StorageStubConfig stubStorageConfig;




// ----------------------------------------
// ----------------------------------------

class MyGame : public GameMini
{
public:
    void onInit(Storage& storage) override {}
    void onUpdate(Input& input, Audio& audio, Storage& storage, float deltaSec) override {}
    bool onDraw(Graphics& graphics, bool requestFullRedraw) override
    {
        if (!requestFullRedraw && !dirty)
        {
            return false;
        }
        graphics.clearScreen();

        dirty = false;
        return true;
    }
    void onTerminate(Storage& storage) override {}
};

// Select one configuration from each category.
GraphicsConfig graphicsConfig = ili9341Config;  // or: ssd1306Config
InputConfig inputConfig = gpioButtonsConfig;    // or: snesConfig
AudioConfig audioConfig = stubAudioConfig;      // or: i2sConfig, pwmConfig
StorageConfig storageConfig = stubStorageConfig; // or: sdConfig, eepromConfig
MyGame game;

void setup()
{
    PLAMIOmini::start(graphicsConfig, inputConfig, storageConfig, audioConfig, game);
}

void loop()
{
}
