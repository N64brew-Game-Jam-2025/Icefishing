#include "player_state.hpp"
#include "player.hpp"
#include "menu.hpp"
#include "sound.hpp"
#include "camera.hpp"
#include "collision.hpp"
#include "util.hpp"
#include "save_manager.hpp"
#include <libdragon.h>
#include <t3d/t3dmath.h>
#include <cmath>

std::string CPlayerState::checkCommonTransitions(CPlayer* player)
{
    CMenu* menu = player->getMenu();
    if (menu && menu->isOpen()) {
        return "";
    }
    
    joypad_buttons_t pressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    
    if (pressed.a && player->hasRodEquipped()) {
        if (menu) {
            int baitIndex = menu->getEquippedBaitIndex();
            if (baitIndex >= 0) {
                const SMenuItem* bait = menu->getItem(EMenuTab::Bait, baitIndex);
                if (bait && bait->quantity > 0) {
                    return "prep";
                }
            }
        }
    }
    
    return "";
}

void CPlayerStateMachine::init(CPlayer* player, const std::string& startState)
{
    if (mCurrentState) {
        mCurrentState->exit(player);
    }
    mCurrentState = getState(startState);
    if (mCurrentState) {
        mCurrentState->init(player);
    }
}

void CPlayerStateMachine::update(CPlayer* player, float dt)
{
    if (!mCurrentState) return;
    
    std::string nextState = mCurrentState->update(player, dt);
    
    if (!nextState.empty() && nextState != mCurrentState->getName()) {
        transitionTo(player, nextState);
    }
}

void CPlayerStateMachine::transitionTo(CPlayer* player, const std::string& stateName)
{
    CPlayerState* newState = getState(stateName);
    if (!newState || newState == mCurrentState) return;
    
    if (mCurrentState) {
        mCurrentState->exit(player);
    }
    
    mCurrentState = newState;
    mCurrentState->init(player);
}

const char* CPlayerStateMachine::getCurrentStateName() const
{
    return mCurrentState ? mCurrentState->getName() : "none";
}

bool CPlayerStateMachine::isInState(const std::string& name) const
{
    return mCurrentState && mCurrentState->getName() == name;
}

CPlayerState* CPlayerStateMachine::getState(const std::string& name)
{
    if (name == "idle") return &mIdleState;
    if (name == "walk") return &mWalkState;
    if (name == "run") return &mRunState;
    if (name == "prep") return &mPrepState;
    if (name == "throw") return &mThrowState;
    if (name == "hold") return &mHoldState;
    if (name == "reel") return &mReelState;
    if (name == "item_get") return &mItemGetState;
    return nullptr;
}

void CPlayerIdleState::init(CPlayer* player)
{
    player->getAnimController().forceBlendFactor(0.0f);
}

std::string CPlayerIdleState::update(CPlayer* player, float dt)
{
    std::string transition = checkCommonTransitions(player);
    if (!transition.empty()) return transition;
    
    if (player->getSpeed() > 0.1f) {
        return "walk";
    }
    
    float currentBlend = player->getAnimController().getBlendFactor();
    if (currentBlend > 0.0f) {
        player->getAnimController().setBlendFactor(currentBlend);
    }
    
    return "";
}

void CPlayerIdleState::exit(CPlayer* player)
{
}

void CPlayerWalkState::init(CPlayer* player)
{
}

std::string CPlayerWalkState::update(CPlayer* player, float dt)
{
    std::string transition = checkCommonTransitions(player);
    if (!transition.empty()) return transition;
    
    joypad_buttons_t pressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    
    if (player->getSpeed() < 0.05f) {
        return "idle";
    }

    if (pressed.l) {
        return "run";
    }
    
    float blend = player->getSpeed() / 0.51f;
    if (blend > 1.0f) blend = 1.0f;
    
    player->getAnimController().setBlendFactor(blend);
    player->getAnimController().setMovementSpeed(blend + 0.8f);
    
    float currentFrame = player->getModel().getAnimationFrame("walk");
    float prevFrame = player->getPrevWalkFrame();
    
    player->playFootstepSound(prevFrame, currentFrame, 2.0f, 40.0f);
    player->setPrevWalkFrame(currentFrame);
    
    return "";
}

