#pragma once

#include <string>

class CPlayer;
struct SItemGetData;

enum class EFishRarity {
    Common,
    Uncommon,
    Rare,
    Legendary
};

struct SFishData {
    const char* name;
    const char* description;
    const char* modelPath;
    int sellPrice;
    int requiredTaps;
    EFishRarity rarity;
    int iconIndex;
    int exp;
};

class CPlayerState {
public:
    virtual ~CPlayerState() = default;
    
    virtual void init(CPlayer* player) = 0;
    virtual std::string update(CPlayer* player, float dt) = 0;
    virtual void exit(CPlayer* player) = 0;
    
    virtual const char* getName() const = 0;

protected:
    static std::string checkCommonTransitions(CPlayer* player);
};

class CPlayerIdleState : public CPlayerState {
public:
    void init(CPlayer* player) override;
    std::string update(CPlayer* player, float dt) override;
    void exit(CPlayer* player) override;
    const char* getName() const override { return "idle"; }
};

class CPlayerWalkState : public CPlayerState {
public:
    void init(CPlayer* player) override;
    std::string update(CPlayer* player, float dt) override;
    void exit(CPlayer* player) override;
    const char* getName() const override { return "walk"; }
};

class CPlayerRunState : public CPlayerState {
public:
    void init(CPlayer* player) override;
    std::string update(CPlayer* player, float dt) override;
    void exit(CPlayer* player) override;
    const char* getName() const override { return "run"; }
};

class CPlayerPrepState : public CPlayerState {
public:
    void init(CPlayer* player) override;
    std::string update(CPlayer* player, float dt) override;
    void exit(CPlayer* player) override;
    const char* getName() const override { return "prep"; }
};

class CPlayerThrowState : public CPlayerState {
public:
    void init(CPlayer* player) override;
    std::string update(CPlayer* player, float dt) override;
    void exit(CPlayer* player) override;
    const char* getName() const override { return "throw"; }
};

class CPlayerHoldState : public CPlayerState {
public:
    void init(CPlayer* player) override;
    std::string update(CPlayer* player, float dt) override;
    void exit(CPlayer* player) override;
    const char* getName() const override { return "hold"; }
    
private:
    float mWaitTimer{0.0f};
    float mBiteTime{0.0f};
};

class CPlayerReelState : public CPlayerState {
public:
    void init(CPlayer* player) override;
    std::string update(CPlayer* player, float dt) override;
    void exit(CPlayer* player) override;
    const char* getName() const override { return "reel"; }
    
    int getTapCount() const { return mTapCount; }
    int getRequiredTaps() const { return mRequiredTaps; }
    float getTimeElapsed() const { return mTimeElapsed; }
    float getMaxTapTime() const { return mMaxTapTime; }
    
    void setFishPool(const SFishData* fish, int count) { mFishPool = fish; mFishPoolCount = count; }
    
private:
    void selectRandomFish();
    
    int mTapCount{0};
    int mRequiredTaps{10};
    float mLastTapTime{0.0f};
    float mTimeElapsed{0.0f};
    float mMaxTapTime{10.0f};
    
    const SFishData* mFishPool{nullptr};
    int mFishPoolCount{0};
    const SFishData* mSelectedFish{nullptr};
    int mSelectedFishIndex{-1};
};

class CPlayerItemGetState : public CPlayerState {
public:
    void init(CPlayer* player) override;
    std::string update(CPlayer* player, float dt) override;
    void exit(CPlayer* player) override;
    const char* getName() const override { return "item_get"; }
    
    float getItemRotation() const { return mItemRotation; }
    float getFadeAlpha() const { return mFadeAlpha; }
    
private:
    float mTimer{0.0f};
    float mRotationStart{0.0f};
    float mRotationTarget{0.0f};
    float mItemRotation{0.0f};
    float mFadeAlpha{1.0f};
    std::string mPreviousState;
    bool mWaitingForButton{false};
    bool mFadingOut{false};
};

class CPlayerStateMachine {
public:
    CPlayerStateMachine() = default;
    ~CPlayerStateMachine() = default;
    
    void init(CPlayer* player, const std::string& startState);
    void update(CPlayer* player, float dt);
    
    void transitionTo(CPlayer* player, const std::string& stateName);
    
    const char* getCurrentStateName() const;
    bool isInState(const std::string& name) const;
    
    const CPlayerItemGetState& getItemGetState() const { return mItemGetState; }
    const CPlayerReelState& getReelState() const { return mReelState; }
    CPlayerReelState& getReelState() { return mReelState; }

private:
    CPlayerState* getState(const std::string& name);
    
    CPlayerIdleState mIdleState;
    CPlayerWalkState mWalkState;
    CPlayerRunState mRunState;
    CPlayerPrepState mPrepState;
    CPlayerThrowState mThrowState;
    CPlayerHoldState mHoldState;
    CPlayerReelState mReelState;
    CPlayerItemGetState mItemGetState;
    
    CPlayerState* mCurrentState = nullptr;
};
