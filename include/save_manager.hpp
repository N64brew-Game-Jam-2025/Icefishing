#pragma once

#include <libdragon.h>
#include <cstdint>
#include "menu.hpp"

constexpr uint32_t SAVE_MAGIC = 0x46495348;
constexpr uint8_t SAVE_VERSION = 3;
constexpr int SAVE_MAX_ITEMS_PER_TAB = 32;

namespace EItemId {
    constexpr uint8_t None = 0;
    constexpr uint8_t OakRod = 1;
    constexpr uint8_t SteelRod = 2;
    constexpr uint8_t WoodenRod = 3;
    constexpr uint8_t Worm = 51;
    constexpr uint8_t Minnow = 52;
    constexpr uint8_t Earthworm = 53;
    constexpr uint8_t FishBase = 101;
    constexpr uint8_t Map = 201;
    constexpr uint8_t EngineBelt = 202;
    constexpr uint8_t SparkPlugs = 203;
    constexpr uint8_t CarburetorKit = 204;
    constexpr uint8_t DriveChain = 205;
    constexpr uint8_t FuelLine = 206;
    constexpr uint8_t Skis = 207;
    constexpr uint8_t Track = 208;
}

struct SSaveItem
{
    uint8_t itemId;
    uint8_t quantity;
    uint8_t iconIndex;
    uint8_t flags;
};

struct SSaveData
{
    uint32_t magic;
    uint8_t version;
    uint8_t checksum;
    uint16_t reserved;
    
    uint8_t level;
    uint8_t uniqueFishCaught;
    uint16_t currentExp;
    uint16_t expToNextLevel;
    uint16_t totalFishCaught;
    uint64_t fishCaughtFlags;
    int32_t currency;
    uint32_t playTimeSeconds;
    uint32_t storyFlags;
    
    uint8_t itemCounts[MENU_TAB_COUNT];
    
    SSaveItem items[3][SAVE_MAX_ITEMS_PER_TAB];
};

static_assert(sizeof(SSaveData) <= 512, "SSaveData too large - should fit in 512 bytes");

class CSaveManager
{
public:
    CSaveManager() = default;
    ~CSaveManager();
    
    bool init();
    void close();
    
    bool isAvailable() const { return mInitialized; }
    bool save(const CMenu& menu);
    bool load(CMenu& menu);
    
    void erase();
    bool hasSaveData();
    
    const char* getLastError() const { return mLastError; }
    
    static uint8_t getItemId(const char* itemName, EMenuTab tab);
    static const char* getItemName(uint8_t itemId);
    static const char* getItemModelPath(uint8_t itemId);
    
private:
    uint8_t calculateChecksum(const SSaveData& data);
    void packMenuItem(const SMenuItem& item, SSaveItem& saveItem, EMenuTab tab);
    void unpackMenuItem(const SSaveItem& saveItem, SMenuItem& item);
    
    bool mInitialized{false};
    eeprom_type_t mEepromType{EEPROM_NONE};
    const char* mLastError{nullptr};
    
    static constexpr const char* SAVE_FILE = "save.dat";
};

extern CSaveManager gSaveManager;
