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
    explicit StorageSD(const StorageSDConfig& config);
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

    StorageSDConfig config;
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
