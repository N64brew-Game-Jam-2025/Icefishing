#include "menu.hpp"
#include "player.hpp"
#include "sound.hpp"
#include "save_manager.hpp"
#include <cstring>
#include <cmath>
#include <cstdio>

CMenu::~CMenu()
{
    freeSprite(mIconSprite);
    freeSprite(mCursorSprite);
    freeSprite(mCheckSprite);
    freeSprite(mButtonSprite);
}

void CMenu::init(int fontId)
{
    mFontId = fontId;
    mState = EMenuState::Closed;
    mCurrentTab = EMenuTab::Stats;
    mSelectedIndex = 0;
    mScrollOffset = 0;
    mAnimProgress = 0.0f;
    
    mCursorSprite = sprite_load("rom:/sflk.ia16.sprite");
    mCheckSprite = sprite_load("rom:/check.ci4.sprite");    mButtonSprite = sprite_load("rom:/btns.ci4.sprite");    
    mPlayerStats = {};
    mPlayerStats.level = 1;
    mPlayerStats.expToNextLevel = 100;
    
    for (int t = 0; t < MENU_TAB_COUNT; ++t) {
        mItemCounts[t] = 0;
        for (int i = 0; i < MENU_MAX_ITEMS; ++i) {
            mItems[t][i] = {};
        }
    }
}

void CMenu::loadIcons(const char* spritePath)
{
    freeSprite(mIconSprite);
    mIconSprite = sprite_load(spritePath);
}

void CMenu::toggle()
{
    if (mState == EMenuState::Closed) {
        CSoundMgr::play("menu_open");
        open();
    } else if (mState == EMenuState::Open) {
        CSoundMgr::play("menu_close");
        close();
    }
}

void CMenu::open()
{
    if (mState == EMenuState::Closed) {
        startOpenAnimation();
        mCurrentTab = EMenuTab::Stats;
        mSelectedIndex = 0;
        mScrollOffset = 0;
    }
}

void CMenu::close()
{
    if (mState == EMenuState::Open || mState == EMenuState::Opening) {
        startCloseAnimation();
    }
}

void CMenu::startOpenAnimation()
{
    mState = EMenuState::Opening;
    mAnimProgress = 0.0f;
}

void CMenu::startCloseAnimation()
{
    mState = EMenuState::Closing;
    mAnimProgress = 0.0f;
}

float CMenu::easeOutBack(float t)
{
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    float tm1 = t - 1.0f;
    return 1.0f + c3 * tm1 * tm1 * tm1 + c1 * tm1 * tm1;
}

float CMenu::easeInBack(float t)
{
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return c3 * t * t * t - c1 * t * t;
}

float CMenu::easeOutQuad(float t)
{
    return 1.0f - (1.0f - t) * (1.0f - t);
}

color_t CMenu::calculateGradientColor(color_t top, color_t bottom, float t, float alpha)
{
    uint8_t r = (uint8_t)(top.r + (bottom.r - top.r) * t);
    uint8_t g = (uint8_t)(top.g + (bottom.g - top.g) * t);
    uint8_t b = (uint8_t)(top.b + (bottom.b - top.b) * t);
    uint8_t a = (uint8_t)((top.a + (bottom.a - top.a) * t) * alpha);
    return {r, g, b, a};
}

color_t CMenu::applyAlpha(color_t color, float alpha)
{
    return {color.r, color.g, color.b, (uint8_t)(color.a * alpha)};
}

void CMenu::drawBorder(int x, int y, int width, int height, int borderWidth, color_t color)
{
    rdpq_set_prim_color(color);
    rdpq_fill_rectangle(x, y, x + width, y + borderWidth);
    rdpq_fill_rectangle(x, y + height - borderWidth, x + width, y + height);
    rdpq_fill_rectangle(x, y, x + borderWidth, y + height);
    rdpq_fill_rectangle(x + width - borderWidth, y, x + width, y + height);
}

void CMenu::setStandardRenderMode()
{
    rdpq_set_mode_standard();
    rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
}

void CMenu::setAlphaBlitMode()
{
    rdpq_set_mode_standard();
    rdpq_mode_alphacompare(1);
}

