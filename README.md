# PLAMIO mini

> **AI-Friendly Game Framework for Arduino IDE**

A lightweight game framework designed for AI-assisted game development.

------------------------------------------------------------------------

# Features

PLAMIOmini is a streamlined version of [PLAMIO](https://github.com/mshujp/PLAMIO/) adapted for the Arduino IDE.
For the full feature set, use PLAMIO instead.

-   Supports the creation of a single game.
-   AI-friendly public API
-   Portable game code across supported Arduino.
-   Unified Graphics / Input / Audio / Storage APIs
-   Fixed 30 FPS game loop
-   Built-in SaveData helper
-   2D viewport and scrolling support
-   PWM / I2S audio support
-   SSD1306 / ILI9341 display support
-   AI-oriented documentation and API design

- **Supported platforms**
  - Raspberry Pi Pico family (RP2040 / RP2350)
  - ESP32
  

| Hardware |  |
| :---: | :---: |
| ![](docs/images/01.jpg) | ![](docs/images/02.jpg) |

| ScreenShots | |
| :---: | :---: |
| ![](docs/images/ss01.png) | ![](docs/images/ss02.png) |
| ![](docs/images/ss03.png) | ![](docs/images/ss04.png) |

------------------------------------------------------------------------

# Philosophy

PLAMIO is designed so that both humans and AI can write games using the
same simple API.

Games implement only a small set of interfaces while the runtime manages
graphics, input, audio, storage, and the game loop.

This allows game logic to remain clean, portable, and easy to generate.

------------------------------------------------------------------------

# Build Requirements

## Required tools

-   Arduino IDE

------------------------------------------------------------------------

# Required Libraries
  
- [LovyanGFX](https://github.com/lovyan03/LovyanGFX)


Please install LovyanGFX via the Arduino Library Manager:

`Tools` → `Manage Libraries...` (or Ctrl+Shift+I / Cmd+Shift+I), search for `LovyanGFX`, and click Install.

------------------------------------------------------------------------

# Install

Download this project as a ZIP file, then install it from the Arduino IDE menu:
`Sketch` → `Include Library` → `Add .ZIP Library...`

------------------------------------------------------------------------

# Creating a Game

To create a game, simply create **one class** that inherits from the `PLAMIOmini::GameMini` class.

The PLAMIO system automatically manages the game loop, rendering, input, audio, and storage.

Your game only needs to implement its own game logic.

## Core API

PLAMIO provides the following hardware abstraction interfaces to every game.

Game code does not need to access platform-specific hardware or drivers directly.

| Class | Purpose |
|------|---------|
| `PLAMIOmini::Graphics` | Drawing API for text, shapes, images, and sprites. |
| `PLAMIOmini::Input` | Controller input, button state, repeat, and hold detection. |
| `PLAMIOmini::Audio` | Play sound effects and music. |
| `PLAMIOmini::Storage` | Read and write save data and configuration files. |

For the complete API reference, see:

- [`src/PLAMIOmini.h`](src/PLAMIOmini.h)

## Hardware Configuration

see the example [00B_Hardware_Setup](examples/00B_Hardware_Setup/00B_Hardware_Setup.ino)

## `PLAMIOmini::GameMini` class

Your game class should inherit from the `PLAMIOmini::GameMini` class.

Most games implement their game logic in:

- `onInit()`
- `onUpdate()`
- `onDraw()`
- `onTerminate()`

For the complete `PLAMIOmini::GameMini` class reference, see:

- [`src/PLAMIOmini.h`](src/PLAMIOmini.h)

## AI Workflow

PLAMIO is designed for AI-assisted game development.

see the example [00A_AI_Game_Generation](examples/00A_AI_Game_Generation/00A_AI_Game_Generation.ino)

------------------------------------------------------------------------

# Sample Games

| Sample | Description |
|--------|-------------|
| [00A_AI_Game_Generation](examples/00A_AI_Game_Generation/00A_AI_Game_Generation.ino) | AI-assisted game generation workflow |
| [00B_Hardware_Setup](examples/00B_Hardware_Setup/00B_Hardware_Setup.ino) | Hardware configuration reference |
| [01_Hello_PLAMIO](examples/01_Hello_PLAMIO/01_Hello_PLAMIO.ino) | Minimal PLAMIOmini game |
| [01_Hello_PLAMIO_SSD1306](examples/01_Hello_PLAMIO_SSD1306/01_Hello_PLAMIO_SSD1306.ino) | Minimal PLAMIOmini game |
| [02_Input_Basics](examples/02_Input_Basics/02_Input_Basics.ino) | Button input and movement |
| [03_Graphics_Basics](examples/03_Graphics_Basics/03_Graphics_Basics.ino) | Shapes, colors, fonts, and alignment |
| [04_Audio_Basics](examples/04_Audio_Basics/04_Audio_Basics.ino) | Sound effects and music |
| [05_Save_Data](examples/05_Save_Data/05_Save_Data.ino) | SaveData loading and saving |
| [06_Collision](examples/06_Collision/06_Collision.ino) | Collision detection APIs |
| [07_Animation](examples/07_Animation/07_Animation.ino) | Time-based animation and Tween |
| [08_Breakout](examples/08_Breakout/08_Breakout.ino) | Complete action game |
| [09_Star_Dodge](examples/09_Star_Dodge/09_Star_Dodge.ino) | Avoidance game with effects |
| [10_Reversi](examples/10_Reversi/10_Reversi.ino) | Board game and CPU logic |
| [11_Memory_Tiles](examples/11_Memory_Tiles/11_Memory_Tiles.ino) | Memory game with state transitions |

Each sample is placed under the [`examples`](examples) directory.

## Learning Path

The examples are intended to be completed in numerical order.
Each example introduces one or more new concepts while building on previous examples.

------------------------------------------------------------------------

## Recommended AI

PLAMIO is designed to work with modern AI coding assistants.

Based on current development experience:

| AI | Recommendation | Notes |
|----|---------------|-------|
| **ChatGPT** | **Highly Recommended** | Best overall experience with PLAMIO |
| **Claude** | **Recommended** | Strong at understanding the SDK and generating well-structured game code |
| **Gemini** | **Not Recommended** | Gemini struggles with interpreting the contents of the ZIP file. |
| **Copilot** | **Not Recommended** | Does not currently support zip file uploads, making it difficult to provide the PLAMIO SDK. |
| **Google Search AI Mode** | **Not Recommended** | Does not currently support file uploads, making it difficult to provide the PLAMIO SDK. |

------------------------------------------------------------------------

# Hardware Notes

## SD Card SPI

For SD card builds, the following configuration is recommended and has been verified on both RP2040 and RP2350.

| Peripheral | SPI |
|------------|-----|
| ILI9341 LCD | SPI1 |
| SD Card | SPI0 |

This configuration has been verified on both RP2040 and RP2350 and is recommended for best compatibility.

### SD Card

> [!IMPORTANT]
> PLAMIO supports **SDHC** and **SDXC** memory cards.
> Standard **SD cards (2GB and smaller)** are **not supported**.

> [!WARNING]
> Although PLAMIO provides a software shutdown option, embedded systems can still lose power unexpectedly (for example, due to battery removal or depletion).
> Do **not** store important or irreplaceable data on the SD card.

## PWM Audio

PWM audio supports only **MUTE** or **ON**.
If adjustable volume is required, use an external amplifier or a potentiometer.

------------------------------------------------------------------------

## Supported Hardware

The following hardware configurations have been verified with PLAMIO.

| Target | Display | Input | Audio | Storage | Status |
|--------|---------|-------|-------|---------|--------|
| RP2040 | SSD1306 | GPIO Buttons | PWM | SD | ✅ Verified |
| RP2040 | ILI9341 | GPIO Buttons | PWM | SD | ✅ Verified |
| RP2350 | ILI9341 | SNES | I2S | SD | ✅ Verified |


Additional hardware configurations can be supported by creating a new hardware profile under:

```text
system/platform/pico/boards/
```

Only the configurations listed above have been verified.

------------------------------------------------------------------------

# License

MIT License