void CPlayerWalkState::exit(CPlayer* player)
{
}

void CPlayerRunState::init(CPlayer* player)
{
    player->getAnimController().playAction("run");
    player->getAnimController().holdAction(true);
}

std::string CPlayerRunState::update(CPlayer* player, float dt)
{
    std::string transition = checkCommonTransitions(player);
    if (!transition.empty()) return transition;
    
    if (player->getSpeed() < 0.05f) {
        return "idle";
    }
    
    if (player->getSpeed() < 0.35f) {
        return "walk";
    }
    
    player->setSpeedMultiplier(3.0f);
    
    float currentFrame = player->getModel().getAnimationFrame("run");
    float prevFrame = player->getPrevRunFrame();
    
    player->playFootstepSound(prevFrame, currentFrame, 5.0f, 23.0f);
    
    player->setPrevRunFrame(currentFrame);
    
    return "";
}

void CPlayerRunState::exit(CPlayer* player)
{
    player->setSpeedMultiplier(1.0f);
    player->getAnimController().stopAction();
}

void CPlayerPrepState::init(CPlayer* player)
{
    player->getAnimController().playAction("prep");
    player->getAnimController().holdAction(true);
    player->setThrowDistance(15.0f);
    player->updateThrowTarget();
}

std::string CPlayerPrepState::update(CPlayer* player, float dt)
{
    joypad_inputs_t joypad = joypad_get_inputs(JOYPAD_PORT_1);
    
    float stickX = (float)joypad.stick_x / 80.0f;
    float stickY = (float)joypad.stick_y / 80.0f;
    
    if (fabsf(stickX) < 0.15f) stickX = 0.0f;
    if (fabsf(stickY) < 0.15f) stickY = 0.0f;
    
    if (fabsf(stickX) > 0.0f) {
        float rotSpeed = 2.5f * dt;
        float newRotY = player->getRotY() - stickX * rotSpeed;
        player->setRotY(newRotY);
    }
    
    if (fabsf(stickY) > 0.0f) {
        float distSpeed = 35.0f * dt;
        float currentDist = player->getThrowDistance();
        float newDist = currentDist + stickY * distSpeed;
        
        constexpr float MIN_THROW_DIST = 5.0f;
        constexpr float MAX_THROW_DIST = 65.0f;
        if (newDist < MIN_THROW_DIST) newDist = MIN_THROW_DIST;
        if (newDist > MAX_THROW_DIST) newDist = MAX_THROW_DIST;
        
        player->setThrowDistance(newDist);
    }
    
    player->updateThrowTarget();
    
    joypad_buttons_t pressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    
    if (pressed.b) {
        //return "idle";
    }
    
    if (pressed.a) {
        CMenu* menu = player->getMenu();
        if (menu) {
            int baitIndex = menu->getEquippedBaitIndex();
            if (baitIndex >= 0) {
                const SMenuItem* bait = menu->getItem(EMenuTab::Bait, baitIndex);
                if (bait && bait->quantity > 0) {
                    return "throw";
                }
            }
        }
    }
    
    return "";
}

void CPlayerPrepState::exit(CPlayer* player)
{
    player->getAnimController().holdAction(false);
}

void CPlayerThrowState::init(CPlayer* player)
{
    player->getAnimController().playAction("throw");
    player->getAnimController().holdAction(true);
    player->startBobberThrow();
}

std::string CPlayerThrowState::update(CPlayer* player, float dt)
{
    player->updateBobber(dt);
    
    joypad_buttons_t pressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    
    if (pressed.b) {
        return "idle";
    }
    
    if (player->hasBobberLanded()) {
        const ColFloorResult& floorResult = player->getThrowFloorResult();
        if (!(floorResult.flags & COL_FLAG_WATER)) {
            player->getAnimController().forceBlendFactor(0.0f);
            return "idle";
        }
    }
    
    if (player->getAnimController().isActionFinished()) {
        return "hold";
    }
    
    return "";
}