void CMenu::freeSprite(sprite_t*& sprite)
{
    if (sprite) {
        sprite_free(sprite);
        sprite = nullptr;
    }
}

bool CMenu::validateTabIndex(int tabIndex) const
{
    return tabIndex >= 0 && tabIndex < MENU_TAB_COUNT;
}

void CMenu::drawScrollArrow(bool isUp, int x, int y, color_t color)
{
    setStandardRenderMode();
    rdpq_set_prim_color(color);
    
    if (isUp) {
        rdpq_fill_rectangle(x - 4, y, x + 4, y + 2);
        rdpq_fill_rectangle(x - 2, y - 2, x + 2, y);
    } else {
        rdpq_fill_rectangle(x - 4, y, x + 4, y + 2);
        rdpq_fill_rectangle(x - 2, y + 2, x + 2, y + 4);
    }
}

void CMenu::unequipAllInTab(EMenuTab tab)
{
    int tabIndex = static_cast<int>(tab);
    if (!validateTabIndex(tabIndex)) return;
    
    for (int i = 0; i < mItemCounts[tabIndex]; ++i) {
        mItems[tabIndex][i].equipped = false;
    }
}

int CMenu::getEquippedItemIndex(EMenuTab tab) const
{
    int tabIndex = static_cast<int>(tab);
    if (!validateTabIndex(tabIndex)) return -1;
    
    for (int i = 0; i < mItemCounts[tabIndex]; ++i) {
        if (mItems[tabIndex][i].equipped) return i;
    }
    return -1;
}

void CMenu::blitSpriteSlice(sprite_t* sprite, int x, int y, int col, int row, int size)
{
    rdpq_blitparms_t params = {};
    params.s0 = col * size;
    params.t0 = row * size;
    params.width = size;
    params.height = size;
    rdpq_sprite_blit(sprite, x, y, &params);
}

void CMenu::drawSnowflake(int cx, int cy, color_t color)
{
    rdpq_set_prim_color(color);
    rdpq_fill_rectangle(cx + 2, cy, cx + 4, cy + 6);
    rdpq_fill_rectangle(cx, cy + 2, cx + 6, cy + 4);
    rdpq_fill_rectangle(cx + 1, cy + 1, cx + 2, cy + 2);
    rdpq_fill_rectangle(cx + 4, cy + 1, cx + 5, cy + 2);
    rdpq_fill_rectangle(cx + 1, cy + 4, cx + 2, cy + 5);
    rdpq_fill_rectangle(cx + 4, cy + 4, cx + 5, cy + 5);
}

bool CMenu::update(float deltaTime, joypad_buttons_t pressed, joypad_buttons_t held)
{
    mSnowflakeTimer += deltaTime;
    mShimmerTimer += deltaTime * 2.0f;
    mCursorTimer += deltaTime * 6.0f;
    
    if (mTabTransitioning) {
        mTabTransitionProgress += deltaTime / mTabTransitionDuration;
        if (mTabTransitionProgress >= 1.0f) {
            mTabTransitionProgress = 1.0f;
            mTabTransitioning = false;
        }
    }
    
    switch (mState) {
        case EMenuState::Closed:
            return false;
            
        case EMenuState::Opening:
            mAnimProgress += deltaTime / mAnimDuration;
            if (mAnimProgress >= 1.0f) {
                mAnimProgress = 1.0f;
                mState = EMenuState::Open;
            }
            return true;
            
        case EMenuState::Closing:
            mAnimProgress += deltaTime / mAnimDuration;
            if (mAnimProgress >= 1.0f) {
                mAnimProgress = 1.0f;
                mState = EMenuState::Closed;

                if (gSaveManager.isAvailable()) {
                    gSaveManager.save(*this);
                }
                return false;
            }
            return true;
            
        case EMenuState::Open:
            break;
    }
    
    if (mState == EMenuState::Open) {
        if (pressed.l) {
            navigateTabs(-1);
        }
        if (pressed.r) {
            navigateTabs(1);
        }
        
        if (mCurrentTab != EMenuTab::Stats) {
            if (pressed.d_up) {
                navigateItems(-1);
            }
            if (pressed.d_down) {
                navigateItems(1);
            }
            
            if (pressed.a) {
                CSoundMgr::play("equip");
                handleSelection();
            }
        }
        
        if (pressed.b || pressed.start) {
            close();
        }
    }
    
    return mState != EMenuState::Closed;
}

