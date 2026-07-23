#include "StorageSD.h"

#include <cstring>

namespace PLAMIOmini {

StorageSDFile::~StorageSDFile()
{
    close();
}

bool StorageSDFile::openRead(const char* path)
{
    close();
    file = SD.open(path, FILE_READ);
    if (!file || file.isDirectory()) return false;

    mode = OpenMode::READ;
    fileSize = static_cast<uint32_t>(file.size());
    return true;
}

bool StorageSDFile::openWrite(const char* path)
{
    close();
    if (SD.exists(path)) SD.remove(path);

    file = SD.open(path, FILE_WRITE);
    if (!file) return false;

    mode = OpenMode::WRITE;
    fileSize = 0;
    return true;
}

bool StorageSDFile::isOpen() const
{
    return mode != OpenMode::CLOSED && static_cast<bool>(file);
}

uint32_t StorageSDFile::size() const
{
    return isOpen() ? fileSize : 0;
}

uint32_t StorageSDFile::read(void* buffer, uint32_t bytes)
{
    if (mode != OpenMode::READ || buffer == nullptr) return 0;
    return static_cast<uint32_t>(file.read(static_cast<uint8_t*>(buffer), bytes));
}

uint32_t StorageSDFile::write(const void* buffer, uint32_t bytes)
{
    if (mode != OpenMode::WRITE || (buffer == nullptr && bytes > 0)) return 0;

    const uint32_t written = static_cast<uint32_t>(
        file.write(static_cast<const uint8_t*>(buffer), bytes));
    fileSize += written;
    return written;
}

void StorageSDFile::close()
{
    if (file) file.close();
    mode = OpenMode::CLOSED;
    fileSize = 0;
}

bool StorageSDFile::closeWrite()
{
    if (mode != OpenMode::WRITE) return false;

    file.flush();
    file.close();
    mode = OpenMode::CLOSED;
    fileSize = 0;
    return true;
}

StorageSD::StorageSD(const StorageSDConfig& value)
    : config(value)
{
}

StorageSD::~StorageSD()
{
    end();
}

bool StorageSD::begin()
{
    if (sdAvailable) return true;
    if (config.csPin < 0) return false;

#if defined(ARDUINO_ARCH_RP2040)
    SPIClassRP2040* rpSpi = config.spiHost == 1 ? &SPI1 : &SPI;
    if (config.misoPin >= 0 && !rpSpi->setRX(config.misoPin)) return false;
    if (config.mosiPin >= 0 && !rpSpi->setTX(config.mosiPin)) return false;
    if (config.sckPin >= 0 && !rpSpi->setSCK(config.sckPin)) return false;
    spi = rpSpi;
    spi->begin();
#elif defined(ARDUINO_ARCH_ESP32)
#if defined(VSPI) && defined(HSPI)
    const uint8_t bus = config.spiHost == 1 ? HSPI : VSPI;
#elif defined(FSPI)
    const uint8_t bus = FSPI;
#else
    const uint8_t bus = SPI2_HOST;
#endif
    spi = new SPIClass(bus);
    if (spi == nullptr) return false;

    ownsSpi = true;
    spi->begin(config.sckPin, config.misoPin, config.mosiPin, config.csPin);
#else
    spi = &SPI;
    spi->begin();
#endif

#if defined(ARDUINO_ARCH_RP2040)
    sdAvailable = SD.begin(config.csPin, config.baudRate, *spi);
#else
    sdAvailable = SD.begin(config.csPin, *spi, config.baudRate);
#endif
    if (!sdAvailable)
    {
        end();
        return false;
    }
    if (!ensureDirectory(ROOT_DIR))
    {
        end();
        return false;
    }
    return true;
}

void StorageSD::end()
{
    fileSlot.close();
    if (sdAvailable) SD.end();

    sdAvailable = false;
    if (spi != nullptr) spi->end();
    if (ownsSpi) delete spi;

    spi = nullptr;
    ownsSpi = false;
}

bool StorageSD::isValidFileName(const char* fileName)
{
    if (fileName == nullptr || fileName[0] == '\0') return false;

    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(fileName); *p; ++p)
    {
        if (*p < 0x20 || *p == 0x7f || *p == '/' || *p == '\\' || *p == ':') return false;
    }
    return true;
}

bool StorageSD::makeDataPath(char* output, size_t outputSize,
                             const char* gameId, const char* fileName) const
{
    if (output == nullptr || !isValidGameId(gameId) || !isValidFileName(fileName)) return false;

    const int count = snprintf(output, outputSize, "%s/%s/%s", ROOT_DIR, gameId, fileName);
    return count >= 0 && static_cast<size_t>(count) < outputSize;
}

bool StorageSD::ensureDirectory(const char* path)
{
    if (!sdAvailable || path == nullptr || path[0] != '/') return false;

    char partial[PATH_MAX_LENGTH] = {};
    const size_t length = strlen(path);
    if (length >= sizeof(partial)) return false;

    for (size_t i = 1; i <= length; ++i)
    {
        if (path[i] == '/' || path[i] == '\0')
        {
            memcpy(partial, path, i);
            partial[i] = '\0';
            if (!SD.exists(partial) && !SD.mkdir(partial)) return false;
        }
    }
    return true;
}

Storage::File* StorageSD::openRead(const char* path)
{
    if (!sdAvailable || path == nullptr) return nullptr;

    fileSlot.close();
    return fileSlot.openRead(path) ? &fileSlot : nullptr;
}

Storage::File* StorageSD::openRead(const char* gameId, const char* fileName)
{
    if (!sdAvailable) return nullptr;

    char path[PATH_MAX_LENGTH];
    if (!makeDataPath(path, sizeof(path), gameId, fileName)) return nullptr;
    return openRead(path);
}

StorageBaseFile* StorageSD::openWrite(const char* gameId, const char* fileName)
{
    if (!sdAvailable || !isValidGameId(gameId) || !isValidFileName(fileName)) return nullptr;

    char directory[PATH_MAX_LENGTH];
    const int count = snprintf(directory, sizeof(directory), "%s/%s", ROOT_DIR, gameId);
    if (count < 0 || static_cast<size_t>(count) >= sizeof(directory) ||
        !ensureDirectory(directory)) return nullptr;

    char path[PATH_MAX_LENGTH];
    if (!makeDataPath(path, sizeof(path), gameId, fileName)) return nullptr;

    fileSlot.close();
    return fileSlot.openWrite(path) ? &fileSlot : nullptr;
}

bool StorageSD::directoryExists(const char* path)
{
    if (!sdAvailable || path == nullptr) return false;

    ::File entry = SD.open(path, FILE_READ);
    const bool result = entry && entry.isDirectory();
    if (entry) entry.close();
    return result;
}

bool StorageSD::fileExists(const char* path)
{
    if (!sdAvailable || path == nullptr) return false;

    ::File entry = SD.open(path, FILE_READ);
    const bool result = entry && !entry.isDirectory();
    if (entry) entry.close();
    return result;
}

} // namespace PLAMIOmini