void CPlayerThrowState::exit(CPlayer* player)
{
    player->getAnimController().holdAction(false);
}

void CPlayerHoldState::init(CPlayer* player)
{
    player->getAnimController().playActionSeamless("hold");
    
    mWaitTimer = 0.0f;
    
    int level = 1;
    CMenu* menu = player->getMenu();
    if (menu) {
        level = menu->getPlayerStats().level;
        if (level < 1) level = 1;
        if (level > 15) level = 15;
    }
    
    float t = (float)(level - 1) / 14.0f;
    float minTime = 2.0f - t * 1.5f;
    float maxTime = 30.0f - t * 29.0f;
    float range = maxTime - minTime;
    mBiteTime = minTime + (rand() % 100) / 100.0f * range;
}

std::string CPlayerHoldState::update(CPlayer* player, float dt)
{
    player->updateBobber(dt);
    
    if (player->hasBobberLanded()) {
        const ColFloorResult& floorResult = player->getThrowFloorResult();
        if (!(floorResult.flags & COL_FLAG_WATER)) {
            player->getAnimController().forceBlendFactor(0.0f);
            return "idle";
        }
    }
    
    mWaitTimer += dt;
    
    joypad_buttons_t pressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    
    if (pressed.b) {
        player->getAnimController().forceBlendFactor(0.0f);
        return "idle";
    }
    
    if (mWaitTimer >= mBiteTime) {
        //CSoundMgr::play("fish_miss");
        return "reel";
    }
    
    return "";
}

void CPlayerHoldState::exit(CPlayer* player)
{
    player->getAnimController().holdAction(false);
    player->getAnimController().stopAction();
}

void CPlayerReelState::selectRandomFish()
{
    if (!mFishPool || mFishPoolCount == 0) {
        mSelectedFish = nullptr;
        mSelectedFishIndex = -1;
        mRequiredTaps = 10;
        return;
    }
    
    int roll = rand() % 100;
    EFishRarity targetRarity;
    
    if (roll < 5) {
        targetRarity = EFishRarity::Legendary;
    } else if (roll < 20) {
        targetRarity = EFishRarity::Rare;
    } else if (roll < 50) {
        targetRarity = EFishRarity::Uncommon;
    } else {
        targetRarity = EFishRarity::Common;
    }
    
    int candidateIndices[32]{};
    int candidateCount = 0;
    
    for (int i = 0; i < mFishPoolCount && candidateCount < 32; i++) {
        if (mFishPool[i].rarity == targetRarity) {
            candidateIndices[candidateCount++] = i;
        }
    }
    
    if (candidateCount == 0) {
        for (int i = 0; i < mFishPoolCount && candidateCount < 32; i++) {
            candidateIndices[candidateCount++] = i;
        }
    }
    
    if (candidateCount > 0) {
        mSelectedFishIndex = candidateIndices[rand() % candidateCount];
        mSelectedFish = &mFishPool[mSelectedFishIndex];
        mRequiredTaps = mSelectedFish->requiredTaps;
    } else {
        mSelectedFish = nullptr;
        mSelectedFishIndex = -1;
        mRequiredTaps = 10;
    }
}

void CPlayerReelState::init(CPlayer* player)
{
    player->getAnimController().playAction("reel");
    player->getAnimController().holdAction(true);
    
    CMenu* menu = player->getMenu();
    if (menu) {
        int baitIndex = menu->getEquippedBaitIndex();
        if (baitIndex >= 0) {
            const SMenuItem* bait = menu->getItem(EMenuTab::Bait, baitIndex);
            if (bait && bait->quantity > 0) {
                int newQty = bait->quantity - 1;
                if (newQty <= 0) {
                    menu->removeItem(EMenuTab::Bait, baitIndex);
                } else {
                    menu->updateItemQuantity(EMenuTab::Bait, baitIndex, newQty);
                }
            }
        }
    }
    
    selectRandomFish();
    
    mTapCount = 0;
    
    mLastTapTime = 0.0f;
    
    mMaxTapTime = 10.0f;
    mTimeElapsed = 0.0f;
}

