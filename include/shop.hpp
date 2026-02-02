#pragma once

#include <libdragon.h>
#include <t3d/t3d.h>
#include "menu.hpp"
#include "viewport.hpp"
#include "model.hpp"

constexpr int SHOP_MAX_ITEMS = 16;

enum class EShopTab
{
    Rods = 0,
    Bait,
    Items,
    Count
};

enum class EShopMode
{
    Buying,
    Selling
};

enum class EShopState
{
    Closed,
    FadingOut,
    SwitchingRes,
    FadingIn,
    Open,
    ClosingFadeOut,
    ClosingFadeIn
};

struct SShopItem
{
    const char* name;
    const char* description;
    const char* modelPath;
    int price;
    EMenuTab inventoryTab;
    int iconIndex;
};

class CMenu;
class CPlayer;

class CShop
{
public:
    CShop() = default;
    ~CShop();

    void init(int fontId);
    void open(EShopMode mode);
    void close();
    bool update(float deltaTime, joypad_buttons_t pressed, joypad_buttons_t held);
    void draw();
    void drawFade();

    bool isOpen() const { return mState != EShopState::Closed; }
    bool shouldDraw() const { return mState == EShopState::Open || mState == EShopState::FadingIn || mState == EShopState::ClosingFadeOut; }
    bool isFading() const { return mState == EShopState::FadingOut || mState == EShopState::FadingIn || mState == EShopState::ClosingFadeOut || mState == EShopState::ClosingFadeIn; }
    
    EShopState getState() const { return mState; }

    void setMenu(CMenu* menu) { mMenu = menu; }
    void setPlayer(CPlayer* player) { mPlayer = player; }
    
    void setFishPool(const struct SFishData* fish, int count) { mFishPool = fish; mFishPoolCount = count; }

    void addShopItem(const SShopItem& item);
    void addShopItems(const SShopItem* items, int count);
    
    template<size_t N>
    void addShopItems(const SShopItem (&items)[N]) { addShopItems(items, N); }

    void clearShopItems();

private:
    void handleInput(joypad_buttons_t pressed, joypad_buttons_t held);
    void switchTab(int direction);
    void buySelectedItem();
    void sellSelectedItem();
    void updateItemPreview();
    void switchToShopResolution();
    void switchToGameResolution();
    
    void drawGradientBackground();
    void drawBorder(int x, int y, int width, int height, color_t color);
    void drawPanel(int x, int y, int width, int height, color_t bgColor, color_t borderColor);
    EShopTab getItemTab(const SShopItem& item) const;
    int getTabItemCount(EShopTab tab) const;
    const SShopItem* findItemInTab(EShopTab tab, int index) const;
    
    EMenuTab shopTabToMenuTab(EShopTab tab) const;
    int getInventoryItemCount(EShopTab tab) const;
    const SMenuItem* getInventoryItem(EShopTab tab, int index) const;
    int getSellPrice(const SMenuItem* item) const;

    bool mIsOpen = false;
    EShopState mState = EShopState::Closed;
    EShopMode mMode = EShopMode::Buying;
    EShopTab mCurrentTab = EShopTab::Rods;
    int mSelectedIndex = 0;
    int mScrollOffset = 0;
    int mFontId = 0;

    CViewport mPreviewViewport;
    CModel mPreviewModel;
    float mPreviewRotation = 0.0f;
    bool mPreviewModelLoaded = false;

    SShopItem mShopItems[SHOP_MAX_ITEMS];
    int mShopItemCount = 0;

    CMenu* mMenu = nullptr;
    CPlayer* mPlayer = nullptr;
    
    const struct SFishData* mFishPool = nullptr;
    int mFishPoolCount = 0;

    sprite_t* mCursorSprite = nullptr;
    float mCursorTimer = 0.0f;

    static constexpr int ITEMS_PER_PAGE = 8;
    static constexpr float TAB_SWITCH_DELAY = 0.15f;
    static constexpr float SELL_PRICE_RATIO = 0.5f;
    float mTabSwitchTimer = 0.0f;
};
