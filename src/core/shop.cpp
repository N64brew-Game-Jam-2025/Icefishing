#include "shop.hpp"
#include "player.hpp"
#include "player_state.hpp"
#include "rdpq_text.h"
#include "wipe.hpp"
#include "save_manager.hpp"
#include <t3d/t3d.h>
#include <cstring>
#include <cmath>

static CWhiteFade gShopFade;

CShop::~CShop()
{
    if (mCursorSprite) {
        sprite_free(mCursorSprite);
        mCursorSprite = nullptr;
    }
    
    if (mPreviewModelLoaded) {
        mPreviewModel.unload();
    }
}

void CShop::init(int fontId)
{
    mFontId = fontId;
    mIsOpen = false;
    mState = EShopState::Closed;
    mCurrentTab = EShopTab::Rods;
    mSelectedIndex = 0;
    mScrollOffset = 0;
    mShopItemCount = 0;
    mPreviewModelLoaded = false;
    mCursorTimer = 0.0f;

    mCursorSprite = sprite_load("rom:/sflk.ia16.sprite");

    mPreviewViewport.init();
    mPreviewViewport.setProjection(45.0f, 1.0f, 100.0f);
    
    gShopFade.init();
}

void CShop::open(EShopMode mode)
{
    debugf("CShop::open() called with mode=%d (Buying=0, Selling=1)\n", (int)mode);
    mIsOpen = true;
    mMode = mode;
    mState = EShopState::FadingOut;
    mCurrentTab = EShopTab::Rods;
    mSelectedIndex = 0;
    mScrollOffset = 0;
    mPreviewRotation = 0.0f;
    
    gShopFade.fadeOut(0.3f);
    
    debugf("CShop mode is now: %s\n", (mMode == EShopMode::Selling) ? "SELLING" : "BUYING");
}

void CShop::close()
{
    debugf("CShop::close() called\n");
    mState = EShopState::ClosingFadeOut;
    gShopFade.fadeOut(0.3f);
}