std::string CPlayerReelState::update(CPlayer* player, float dt)
{
    player->updateBobber(dt);
    
    mTimeElapsed += dt;
    
    if (mTimeElapsed >= mMaxTapTime) {
        CSoundMgr::play("fish_miss", false);
        return "idle";
    }
    
    joypad_buttons_t pressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    
    if (pressed.a) {
        mTapCount++;
        mLastTapTime = 0.0f;
        
        //CSoundMgr::play("p2mp");
        
        if (mTapCount >= mRequiredTaps) {
            
            if (mSelectedFish) {
                static SItemGetData fishItem;
                fishItem.name = mSelectedFish->name;
                fishItem.description = mSelectedFish->description;
                fishItem.modelPath = mSelectedFish->modelPath;
                fishItem.inventoryTab = EMenuTab::MiscItems;
                fishItem.quantity = 1;
                fishItem.iconIndex = mSelectedFish->iconIndex;
                fishItem.exp = mSelectedFish->exp;
                fishItem.fishIndex = mSelectedFishIndex;
                
                player->triggerItemGet(fishItem, "idle");
                return "item_get";
            }
            
            return "idle";
        }
    }
    
    mLastTapTime += dt;
    
    if (pressed.b) {
        CSoundMgr::play("fish_miss", false);
        return "idle";
    }
    
    return "";
}

void CPlayerReelState::exit(CPlayer* player)
{
    player->getAnimController().holdAction(false);
    player->getAnimController().stopAction();
}

void CPlayerItemGetState::init(CPlayer* player)
{
    mTimer = 0.0f;
    mItemRotation = 0.0f;
    mFadeAlpha = 1.0f;
    mWaitingForButton = false;
    mFadingOut = false;
    
    mRotationStart = player->getRotY();
    mRotationTarget = (-player->getCameraAngle()) + 3.14159265f;
    
    CSoundMgr::play("fish_get");
    
    player->getAnimController().setBlendFactor(0.0f);
    
    if (player->getCamera()) {
        player->getCamera()->startItemGet();
    }
    
    if (player->getMenu() && player->getCurrentItem()) {
        const SItemGetData* item = player->getCurrentItem();
        player->getMenu()->addItem(item->inventoryTab, item->name, item->quantity, item->iconIndex, item->modelPath);
        
        if (item->fishIndex >= 0) {
            player->getMenu()->addFishCaught(1);
            player->getMenu()->registerFishCaught(item->fishIndex);
        }
        
        if (item->exp > 0) {
            player->awardExp(item->exp);
        }
    }
}

std::string CPlayerItemGetState::update(CPlayer* player, float dt)
{
    mTimer += dt;
    
    if (mFadingOut) {
        mFadeAlpha -= dt * 2.0f;
        if (mFadeAlpha <= 0.0f) {
            mFadeAlpha = 0.0f;
            const std::string& returnState = player->getItemGetReturnState();
            if (returnState == "conversation") {
                return "idle";
            }
            return returnState.empty() ? "idle" : returnState;
        }
    } else if (mWaitingForButton) {
        joypad_buttons_t pressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);
        if (pressed.a || pressed.b || pressed.start) {
            mFadingOut = true;
            mTimer = 0.0f;
        }
    } else {
        
        if (mTimer >= 2.0f) {
            mWaitingForButton = true;
        }
    }
    
    mItemRotation += dt * 2.0f;
    while (mItemRotation > (T3D_PI * 2.0f)) mItemRotation -= (T3D_PI * 2.0f);
    
    return "";
}

void CPlayerItemGetState::exit(CPlayer* player)
{
    mWaitingForButton = false;
    mFadingOut = false;
    
    player->triggerPendingLevelUp();
    
    player->closeItemGetTextBox();
    
    if (player->getCamera()) {
        player->getCamera()->endItemGet();
    }

    player->getAnimController().forceBlendFactor(0.0f);
    
    if (player->getMenu() && gSaveManager.isAvailable()) {
        gSaveManager.save(*player->getMenu());
    }
}