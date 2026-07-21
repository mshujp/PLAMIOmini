#pragma once

#include "StorageBase.h"
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

namespace PLAMIOmini {

class StorageSDFile : public StorageBaseFile
{
public:
    ~StorageSDFile() override;

    bool openRead(const char* path);
    bool openWrite(const char* path);
    bool isOpen() const override;
    uint32_t size() const override;
    uint32_t read(void* buffer, uint32_t bytes) override;
    uint32_t write(const void* buffer, uint32_t bytes) override;
    void close() override;
    bool closeWrite() override;

private:
    ::File file;
    uint32_t fileSize = 0;
};

class StorageSD : public StorageBase
{
public:
    struct Config
    {
        uint8_t spiHost = 0;
        int8_t misoPin = -1;
        int8_t sckPin = -1;
        int8_t mosiPin = -1;
        int8_t csPin = -1;
        uint32_t baudRate = 12 * 1000 * 1000;
    };

    explicit StorageSD(const Config& config);
    ~StorageSD() override;

    bool begin() override;
    void end() override;
    bool isAvailable() const override { return sdAvailable; }

    File* openRead(const char* path) override;
    File* openRead(const char* gameId, const char* fileName) override;
    bool directoryExists(const char* path) override;
    bool fileExists(const char* path) override;

protected:
    StorageBaseFile* openWrite(const char* gameId, const char* fileName) override;

private:
    static constexpr size_t PATH_MAX_LENGTH = 128;
    static constexpr const char* ROOT_DIR = "/PLAMIO_Games";

    Config config;
    bool sdAvailable = false;
    SPIClass* spi = nullptr;
    bool ownsSpi = false;
    StorageSDFile fileSlot;

    bool makeDataPath(char* output, size_t outputSize,
                      const char* gameId, const char* fileName) const;
    bool ensureDirectory(const char* path);
    static bool isValidFileName(const char* fileName);
};

} // namespace PLAMIOmini
