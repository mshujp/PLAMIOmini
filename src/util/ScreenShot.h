#pragma once

#include "../storage/StorageBase.h"

namespace PLAMIOmini {

class GraphicsILI9341;
class GraphicsSSD1306;

class ScreenShot
{
public:
    static bool save(GraphicsILI9341& graphics, StorageBase& storage,
                     const char* fileName = nullptr);
    static bool save(GraphicsSSD1306& graphics, StorageBase& storage,
                     const char* fileName = nullptr);
};

} // namespace PLAMIOmini
