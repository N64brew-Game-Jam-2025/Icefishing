#pragma once

#include <libdragon.h>
#include <cstdint>

constexpr int MENU_MAX_ITEMS = 32;
constexpr int MENU_ITEM_NAME_LEN = 32;
constexpr int MENU_TAB_COUNT = 4;

enum class EMenuState
{
    Closed,
    Opening,
    Open,
    Closing
};

enum class EMenuTab
{
    Stats = 0,
    FishingRods,
    Bait,
    MiscItems,
    Count
};

constexpr int MENU_ITEM_MODEL_PATH_LEN = 64;

struct SMenuItem
{
    char name[MENU_ITEM_NAME_LEN];
    char modelPath[MENU_ITEM_MODEL_PATH_LEN];
    int quantity;
    int iconIndex;
    bool equipped;
};

namespace EStoryFlag {
    constexpr uint32_t MetShopkeeper = 1 << 0;
    constexpr uint32_t TutorialDone  = 1 << 1;
}

struct SPlayerStats
{
    int level;
    int currentExp;
    int expToNextLevel;
    int totalFishCaught;
    int uniqueFishCaught;
    int totalFishSpecies;
    uint64_t fishCaughtFlags;
    int currency;
    float playTimeSeconds;
    uint32_t storyFlags;
    
    bool hasStoryFlag(uint32_t flag) const { return (storyFlags & flag) != 0; }
    void setStoryFlag(uint32_t flag) { storyFlags |= flag; }
};

class CMenu
{
public:
    CMenu() = default;
    ~CMenu();

    void init(int fontId);
    void loadIcons(const char* spritePath);
    void toggle();
    void open();
    void close();
    bool update(float deltaTime, joypad_buttons_t pressed, joypad_buttons_t held);
    void draw();
    bool isActive() const { return mState != EMenuState::Closed; }
    bool isOpen() const { return mState == EMenuState::Open; }

    void setPlayerStats(const SPlayerStats& stats) { mPlayerStats = stats; }
    void setLevel(int level) { mPlayerStats.level = level; }
    void setExp(int current, int toNext) { mPlayerStats.currentExp = current; mPlayerStats.expToNextLevel = toNext; }
    void addFishCaught(int count = 1) { mPlayerStats.totalFishCaught += count; }
    void registerFishCaught(int fishIndex) {
        if (fishIndex < 0 || fishIndex >= 64) return;
        uint64_t bit = 1ULL << fishIndex;
        if (!(mPlayerStats.fishCaughtFlags & bit)) {
            mPlayerStats.fishCaughtFlags |= bit;
            mPlayerStats.uniqueFishCaught++;
        }
    }
    void setTotalFishSpecies(int count) { mPlayerStats.totalFishSpecies = count; }
    void setCurrency(int amount) { mPlayerStats.currency = amount; }
    void addCurrency(int amount) { mPlayerStats.currency += amount; }
    void setPlayTime(float seconds) { mPlayerStats.playTimeSeconds = seconds; }
    void addPlayTime(float seconds) { mPlayerStats.playTimeSeconds += seconds; }
    const SPlayerStats& getPlayerStats() const { return mPlayerStats; }

    bool addItem(EMenuTab tab, const char* name, int quantity = 1, int iconIndex = -1, const char* modelPath = nullptr);
    bool removeItem(EMenuTab tab, int index);
    bool updateItemQuantity(EMenuTab tab, int index, int newQuantity);
    int findItem(EMenuTab tab, const char* name);
    const SMenuItem* getItem(EMenuTab tab, int index) const;
    int getItemCount(EMenuTab tab) const;
    void equipFishingRod(int index);
    void equipBait(int index);
    
    int getEquippedRodIndex() const;
    int getEquippedBaitIndex() const;
    
    const char* getEquippedRodModelPath() const;
    
