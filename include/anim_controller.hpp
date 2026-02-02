#pragma once

#include <string>
#include <unordered_map>
#include "skinned_model.hpp"

struct AnimationConfig {
    bool loops = true;
    float speed = 1.0f;
};

class CAnimController {
public:
    CAnimController() = default;
    ~CAnimController() = default;

    void init(CSkinnedModel* model);
    void setBaseAnimation(const std::string& name, const AnimationConfig& config = {});
    void setMovementAnimation(const std::string& name, const AnimationConfig& config = {});
    void registerAction(const std::string& name, const AnimationConfig& config = {});
    void setBlendFactor(float blend) { mTargetBlend = blend; }
    void forceBlendFactor(float blend) { mTargetBlend = blend; mCurrentBlend = blend; }
    void setMovementSpeed(float speed);
    void playAction(const std::string& name);
    void playActionSeamless(const std::string& name);
    void holdAction(bool hold) { mActionHold = hold; }
    void stopAction();
    bool isActionFinished() const;
    void update(float dt);
    bool isActionPlaying() const { return mActionActive || mActionWeight > 0.0f; }
    float getBlendFactor() const { return mCurrentBlend; }

private:
    CSkinnedModel* mModel = nullptr;

    std::string mBaseAnim;
    std::string mMovementAnim;

    float mCurrentBlend = 0.0f;
    float mTargetBlend = 0.0f;

    std::string mActionAnim;
    bool mActionActive = false;
    bool mActionHold = false;
    float mActionWeight = 0.0f;
};
