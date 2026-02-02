#include "save_manager.hpp"
#include <cstring>

CSaveManager gSaveManager;

static const eepfs_entry_t sSaveFiles[] = {
    { "save.dat", sizeof(SSaveData) }
};

struct SItemDef {
    uint8_t id;
    const char* name;
    const char* modelPath;
};

static const SItemDef sRodTable[] = {
    { EItemId::OakRod,    "Oak Rod",    "rom:/rod-redgolden.t3dm" },
    { EItemId::SteelRod,  "Steel Rod",  "rom:/rod-redgolden.t3dm" },
    { EItemId::WoodenRod, "Wooden Rod", "rom:/rod-redgolden.t3dm" },
    { 0, nullptr, nullptr }
};

static const SItemDef sBaitTable[] = {
    { EItemId::Worm,      "Worm",      nullptr },
    { EItemId::Minnow,    "Minnow",    nullptr },
    { EItemId::Earthworm, "Earthworm", nullptr },
    { 0, nullptr, nullptr }
};

static const char* sFishNames[] = {
    "Minnow",
    "Bluegill",
    "Perch",
    "Carp",
    "Sunfish",
    "Chub",
    "Shiner",
    "Dace",
    "Roach",
    "Bream",
    "Goldfish",
    "Gudgeon",
    "Bass",
    "Trout",
    "Catfish",
    "Walleye",
    "Crappie",
    "Eel",
    "Tench",
    "Zander",
    "Grayling",
    "Barbel",
    "Pike",
    "Salmon",
    "Sturgeon",
    "Muskie",
    "Arctic Char",
    "Lake Trout",
    "Arapaima",
    "Taimen",
    "Golden Carp",
    "Ghost Koi",
    "Ancient Lungfish",
    "Void Bass",
    "Crystal Perch",
    "The Old One",
    nullptr
};
constexpr int sFishCount = (sizeof(sFishNames) / sizeof(sFishNames[0])) - 1;

static const SItemDef sOtherTable[] = {
    { EItemId::Map, "Map", nullptr },
    { EItemId::EngineBelt, "Engine Belt", nullptr },
    { EItemId::SparkPlugs, "Spark Plugs", nullptr },
    { EItemId::CarburetorKit, "Carburetor Kit", nullptr },
    { EItemId::DriveChain, "Drive Chain", nullptr },
    { EItemId::FuelLine, "Fuel Line", nullptr },
    { EItemId::Skis, "Skis", nullptr },
    { EItemId::Track, "Track", nullptr },
    { 0, nullptr, nullptr }
};

uint8_t CSaveManager::getItemId(const char* itemName, EMenuTab tab)
{
    if (!itemName || itemName[0] == '\0') return EItemId::None;
    
    switch (tab) {
        case EMenuTab::FishingRods:
            for (const SItemDef* def = sRodTable; def->name; ++def) {
                if (strcmp(def->name, itemName) == 0) return def->id;
            }
            break;
            
        case EMenuTab::Bait:
            for (const SItemDef* def = sBaitTable; def->name; ++def) {
                if (strcmp(def->name, itemName) == 0) return def->id;
            }
            break;
            
        case EMenuTab::MiscItems:
            for (int i = 0; i < sFishCount && sFishNames[i]; ++i) {
                if (strcmp(sFishNames[i], itemName) == 0) {
                    return EItemId::FishBase + i;
                }
            }
            for (const SItemDef* def = sOtherTable; def->name; ++def) {
                if (strcmp(def->name, itemName) == 0) return def->id;
            }
            break;
            
        default:
            break;
    }
    
    return EItemId::None;
}

const char* CSaveManager::getItemName(uint8_t itemId)
{
    if (itemId == EItemId::None) return nullptr;
    
    if (itemId >= 1 && itemId <= 50) {
        for (const SItemDef* def = sRodTable; def->name; ++def) {
            if (def->id == itemId) return def->name;
        }
    }
    else if (itemId >= 51 && itemId <= 100) {
        for (const SItemDef* def = sBaitTable; def->name; ++def) {
            if (def->id == itemId) return def->name;
        }
    }
    else if (itemId >= EItemId::FishBase && itemId < EItemId::FishBase + 100) {
        int fishIndex = itemId - EItemId::FishBase;
        if (fishIndex < sFishCount && sFishNames[fishIndex]) {
            return sFishNames[fishIndex];
        }
    }
    else if (itemId >= 201) {
        for (const SItemDef* def = sOtherTable; def->name; ++def) {
            if (def->id == itemId) return def->name;
        }
    }
    
    return nullptr;
}