void CMenu::navigateTabs(int direction)
{
    int tabIndex = static_cast<int>(mCurrentTab);
    tabIndex += direction;
    
    if (tabIndex < 0) {
        tabIndex = MENU_TAB_COUNT - 1;
    } else if (tabIndex >= MENU_TAB_COUNT) {
        tabIndex = 0;
    }
    
    mPreviousTab = mCurrentTab;
    mCurrentTab = static_cast<EMenuTab>(tabIndex);
    mTabTransitioning = true;
    mTabTransitionProgress = 0.0f;
    mTabTransitionDirection = direction;
    
    mSelectedIndex = 0;
    mScrollOffset = 0;
}

void CMenu::navigateItems(int direction)
{
    int tabIndex = static_cast<int>(mCurrentTab);
    int itemCount = mItemCounts[tabIndex];
    
    if (itemCount == 0) return;
    
    mSelectedIndex += direction;
    
    if (mSelectedIndex < 0) {
        mSelectedIndex = 0;
    } else if (mSelectedIndex >= itemCount) {
        mSelectedIndex = itemCount - 1;
    }
    
    if (mSelectedIndex < mScrollOffset) {
        mScrollOffset = mSelectedIndex;
    } else if (mSelectedIndex >= mScrollOffset + mVisibleItems) {
        mScrollOffset = mSelectedIndex - mVisibleItems + 1;
    }
}

void CMenu::handleSelection()
{
    if (mCurrentTab == EMenuTab::FishingRods) {
        equipFishingRod(mSelectedIndex);
    }
    else if (mCurrentTab == EMenuTab::Bait) {
        equipBait(mSelectedIndex);
    }
}