void CShop::switchToShopResolution()
{
    debugf("Switching to 640x480 resolution\n");
    rspq_wait();
    display_close();
    display_init(RESOLUTION_640x480, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
    rdpq_init();
}

void CShop::switchToGameResolution()
{
    debugf("Switching to 256x240 resolution\n");
    rspq_wait();
    display_close();
    display_init(RESOLUTION_256x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
    rdpq_init();
}

bool CShop::update(float deltaTime, joypad_buttons_t pressed, joypad_buttons_t held)
{
    if (mState == EShopState::Closed) return false;

    mCursorTimer += deltaTime * 6.0f;

    gShopFade.update(deltaTime);
    
    switch (mState) {
        case EShopState::FadingOut:
            if (gShopFade.isFadedOut()) {
                switchToShopResolution();
                mState = EShopState::FadingIn;
                gShopFade.fadeIn(0.3f);
                updateItemPreview();
            }
            break;
            
        case EShopState::FadingIn:
            if (gShopFade.isFadedIn()) {
                mState = EShopState::Open;
                debugf("Shop is now fully open\n");
            }
            break;
            
        case EShopState::Open:
            mPreviewRotation += deltaTime * 0.5f;
            handleInput(pressed, held);
            break;
            
        case EShopState::ClosingFadeOut:
            if (gShopFade.isFadedOut()) {
                switchToGameResolution();
                mState = EShopState::ClosingFadeIn;
                gShopFade.fadeIn(0.3f);
                
                if (mPreviewModelLoaded) {
                    mPreviewModel.unload();
                    mPreviewModelLoaded = false;
                }
            }
            break;
            
        case EShopState::ClosingFadeIn:
            if (gShopFade.isFadedIn()) {
                mState = EShopState::Closed;
                mIsOpen = false;
                debugf("Shop is now fully closed\n");
            }
            break;
            
        default:
            break;
    }

    if (mTabSwitchTimer > 0.0f) {
        mTabSwitchTimer -= deltaTime;
    }

    return true;
}

void CShop::handleInput(joypad_buttons_t pressed, joypad_buttons_t held)
{
    if (pressed.b) {
        close();
        return;
    }

    if (pressed.l && mTabSwitchTimer <= 0.0f) {
        switchTab(-1);
        mTabSwitchTimer = TAB_SWITCH_DELAY;
    }
    if (pressed.r && mTabSwitchTimer <= 0.0f) {
        switchTab(1);
        mTabSwitchTimer = TAB_SWITCH_DELAY;
    }

    int itemCount = (mMode == EShopMode::Buying) 
        ? getTabItemCount(mCurrentTab) 
        : getInventoryItemCount(mCurrentTab);
    if (itemCount == 0) return;

    if (pressed.d_up) {
        mSelectedIndex--;
        if (mSelectedIndex < 0) mSelectedIndex = itemCount - 1;
        updateItemPreview();
    }
    if (pressed.d_down) {
        mSelectedIndex++;
        if (mSelectedIndex >= itemCount) mSelectedIndex = 0;
        updateItemPreview();
    }

    if (mSelectedIndex < mScrollOffset) {
        mScrollOffset = mSelectedIndex;
    }
    if (mSelectedIndex >= mScrollOffset + ITEMS_PER_PAGE) {
        mScrollOffset = mSelectedIndex - ITEMS_PER_PAGE + 1;
    }

    if (pressed.a) {
        if (mMode == EShopMode::Buying) {
            buySelectedItem();
        } else {
            sellSelectedItem();
        }
    }
}

void CShop::switchTab(int direction)
{
    int tabIndex = (int)mCurrentTab + direction;
    if (tabIndex < 0) tabIndex = (int)EShopTab::Count - 1;
    if (tabIndex >= (int)EShopTab::Count) tabIndex = 0;

    mCurrentTab = (EShopTab)tabIndex;
    mSelectedIndex = 0;
    mScrollOffset = 0;
    updateItemPreview();
}

void CShop::drawGradientBackground()
{
    rdpq_set_mode_fill(RGBA32(15, 25, 45, 255));
    rdpq_fill_rectangle(0, 0, 640, 480);
    
    rdpq_set_mode_fill(RGBA32(30, 50, 80, 255));
    rdpq_fill_rectangle(0, 0, 640, 200);
}

void CShop::drawBorder(int x, int y, int width, int height, color_t color)
{
    rdpq_set_mode_fill(RGBA32(color.r, color.g, color.b, color.a));
    rdpq_fill_rectangle(x, y, x + width, y + 4);
    rdpq_fill_rectangle(x, y + height - 4, x + width, y + height);
    rdpq_fill_rectangle(x, y, x + 4, y + height);
    rdpq_fill_rectangle(x + width - 4, y, x + width, y + height);
}

void CShop::drawPanel(int x, int y, int width, int height, color_t bgColor, color_t borderColor)
{
    rdpq_set_mode_fill(RGBA32(bgColor.r, bgColor.g, bgColor.b, bgColor.a));
    rdpq_fill_rectangle(x, y, x + width, y + height);
    
    rdpq_set_mode_fill(RGBA32(borderColor.r, borderColor.g, borderColor.b, borderColor.a));
    rdpq_fill_rectangle(x, y, x + width, y + 3);
    rdpq_fill_rectangle(x, y + height - 3, x + width, y + height);
}

EShopTab CShop::getItemTab(const SShopItem& item) const
{
    if (item.inventoryTab == EMenuTab::FishingRods) return EShopTab::Rods;
    if (item.inventoryTab == EMenuTab::Bait) return EShopTab::Bait;
    return EShopTab::Items;
}

int CShop::getTabItemCount(EShopTab tab) const
{
    int count = 0;
    for (int i = 0; i < mShopItemCount; i++) {
        if (getItemTab(mShopItems[i]) == tab) count++;
    }
    return count;
}

const SShopItem* CShop::findItemInTab(EShopTab tab, int index) const
{
    int currentIndex = 0;
    for (int i = 0; i < mShopItemCount; i++) {
        if (getItemTab(mShopItems[i]) == tab) {
            if (currentIndex == index) return &mShopItems[i];
            currentIndex++;
        }
    }
    return nullptr;
}

EMenuTab CShop::shopTabToMenuTab(EShopTab tab) const
{
    switch (tab) {
        case EShopTab::Rods:  return EMenuTab::FishingRods;
        case EShopTab::Bait:  return EMenuTab::Bait;
        case EShopTab::Items: return EMenuTab::MiscItems;
        default:              return EMenuTab::MiscItems;
    }
}

int CShop::getInventoryItemCount(EShopTab tab) const
{
    if (!mMenu) return 0;
    return mMenu->getItemCount(shopTabToMenuTab(tab));
}

const SMenuItem* CShop::getInventoryItem(EShopTab tab, int index) const
{
    if (!mMenu) return nullptr;
    return mMenu->getItem(shopTabToMenuTab(tab), index);
}

int CShop::getSellPrice(const SMenuItem* item) const
{
    if (!item) return 0;
    
    for (int i = 0; i < mShopItemCount; i++) {
        if (strcmp(mShopItems[i].name, item->name) == 0) {
            return (int)(mShopItems[i].price * SELL_PRICE_RATIO);
        }
    }
    
    if (mFishPool) {
        for (int i = 0; i < mFishPoolCount; i++) {
            if (strcmp(mFishPool[i].name, item->name) == 0) {
                return mFishPool[i].sellPrice;
            }
        }
    }
    
    return 10;
}

void CShop::buySelectedItem()
{
    if (!mMenu || !mPlayer) return;

    // Find the selected item in current tab
    const SShopItem* item = findItemInTab(mCurrentTab, mSelectedIndex);
    if (!item) return;

    const SPlayerStats& stats = mMenu->getPlayerStats();
    if (stats.currency >= item->price) {
        mMenu->addCurrency(-item->price);

        mMenu->addItem(item->inventoryTab, 
                       item->name, 
                       1, 
                       item->iconIndex,
                       item->modelPath);
        
        if (gSaveManager.isAvailable()) {
            gSaveManager.save(*mMenu);
        }
    }
}

void CShop::sellSelectedItem()
{
    if (!mMenu) return;
    
    const SMenuItem* item = getInventoryItem(mCurrentTab, mSelectedIndex);
    if (!item) return;
    
    int sellPrice = getSellPrice(item);
    mMenu->addCurrency(sellPrice);
    
    EMenuTab menuTab = shopTabToMenuTab(mCurrentTab);
    if (item->quantity > 1) {
        mMenu->updateItemQuantity(menuTab, mSelectedIndex, item->quantity - 1);
    } else {
        mMenu->removeItem(menuTab, mSelectedIndex);
        int newCount = getInventoryItemCount(mCurrentTab);
        if (mSelectedIndex >= newCount && newCount > 0) {
            mSelectedIndex = newCount - 1;
        }
    }
    
    if (gSaveManager.isAvailable()) {
        gSaveManager.save(*mMenu);
    }
    
    updateItemPreview();
}

void CShop::updateItemPreview()
{
    if (mPreviewModelLoaded) {
        mPreviewModel.unload();
        mPreviewModelLoaded = false;
    }

    if (mMode == EShopMode::Buying) {
        const SShopItem* item = findItemInTab(mCurrentTab, mSelectedIndex);
        if (item && item->modelPath != nullptr) {
            mPreviewModel.load(item->modelPath);
            mPreviewModelLoaded = true;
        }
    } else {
        const SMenuItem* item = getInventoryItem(mCurrentTab, mSelectedIndex);
        if (item && item->modelPath[0] != '\0') {
            mPreviewModel.load(item->modelPath);
            mPreviewModelLoaded = true;
        }
    }
}

void CShop::draw()
{
    if (mState == EShopState::Open || mState == EShopState::FadingIn || mState == EShopState::ClosingFadeOut) {
        debugf("CShop::draw() rendering shop UI\n");
        
        rdpq_sync_pipe();

        drawGradientBackground();
        
        drawBorder(10, 10, 620, 460, {120, 180, 220, 255});
        
        rdpq_set_mode_fill(RGBA32(80, 120, 160, 200));
        rdpq_fill_rectangle(20, 20, 620, 22);
        rdpq_fill_rectangle(20, 458, 620, 460);
        rdpq_fill_rectangle(20, 20, 22, 460);
        rdpq_fill_rectangle(618, 20, 620, 460);

        drawPanel(30, 30, 580, 50, {40, 70, 110, 230}, {150, 200, 240, 255});
        
        const char* modeText = (mMode == EShopMode::Buying) ? "FISH SHOP - BUYING" : "FISH SHOP - SELLING";
        rdpq_sync_pipe();
        int titleX = 220;
        rdpq_text_printf(nullptr, mFontId, titleX, 50, "%s", modeText);

        if (mMenu) {
            const SPlayerStats& stats = mMenu->getPlayerStats();
            
            rdpq_set_mode_fill(RGBA32(60, 90, 130, 220));
            rdpq_fill_rectangle(480, 40, 590, 70);
            rdpq_set_mode_fill(RGBA32(180, 200, 100, 255));
            rdpq_fill_rectangle(480, 40, 590, 43);
            
            rdpq_sync_pipe();
            rdpq_text_printf(nullptr, mFontId, 490, 52, "Gold: %d", stats.currency);
        }

        rdpq_set_mode_fill(RGBA32(30, 55, 90, 200));
        rdpq_fill_rectangle(30, 95, 610, 130);

        const char* tabNames[] = { "FISHING RODS", "BAIT", "ITEMS" };
        int tabSpacing = 193;
        for (int i = 0; i < (int)EShopTab::Count; i++) {
            int tabX = 30 + (i * tabSpacing);
            int tabY = 100;
            int tabW = 180;
            int tabH = 25;

            if ((int)mCurrentTab == i) {
                rdpq_set_mode_fill(RGBA32(100, 160, 220, 255));
                rdpq_fill_rectangle(tabX + 5, tabY, tabX + tabW, tabY + tabH);
                rdpq_set_mode_fill(RGBA32(180, 220, 255, 255));
                rdpq_fill_rectangle(tabX + 5, tabY, tabX + tabW, tabY + 3);
            } else {
                rdpq_set_mode_fill(RGBA32(50, 80, 120, 200));
                rdpq_fill_rectangle(tabX + 5, tabY, tabX + tabW, tabY + tabH);
            }
            
            rdpq_sync_pipe();
            int textX = tabX + 35;
            rdpq_text_printf(nullptr, mFontId, textX, tabY + 8, "%s", tabNames[i]);
        }

        drawPanel(30, 145, 350, 285, {25, 45, 75, 230}, {100, 150, 200, 255});

        int itemY = 160;
        int visibleIndex = 0;

        rdpq_sync_pipe();
        
        if (mMode == EShopMode::Buying) {
            int currentIndex = 0;
            for (int i = 0; i < mShopItemCount; i++) {
                EShopTab itemTab = getItemTab(mShopItems[i]);

                if (itemTab == mCurrentTab) {
                    if (currentIndex < mScrollOffset) {
                        currentIndex++;
                        continue;
                    }
                    if (visibleIndex >= ITEMS_PER_PAGE) break;

                    if (currentIndex == mSelectedIndex) {
                        rdpq_sync_pipe();
                        rdpq_set_mode_fill(RGBA32(70, 110, 160, 200));
                        rdpq_fill_rectangle(35, itemY - 3, 375, itemY + 17);
                        
                        if (mCursorSprite) {
                            float bobOffset = sinf(mCursorTimer) * 2.0f;
                            int cursorX = 38 + (int)bobOffset;
                            int cursorY = itemY + 0;
                            
                            rdpq_sync_pipe();
                            rdpq_set_mode_standard();
                            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
                            rdpq_sprite_blit(mCursorSprite, cursorX, cursorY, NULL);
                        }
                    }

                    rdpq_sync_pipe();
                    rdpq_text_printf(nullptr, mFontId, 55, itemY + 10, 
                                   "%-22s", mShopItems[i].name);
                    rdpq_text_printf(nullptr, mFontId, 300, itemY + 10, "%dG", mShopItems[i].price);
                    itemY += 20;
                    visibleIndex++;
                    currentIndex++;
                }
            }
        } else {
            int itemCount = getInventoryItemCount(mCurrentTab);
            for (int i = 0; i < itemCount; i++) {
                if (i < mScrollOffset) continue;
                if (visibleIndex >= ITEMS_PER_PAGE) break;

                const SMenuItem* item = getInventoryItem(mCurrentTab, i);
                if (!item) continue;

                if (i == mSelectedIndex) {
                    rdpq_sync_pipe();
                    rdpq_set_mode_fill(RGBA32(70, 110, 160, 200));
                    rdpq_fill_rectangle(35, itemY - 3, 375, itemY + 17);
                    
                    if (mCursorSprite) {
                        float bobOffset = sinf(mCursorTimer) * 2.0f;
                        int cursorX = 38 + (int)bobOffset;
                        int cursorY = itemY + 0;
                        
                        rdpq_sync_pipe();
                        rdpq_set_mode_standard();
                        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
                        rdpq_sprite_blit(mCursorSprite, cursorX, cursorY, NULL);
                    }
                }

                rdpq_sync_pipe();
                rdpq_text_printf(nullptr, mFontId, 55, itemY + 10, 
                               "%-18s x%d", item->name, item->quantity);
                int sellPrice = getSellPrice(item);
                rdpq_text_printf(nullptr, mFontId, 300, itemY + 10, "%dG", sellPrice);
                itemY += 20;
                visibleIndex++;
            }
        }

        drawPanel(395, 145, 215, 285, {25, 45, 75, 230}, {100, 150, 200, 255});

        if (mMode == EShopMode::Buying) {
            const SShopItem* selectedItem = findItemInTab(mCurrentTab, mSelectedIndex);
            if (selectedItem) {
                rdpq_sync_pipe();
                rdpq_text_printf(nullptr, mFontId, 405, 360, "Item:");
                rdpq_text_printf(nullptr, mFontId, 405, 375, "%s", selectedItem->name);
                rdpq_text_printf(nullptr, mFontId, 405, 395, "Info:");
                rdpq_text_printf(nullptr, mFontId, 405, 410, "%s", selectedItem->description);
            }
        } else {
            const SMenuItem* selectedItem = getInventoryItem(mCurrentTab, mSelectedIndex);
            if (selectedItem) {
                rdpq_sync_pipe();
                rdpq_text_printf(nullptr, mFontId, 405, 360, "Item:");
                rdpq_text_printf(nullptr, mFontId, 405, 375, "%s", selectedItem->name);
                rdpq_text_printf(nullptr, mFontId, 405, 395, "Owned: %d", selectedItem->quantity);
                int sellPrice = getSellPrice(selectedItem);
                rdpq_text_printf(nullptr, mFontId, 405, 410, "Sell for: %dG", sellPrice);
            }
        }

        if (mPreviewModelLoaded) {
            rdpq_sync_pipe();
            rdpq_set_mode_fill(RGBA32(40, 70, 110, 180));
            rdpq_fill_rectangle(400, 150, 605, 350);
            
            rdpq_set_mode_fill(RGBA32(120, 180, 220, 255));
            rdpq_fill_rectangle(400, 150, 605, 153);
            rdpq_fill_rectangle(400, 347, 605, 350);
            
            t3d_frame_start();
            
            rdpq_sync_pipe();
            rdpq_sync_tile();
            
            const int viewportSize = 200;
            const int viewportX = 402;
            const int viewportY = 153;
            
            T3DViewport* vp = mPreviewViewport.getViewport();
            vp->size[0] = viewportSize;
            vp->size[1] = viewportSize;
            vp->offset[0] = viewportX;
            vp->offset[1] = viewportY;
            
            float camDist = 15.0f;
            float camX = 0.0f;
            float camZ = camDist;
            float camY = 0.0f;
            
            mPreviewViewport.lookAt({camX, camY, camZ}, {0.0f, 0.0f, 0.0f});
            mPreviewViewport.attach();
            
            uint8_t lightColor[4] = {255, 255, 255, 255};
            t3d_light_set_ambient(lightColor);
            
            mPreviewModel.setPosition({0.0f, 0.0f, 0.0f});
            mPreviewModel.setRotation(TVec3F(0.0f, mPreviewRotation, 0.0f));
            mPreviewModel.updateMatrix();
            
            rdpq_sync_pipe();
            rdpq_set_mode_standard();
            rdpq_mode_alphacompare(1);
            
            mPreviewModel.draw();
            
            rdpq_sync_pipe();
            
            rdpq_set_scissor(0, 0, 640, 480);
        }

        drawPanel(30, 438, 580, 27, {40, 70, 110, 230}, {120, 180, 220, 255});
        
        rdpq_sync_pipe();
        const char* actionText = (mMode == EShopMode::Buying) ? "A: Buy" : "A: Sell";
        rdpq_text_printf(nullptr, mFontId, 150, 447, "L/R: Switch Tab   %s   B: Exit", actionText);
        
        rdpq_sync_pipe();
        rdpq_set_mode_standard();
    }
}

void CShop::drawFade()
{
    rdpq_sync_pipe();
    rdpq_sync_tile();
    rdpq_set_mode_standard();
    
    gShopFade.draw();
}

void CShop::addShopItem(const SShopItem& item)
{
    if (mShopItemCount >= SHOP_MAX_ITEMS) return;
    mShopItems[mShopItemCount++] = item;
}

void CShop::addShopItems(const SShopItem* items, int count)
{
    for (int i = 0; i < count; i++) {
        addShopItem(items[i]);
    }
}

void CShop::clearShopItems()
{
    mShopItemCount = 0;
}