const char* CSaveManager::getItemModelPath(uint8_t itemId)
{
    if (itemId == EItemId::None) return nullptr;
    
    if (itemId >= 1 && itemId <= 50) {
        for (const SItemDef* def = sRodTable; def->name; ++def) {
            if (def->id == itemId) return def->modelPath;
        }
    }
    else if (itemId >= 51 && itemId <= 100) {
        for (const SItemDef* def = sBaitTable; def->name; ++def) {
            if (def->id == itemId) return def->modelPath;
        }
    }
    
    return nullptr;
}

CSaveManager::~CSaveManager()
{
    close();
}

bool CSaveManager::init()
{
    if (mInitialized) return true;
    
    mEepromType = eeprom_present();
    if (mEepromType == EEPROM_NONE) {
        mLastError = "No EEPROM detected";
        return false;
    }
    
    int result = eepfs_init(sSaveFiles, 1);
    if (result != EEPFS_ESUCCESS) {
        switch (result) {
            case EEPFS_EBADFS:
                mLastError = "Bad filesystem";
                break;
            case EEPFS_ENOMEM:
                mLastError = "Not enough memory";
                break;
            default:
                mLastError = "Unknown error";
                break;
        }
        return false;
    }
    
    if (!eepfs_verify_signature()) {
        eepfs_wipe();
    }
    
    mInitialized = true;
    mLastError = nullptr;
    return true;
}

void CSaveManager::close()
{
    if (mInitialized) {
        eepfs_close();
        mInitialized = false;
    }
}

uint8_t CSaveManager::calculateChecksum(const SSaveData& data)
{
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&data);
    uint8_t sum = 0;
    for (size_t i = 6; i < sizeof(SSaveData); ++i) {
        sum += bytes[i];
    }
    return sum;
}

void CSaveManager::packMenuItem(const SMenuItem& item, SSaveItem& saveItem, EMenuTab tab)
{
    memset(&saveItem, 0, sizeof(SSaveItem));
    
    saveItem.itemId = getItemId(item.name, tab);
    saveItem.quantity = static_cast<uint8_t>(item.quantity > 255 ? 255 : item.quantity);
    saveItem.iconIndex = static_cast<uint8_t>(item.iconIndex & 0xFF);
    saveItem.flags = item.equipped ? 0x01 : 0x00;
}

void CSaveManager::unpackMenuItem(const SSaveItem& saveItem, SMenuItem& item)
{
    memset(&item, 0, sizeof(SMenuItem));
    
    if (saveItem.itemId == EItemId::None) return;
    
    const char* name = getItemName(saveItem.itemId);
    if (name) {
        strncpy(item.name, name, MENU_ITEM_NAME_LEN - 1);
        item.name[MENU_ITEM_NAME_LEN - 1] = '\0';
    }
    
    const char* modelPath = getItemModelPath(saveItem.itemId);
    if (modelPath) {
        strncpy(item.modelPath, modelPath, MENU_ITEM_MODEL_PATH_LEN - 1);
        item.modelPath[MENU_ITEM_MODEL_PATH_LEN - 1] = '\0';
    }
    
    item.quantity = saveItem.quantity;
    item.iconIndex = saveItem.iconIndex;
    item.equipped = (saveItem.flags & 0x01) != 0;
}