void CMenu::draw()
{
    if (mState == EMenuState::Closed) return;
    
    float animT = 0.0f;
    if (mState == EMenuState::Opening) {
        animT = easeOutBack(mAnimProgress);
    } else if (mState == EMenuState::Closing) {
        animT = 1.0f - easeInBack(mAnimProgress);
    } else {
        animT = 1.0f;
    }
    
    int centerX = mMenuX + mMenuWidth / 2;
    int centerY = mMenuY + mMenuHeight / 2;
    int drawW = (int)(mMenuWidth * animT);
    int drawH = (int)(mMenuHeight * animT);
    int drawX = centerX - drawW / 2;
    int drawY = centerY - drawH / 2;
    
    if (drawW < 8 || drawH < 8) return;
    
    rdpq_mode_push();
    rdpq_set_mode_standard();
    rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    
    int gradientSteps = 12;
    float stripHeight = (float)drawH / gradientSteps;
    float alphaT = animT;
    if (alphaT < 0.0f) alphaT = 0.0f;
    if (alphaT > 1.0f) alphaT = 1.0f;
    
    for (int i = 0; i < gradientSteps; ++i) {
        float t = (float)i / (gradientSteps - 1);
        color_t gradColor = calculateGradientColor(mBgColorTop, mBgColorBottom, t, alphaT);
        rdpq_set_prim_color(gradColor);
        
        int y1 = drawY + (int)(i * stripHeight);
        int y2 = drawY + (int)((i + 1) * stripHeight);
        if (i == gradientSteps - 1) y2 = drawY + drawH;
        
        rdpq_fill_rectangle(drawX, y1, drawX + drawW, y2);
    }
    
    int borderWidth = 3;
    drawBorder(drawX, drawY, drawW, drawH, borderWidth, applyAlpha(mBorderColor, alphaT));
    
    rdpq_set_prim_color(applyAlpha({0xFF, 0xFF, 0xFF, 0x60}, animT));
    rdpq_fill_rectangle(drawX + borderWidth, drawY + borderWidth, 
                        drawX + drawW - borderWidth, drawY + borderWidth + 1);
    

    if (animT > 0.7f) {
        float contentAlpha = (animT - 0.7f) / 0.3f;
        if (contentAlpha > 1.0f) contentAlpha = 1.0f;
        
        int tabWidth = (drawW / MENU_TAB_COUNT) - 1;
        int tabY = drawY + borderWidth + 2;
        
        for (int i = 0; i < MENU_TAB_COUNT; ++i) {
            int tabX = (drawX + 1) + i * tabWidth;
            
            bool isActive = (i == static_cast<int>(mCurrentTab));
            color_t tabColor = applyAlpha(isActive ? mTabActiveColor : mTabInactiveColor, contentAlpha);
            
            rdpq_set_prim_color(tabColor);
            rdpq_fill_rectangle(tabX + 2, tabY, tabX + tabWidth - 2, tabY + mTabHeight);
            
            if (isActive) {
                float shimmer = sinf(mShimmerTimer * 3.0f) * 0.3f + 0.7f;
                uint8_t highlightAlpha = (uint8_t)(0x40 * contentAlpha * shimmer);
                rdpq_set_prim_color((color_t){0xFF, 0xFF, 0xFF, highlightAlpha});
                rdpq_fill_rectangle(tabX + 4, tabY + 2, tabX + tabWidth - 4, tabY + 4);
            }
        }
        
        rdpq_sync_pipe();
        for (int i = 0; i < MENU_TAB_COUNT; ++i) {
            int tabX = drawX + i * tabWidth + tabWidth / 2 - 12;
            int textY = tabY + 6;
            rdpq_text_printf(NULL, mFontId, tabX, textY, "%s", mTabNames[i]);
        }
        
        if (mButtonSprite) {
            int hintY = tabY + 2;
            setAlphaBlitMode();
            
            blitSpriteSlice(mButtonSprite, drawX - 6, hintY - 8, 0, 1, 12);
            
            blitSpriteSlice(mButtonSprite, drawX + drawW - 8, hintY - 8, 2, 1, 12);
        }
        
        int contentY = tabY + mTabHeight + 4;
        int contentHeight = drawH - (contentY - drawY) - borderWidth - 4;
        
        setStandardRenderMode();
        rdpq_set_prim_color(applyAlpha({0x00, 0x10, 0x30, 0x40}, contentAlpha));
        rdpq_fill_rectangle(drawX + borderWidth + 2, contentY, 
                           drawX + drawW - borderWidth - 2, contentY + contentHeight);
        
        rdpq_sync_pipe();
        
        bool shouldDrawContent = true;
        if (mTabTransitioning) {
            float transitionT = easeOutQuad(mTabTransitionProgress);
            
            shouldDrawContent = transitionT >= 0.5f;
            
            int numBars = 8;
            int barWidth = (drawW - borderWidth * 2 - 4) / numBars;
            
            rdpq_set_mode_standard();
            rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
            
            for (int i = 0; i < numBars; ++i) {
                float barDelay = (float)i / numBars * 0.3f;
                float barProgress = (transitionT - barDelay) / (1.0f - barDelay);
                if (barProgress < 0.0f) barProgress = 0.0f;
                if (barProgress > 1.0f) barProgress = 1.0f;
                
                int barX;
                if (mTabTransitionDirection > 0) {
                    barX = drawX + borderWidth + 2 + i * barWidth;
                } else {
                    barX = drawX + borderWidth + 2 + (numBars - 1 - i) * barWidth;
                }
                
                int barHeight = (int)(contentHeight * barProgress);
                
                uint8_t barAlpha = (uint8_t)((1.0f - barProgress) * 180.0f);
                rdpq_set_prim_color((color_t){0xAA, 0xDD, 0xFF, barAlpha});
                rdpq_fill_rectangle(barX, contentY, barX + barWidth, contentY + barHeight);
            }
            
            rdpq_sync_pipe();
        }
        
        if (shouldDrawContent) {
            switch (mCurrentTab) {
                case EMenuTab::Stats:
                    drawStatsTab();
                    break;
                case EMenuTab::FishingRods:
                case EMenuTab::Bait:
                case EMenuTab::MiscItems:
                    drawInventoryTab(mCurrentTab);
                    break;
                default:
                    break;
            }
        }
        
        drawSnowflakeDecor();
    }
    
    rdpq_mode_pop();
}

