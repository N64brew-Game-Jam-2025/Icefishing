#include "anim_controller.hpp"

void CAnimController::init(CSkinnedModel* model)
{
    mModel = model;
}

void CAnimController::setBaseAnimation(const std::string& name, const AnimationConfig& config)
{
    mBaseAnim = name;
    mModel->setAnimationLooping(name, config.loops);
    mModel->setAnimationSpeed(name, config.speed);
}

void CAnimController::setMovementAnimation(const std::string& name, const AnimationConfig& config)
{
    mMovementAnim = name;
    mModel->setAnimationLooping(name, config.loops);
    mModel->setAnimationSpeed(name, config.speed);
}

void CAnimController::registerAction(const std::string& name, const AnimationConfig& config)
{
    mModel->setAnimationLooping(name, config.loops);
    mModel->setAnimationSpeed(name, config.speed);
    mModel->stopAnimation(name);
}

void CAnimController::setMovementSpeed(float speed)
{
    if (!mMovementAnim.empty()) {
        mModel->setAnimationSpeed(mMovementAnim, speed);
    }
}

void CAnimController::playAction(const std::string& name)
{
    mActionAnim = name;
    mActionActive = true;
    mActionHold = false;
    mActionWeight = 0.0f;
    
    mModel->resetAnimation(name);
    mModel->playAnimation(name);
}

void CAnimController::playActionSeamless(const std::string& name)
{
    float preservedWeight = mActionWeight;
    
    mActionAnim = name;
    mActionActive = true;
    mActionHold = false;
    mActionWeight = preservedWeight > 0.5f ? preservedWeight : 1.0f;
    
    mModel->resetAnimation(name);
    mModel->playAnimation(name);
}

void CAnimController::stopAction()
{
    if (mActionAnim.empty()) return;
    
    mModel->stopAnimation(mActionAnim);
    mActionActive = false;
    mActionHold = false;
}

bool CAnimController::isActionFinished() const
{
    if (!mModel || mActionAnim.empty()) return true;
    return !mModel->isAnimationPlaying(mActionAnim);
}

void CAnimController::update(float dt)
{
    if (!mModel) return;
    
    const float blendSpeed = 8.0f;
    if (mCurrentBlend < mTargetBlend) {
        mCurrentBlend += blendSpeed * dt;
        if (mCurrentBlend > mTargetBlend) mCurrentBlend = mTargetBlend;
    } else if (mCurrentBlend > mTargetBlend) {
        mCurrentBlend -= blendSpeed * dt;
        if (mCurrentBlend < mTargetBlend) mCurrentBlend = mTargetBlend;
    }
    
    const float actionBlendInSpeed = 10.0f;
    const float actionBlendOutSpeed = 20.0f;
    if (mActionActive) {
        if (mActionWeight < 1.0f) {
            mActionWeight += actionBlendInSpeed * dt;
            if (mActionWeight > 1.0f) mActionWeight = 1.0f;
        }
        
        if (!mModel->isAnimationPlaying(mActionAnim) && !mActionHold) {
            mActionActive = false;
        }
    } else if (mActionWeight > 0.0f && !mActionHold) {
        mActionWeight -= actionBlendOutSpeed * dt;
        if (mActionWeight < 0.1f) {
            mActionWeight = 0.0f;
            mActionAnim.clear();
        }
    }
    
    if (!mBaseAnim.empty()) {
        mModel->updateAnimation(mBaseAnim, dt);
    }
    
    if (!mMovementAnim.empty()) {
        mModel->updateAnimation(mMovementAnim, dt);
    }
    
    if (!mActionAnim.empty() && (mActionActive || mActionWeight > 0.0f)) {
        mModel->updateAnimation(mActionAnim, dt);
    }
    
    float effectiveBlend = mCurrentBlend;
    if (mActionWeight > 0.0f) {
        effectiveBlend *= (1.0f - mActionWeight * 0.7f);
    }
    mModel->blendAnimations(mBaseAnim, mMovementAnim, effectiveBlend);
    
    if (mActionWeight > 0.0f) {
        mModel->blendAction(mActionWeight);
    }
}