bool CSaveManager::save(const CMenu& menu)
{
    if (!mInitialized) {
        mLastError = "Save system not initialized";
        return false;
    }
    
    SSaveData data;
    memset(&data, 0, sizeof(data));
    
    data.magic = SAVE_MAGIC;
    data.version = SAVE_VERSION;
    
    const SPlayerStats& stats = menu.getPlayerStats();
    data.level = static_cast<uint8_t>(stats.level > 255 ? 255 : stats.level);
    data.currentExp = static_cast<uint16_t>(stats.currentExp > 65535 ? 65535 : stats.currentExp);
    data.expToNextLevel = static_cast<uint16_t>(stats.expToNextLevel > 65535 ? 65535 : stats.expToNextLevel);
    data.totalFishCaught = static_cast<uint16_t>(stats.totalFishCaught > 65535 ? 65535 : stats.totalFishCaught);
    data.uniqueFishCaught = static_cast<uint8_t>(stats.uniqueFishCaught > 255 ? 255 : stats.uniqueFishCaught);
    data.fishCaughtFlags = stats.fishCaughtFlags;
    data.currency = stats.currency;
    data.playTimeSeconds = static_cast<uint32_t>(stats.playTimeSeconds);
    data.storyFlags = stats.storyFlags;
    
    for (int tab = 1; tab < MENU_TAB_COUNT; ++tab) {
        EMenuTab menuTab = static_cast<EMenuTab>(tab);
        int count = menu.getItemCount(menuTab);
        int saveCount = count > SAVE_MAX_ITEMS_PER_TAB ? SAVE_MAX_ITEMS_PER_TAB : count;
        data.itemCounts[tab] = static_cast<uint8_t>(saveCount);
        
        for (int i = 0; i < saveCount; ++i) {
            const SMenuItem* item = menu.getItem(menuTab, i);
            if (item) {
                packMenuItem(*item, data.items[tab - 1][i], menuTab);
            }
        }
    }
    
    data.checksum = calculateChecksum(data);
    
    int result = eepfs_write(SAVE_FILE, &data, sizeof(data));
    if (result != EEPFS_ESUCCESS) {
        mLastError = "Failed to write save data";
        return false;
    }
    
    mLastError = nullptr;
    return true;
}

bool CSaveManager::load(CMenu& menu)
{
    if (!mInitialized) {
        mLastError = "Save system not initialized";
        return false;
    }
    
    SSaveData data;
    memset(&data, 0, sizeof(data));
    
    int result = eepfs_read(SAVE_FILE, &data, sizeof(data));
    if (result != EEPFS_ESUCCESS) {
        mLastError = "Failed to read save data";
        return false;
    }
    
    if (data.magic != SAVE_MAGIC) {
        mLastError = "Invalid save data (bad magic)";
        return false;
    }
    
    if (data.version != SAVE_VERSION) {
        mLastError = "Incompatible save version";
        return false;
    }
    
    if (data.checksum != calculateChecksum(data)) {
        mLastError = "Corrupted save data (bad checksum)";
        return false;
    }
    
    SPlayerStats stats = menu.getPlayerStats();
    stats.level = data.level;
    stats.currentExp = data.currentExp;
    stats.expToNextLevel = data.expToNextLevel;
    stats.totalFishCaught = data.totalFishCaught;
    stats.uniqueFishCaught = data.uniqueFishCaught;
    stats.fishCaughtFlags = data.fishCaughtFlags;
    stats.currency = data.currency;
    stats.playTimeSeconds = static_cast<float>(data.playTimeSeconds);
    stats.storyFlags = data.storyFlags;
    menu.setPlayerStats(stats);
    
    for (int tab = 1; tab < MENU_TAB_COUNT; ++tab) {
        EMenuTab menuTab = static_cast<EMenuTab>(tab);
        int count = data.itemCounts[tab];
        
        for (int i = 0; i < count && i < SAVE_MAX_ITEMS_PER_TAB; ++i) {
            const SSaveItem& saveItem = data.items[tab - 1][i];
            if (saveItem.itemId != EItemId::None) {
                SMenuItem item;
                unpackMenuItem(saveItem, item);
                
                if (item.name[0] != '\0') {
                    menu.addItem(menuTab, item.name, item.quantity, item.iconIndex, 
                                 item.modelPath[0] ? item.modelPath : nullptr);
                    
                    if (item.equipped) {
                        int idx = menu.findItem(menuTab, item.name);
                        if (idx >= 0) {
                            if (menuTab == EMenuTab::FishingRods) {
                                menu.equipFishingRod(idx);
                            } else if (menuTab == EMenuTab::Bait) {
                                menu.equipBait(idx);
                            }
                        }
                    }
                }
            }
        }
    }
    
    mLastError = nullptr;
    return true;
}

void CSaveManager::erase()
{
    if (mInitialized) {
        eepfs_wipe();
    }
}

bool CSaveManager::hasSaveData()
{
    if (!mInitialized) return false;
    
    SSaveData data;
    int result = eepfs_read(SAVE_FILE, &data, sizeof(data));
    if (result != EEPFS_ESUCCESS) return false;
    
    return data.magic == SAVE_MAGIC && 
           data.version == SAVE_VERSION &&
           data.checksum == calculateChecksum(data);
}