void CMenu::drawStatsTab()
{
    int contentX = mMenuX + mPadding + 4;
    int contentY = mMenuY + 30 + mTabHeight;
    int lineHeight = 14;
    
    char timeBuffer[32];
    formatPlayTime(timeBuffer, sizeof(timeBuffer), mPlayerStats.playTimeSeconds);
    
    rdpq_text_printf(NULL, mFontId, contentX, contentY, "-- Newbie --");
    contentY += lineHeight + 4;
    
    rdpq_text_printf(NULL, mFontId, contentX, contentY, "Level: %d", mPlayerStats.level);
    contentY += lineHeight;
    
    rdpq_text_printf(NULL, mFontId, contentX, contentY, "EXP: %d / %d", 
                    mPlayerStats.currentExp, mPlayerStats.expToNextLevel);
    contentY += lineHeight;
    
    int barX = contentX;
    int barY = contentY;
    int barWidth = mMenuWidth - mPadding * 2 - 8;
    int barHeight = 8;
    
    setStandardRenderMode();
    
    rdpq_set_prim_color((color_t){0x20, 0x30, 0x50, 0xFF});
    rdpq_fill_rectangle(barX, barY, barX + barWidth, barY + barHeight);
    
    float expPercent = (float)mPlayerStats.currentExp / (float)mPlayerStats.expToNextLevel;
    if (expPercent > 1.0f) expPercent = 1.0f;
    int fillWidth = (int)(barWidth * expPercent);
    
    rdpq_set_prim_color((color_t){0x40, 0x90, 0xE0, 0xFF});
    rdpq_fill_rectangle(barX, barY, barX + fillWidth, barY + barHeight);
    
    float shimmer = sinf(mShimmerTimer * 4.0f) * 0.5f + 0.5f;
    rdpq_set_prim_color((color_t){0xFF, 0xFF, 0xFF, (uint8_t)(0x40 * shimmer)});
    rdpq_fill_rectangle(barX, barY, barX + fillWidth, barY + 2);
    
    drawBorder(barX, barY, barWidth, barHeight, 1, mBorderColor);
    
    contentY += barHeight + lineHeight;
    
    rdpq_sync_pipe();
    rdpq_text_printf(NULL, mFontId, contentX, contentY, "~~~~~~~~~~~~~~~~~~~~~~~~");
    contentY += lineHeight;
    
    rdpq_text_printf(NULL, mFontId, contentX, contentY, "Fish Caught: %d", mPlayerStats.totalFishCaught);
    contentY += lineHeight;
    
    rdpq_text_printf(NULL, mFontId, contentX, contentY, "Fish Types: %d / %d", 
                    mPlayerStats.uniqueFishCaught, mPlayerStats.totalFishSpecies);
    contentY += lineHeight;
    
    rdpq_text_printf(NULL, mFontId, contentX, contentY, "Gold: %d G", mPlayerStats.currency);
    contentY += lineHeight;
    
    rdpq_text_printf(NULL, mFontId, contentX, contentY, "Play Time: %s", timeBuffer);
    contentY += lineHeight + 4;
    
    rdpq_text_printf(NULL, mFontId, contentX, contentY, "~~~~~~~~~~~~~~~~~~~~~~~~");
}