    void setPlayer(class CPlayer* player) { mPlayer = player; }

private:
    void startOpenAnimation();
    void startCloseAnimation();
    void navigateTabs(int direction);
    void navigateItems(int direction);
    void handleSelection();
    
    void drawBackground();
    void drawTabs();
    void drawTabContent();
    void drawStatsTab();
    void drawInventoryTab(EMenuTab tab);
    void drawSnowflakeDecor();
    void drawIceBorder();
    
    float easeOutBack(float t);
    float easeInBack(float t);
    float easeOutQuad(float t);
    
    void formatPlayTime(char* buffer, int bufferSize, float seconds);
    
    color_t calculateGradientColor(color_t top, color_t bottom, float t, float alpha = 1.0f);
    color_t applyAlpha(color_t color, float alpha);
    void drawBorder(int x, int y, int width, int height, int borderWidth, color_t color);
    void setStandardRenderMode();
    void setAlphaBlitMode();
    void freeSprite(sprite_t*& sprite);
    bool validateTabIndex(int tabIndex) const;
    void drawScrollArrow(bool isUp, int x, int y, color_t color);
    void unequipAllInTab(EMenuTab tab);
    int getEquippedItemIndex(EMenuTab tab) const;
    void blitSpriteSlice(sprite_t* sprite, int x, int y, int col, int row, int size);
    void drawSnowflake(int cx, int cy, color_t color);

    EMenuState mState = EMenuState::Closed;
    EMenuTab mCurrentTab = EMenuTab::Stats;
    EMenuTab mPreviousTab = EMenuTab::Stats;
    bool mTabTransitioning = false;
    float mTabTransitionProgress = 0.0f;
    float mTabTransitionDuration = 0.25f;
    int mTabTransitionDirection = 0;
    int mSelectedIndex = 0;
    int mScrollOffset = 0;
    float mAnimProgress = 0.0f;
    float mAnimDuration = 0.35f;
    
    int mFontId = 1;
    int mMenuX = 20;
    int mMenuY = 20;
    int mMenuWidth = 216;
    int mMenuHeight = 200;
    int mTabHeight = 20;
    int mItemHeight = 16;
    int mVisibleItems = 10;
    int mPadding = 8;
    
    color_t mBgColorTop = {0x1A, 0x3A, 0x5C, 0xF0};
    color_t mBgColorBottom = {0x0F, 0x1E, 0x3C, 0xF0};
    color_t mTabActiveColor = {0x4A, 0x8A, 0xC0, 0xFF};
    color_t mTabInactiveColor = {0x2A, 0x4A, 0x70, 0xC0};
    color_t mBorderColor = {0xAA, 0xDD, 0xFF, 0xFF};
    color_t mTextColor = {0xFF, 0xFF, 0xFF, 0xFF};
    color_t mHighlightColor = {0x60, 0xA0, 0xE0, 0xFF};
    color_t mAccentColor = {0x80, 0xC0, 0xFF, 0xFF};
    color_t mSnowColor = {0xFF, 0xFF, 0xFF, 0xB0};
    
    SPlayerStats mPlayerStats{};
    
    class CPlayer* mPlayer = nullptr;
    
    SMenuItem mItems[MENU_TAB_COUNT][MENU_MAX_ITEMS]{};
    int mItemCounts[MENU_TAB_COUNT] = {0, 0, 0, 0};
    
    sprite_t* mIconSprite = nullptr;
    int mIconSize = 16;
    
    sprite_t* mCursorSprite = nullptr;
    sprite_t* mCheckSprite = nullptr;
    sprite_t* mButtonSprite = nullptr;
    
    const char* mTabNames[MENU_TAB_COUNT] = {"STATS", "RODS", "BAIT", "ITEMS"};
    
    float mSnowflakeTimer = 0.0f;
    float mShimmerTimer = 0.0f;
    float mCursorTimer = 0.0f;
    color_t mCursorColor = {0xFF, 0xDD, 0x55, 0xFF};
};