void CMenu::drawInventoryTab(EMenuTab tab)
{
    int tabIndex = static_cast<int>(tab);
    int itemCount = mItemCounts[tabIndex];
    
    int contentX = mMenuX + mPadding + 4;
    int contentY = mMenuY + 30 + mTabHeight;
    int lineHeight = mItemHeight;
    
    if (itemCount == 0) {
        rdpq_text_printf(NULL, mFontId, contentX, contentY + 20, "No items yet...");
        return;
    }
    
    for (int i = 0; i < mVisibleItems && (mScrollOffset + i) < itemCount; ++i) {
        int itemIndex = mScrollOffset + i;
        const SMenuItem& item = mItems[tabIndex][itemIndex];
        
        int itemY = contentY + i * lineHeight;
        
        if (itemIndex == mSelectedIndex && mState == EMenuState::Open) {
            rdpq_set_mode_standard();
            rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
            
            float pulse = sinf(mShimmerTimer * 5.0f) * 0.2f + 0.8f;
            uint8_t highlightAlpha = (uint8_t)(mHighlightColor.a * pulse);
            rdpq_set_prim_color((color_t){mHighlightColor.r, mHighlightColor.g, 
                                          mHighlightColor.b, highlightAlpha});
            rdpq_fill_rectangle(contentX + 10, itemY - 10, 
                               contentX + mMenuWidth - mPadding * 2 - 8, itemY + lineHeight - 10);
            
            if (mCursorSprite) {
                float bobOffset = sinf(mCursorTimer) * 2.0f;
                int cursorX = contentX + (int)bobOffset;
                int cursorY = itemY - 6;
                
                rdpq_set_mode_standard();
                rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
                rdpq_sprite_blit(mCursorSprite, cursorX - 7, cursorY - 4, NULL);
            }
        }
        
        rdpq_sync_pipe();
        
        int textX = contentX + 12;
        if (mIconSprite && item.iconIndex >= 0) {
            int iconsPerRow = mIconSprite->width / mIconSize;
            int iconCol = item.iconIndex % iconsPerRow;
            int iconRow = item.iconIndex / iconsPerRow;
            
            setAlphaBlitMode();
            blitSpriteSlice(mIconSprite, textX, itemY - 2, iconCol, iconRow, mIconSize);
            textX += mIconSize + 4;
        }
        
        rdpq_text_printf(NULL, mFontId, textX, itemY, "%s", item.name);
        
        if ((tab == EMenuTab::FishingRods || tab == EMenuTab::Bait) && item.equipped) {
            if (mCheckSprite) {
                setAlphaBlitMode();
                rdpq_sprite_blit(mCheckSprite, contentX + mMenuWidth - mPadding * 2 - 50, itemY - 11, NULL);
            }
        } else if (item.quantity > 1) {
            rdpq_text_printf(NULL, mFontId, contentX + mMenuWidth - mPadding * 2 - 40, itemY, "x%d", item.quantity);
        }
    }
    
    int arrowX = mMenuX + mMenuWidth / 2;
    if (mScrollOffset > 0) {
        int arrowY = contentY - 6;
        drawScrollArrow(true, arrowX, arrowY, mAccentColor);
    }
    
    if (mScrollOffset + mVisibleItems < itemCount) {
        int arrowY = contentY + mVisibleItems * lineHeight + 2;
        drawScrollArrow(false, arrowX, arrowY, mAccentColor);
    }
}

void CMenu::drawSnowflakeDecor()
{
    setStandardRenderMode();
    
    float time = mSnowflakeTimer;
    
    int corners[4][2] = {
        {mMenuX + 8, mMenuY + 8},
        {mMenuX + mMenuWidth - 16, mMenuY + 8},
        {mMenuX + 8, mMenuY + mMenuHeight - 16},
        {mMenuX + mMenuWidth - 16, mMenuY + mMenuHeight - 16}
    };
    
    for (int c = 0; c < 4; ++c) {
        float phase = time + c * 1.5f;
        float alpha = (sinf(phase * 2.0f) * 0.3f + 0.7f);
        
        int cx = corners[c][0];
        int cy = corners[c][1];
        
        drawSnowflake(cx, cy, applyAlpha(mSnowColor, alpha));
    }
}

void CMenu::formatPlayTime(char* buffer, int bufferSize, float seconds)
{
    int totalSeconds = (int)seconds;
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int secs = totalSeconds % 60;
    
    snprintf(buffer, bufferSize, "%02d:%02d:%02d", hours, minutes, secs);
}

bool CMenu::addItem(EMenuTab tab, const char* name, int quantity, int iconIndex, const char* modelPath)
{
    int tabIndex = static_cast<int>(tab);
    if (!validateTabIndex(tabIndex)) return false;
    if (mItemCounts[tabIndex] >= MENU_MAX_ITEMS) return false;
    
    int existingIndex = findItem(tab, name);
    if (existingIndex >= 0) {
        mItems[tabIndex][existingIndex].quantity += quantity;
        return true;
    }
    
    int idx = mItemCounts[tabIndex];
    strncpy(mItems[tabIndex][idx].name, name, MENU_ITEM_NAME_LEN - 1);
    mItems[tabIndex][idx].name[MENU_ITEM_NAME_LEN - 1] = '\0';
    
    if (modelPath && modelPath[0] != '\0') {
        strncpy(mItems[tabIndex][idx].modelPath, modelPath, MENU_ITEM_MODEL_PATH_LEN - 1);
        mItems[tabIndex][idx].modelPath[MENU_ITEM_MODEL_PATH_LEN - 1] = '\0';
    } else {
        mItems[tabIndex][idx].modelPath[0] = '\0';
    }
    
    mItems[tabIndex][idx].quantity = quantity;
    mItems[tabIndex][idx].iconIndex = iconIndex;
    mItems[tabIndex][idx].equipped = false;
    
    mItemCounts[tabIndex]++;
    return true;
}

bool CMenu::removeItem(EMenuTab tab, int index)
{
    int tabIndex = static_cast<int>(tab);
    if (!validateTabIndex(tabIndex)) return false;
    if (index < 0 || index >= mItemCounts[tabIndex]) return false;
    
    for (int i = index; i < mItemCounts[tabIndex] - 1; ++i) {
        mItems[tabIndex][i] = mItems[tabIndex][i + 1];
    }
    mItemCounts[tabIndex]--;
    
    if (mSelectedIndex >= mItemCounts[tabIndex] && mItemCounts[tabIndex] > 0) {
        mSelectedIndex = mItemCounts[tabIndex] - 1;
    }
    
    return true;
}

bool CMenu::updateItemQuantity(EMenuTab tab, int index, int newQuantity)
{
    int tabIndex = static_cast<int>(tab);
    if (tabIndex < 0 || tabIndex >= MENU_TAB_COUNT) return false;
    if (index < 0 || index >= mItemCounts[tabIndex]) return false;
    
    if (newQuantity <= 0) {
        return removeItem(tab, index);
    }
    
    mItems[tabIndex][index].quantity = newQuantity;
    return true;
}

int CMenu::findItem(EMenuTab tab, const char* name)
{
    int tabIndex = static_cast<int>(tab);
    if (!validateTabIndex(tabIndex)) return -1;
    
    for (int i = 0; i < mItemCounts[tabIndex]; ++i) {
        if (strcmp(mItems[tabIndex][i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

const SMenuItem* CMenu::getItem(EMenuTab tab, int index) const
{
    int tabIndex = static_cast<int>(tab);
    if (!validateTabIndex(tabIndex)) return nullptr;
    if (index < 0 || index >= mItemCounts[tabIndex]) return nullptr;
    
    return &mItems[tabIndex][index];
}

int CMenu::getItemCount(EMenuTab tab) const
{
    int tabIndex = static_cast<int>(tab);
    if (!validateTabIndex(tabIndex)) return 0;
    
    return mItemCounts[tabIndex];
}

void CMenu::equipFishingRod(int index)
{
    int tabIndex = static_cast<int>(EMenuTab::FishingRods);
    if (index < 0 || index >= mItemCounts[tabIndex]) return;
    
    unequipAllInTab(EMenuTab::FishingRods);
    
    mItems[tabIndex][index].equipped = true;
    
    if (mPlayer && mItems[tabIndex][index].modelPath[0] != '\0') {
        mPlayer->equipFishingRod(mItems[tabIndex][index].modelPath);
    }
}

void CMenu::equipBait(int index)
{
    int tabIndex = static_cast<int>(EMenuTab::Bait);
    if (index < 0 || index >= mItemCounts[tabIndex]) return;
    
    unequipAllInTab(EMenuTab::Bait);
    
    mItems[tabIndex][index].equipped = true;
}

int CMenu::getEquippedRodIndex() const
{
    return getEquippedItemIndex(EMenuTab::FishingRods);
}

int CMenu::getEquippedBaitIndex() const
{
    return getEquippedItemIndex(EMenuTab::Bait);
}

const char* CMenu::getEquippedRodModelPath() const
{
    int tabIndex = static_cast<int>(EMenuTab::FishingRods);
    for (int i = 0; i < mItemCounts[tabIndex]; ++i) {
        if (mItems[tabIndex][i].equipped) {
            if (mItems[tabIndex][i].modelPath[0] != '\0') {
                return mItems[tabIndex][i].modelPath;
            }
            return nullptr;
        }
    }
    return nullptr;
}
