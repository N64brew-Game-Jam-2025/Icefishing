#pragma once

#include <t3d/t3d.h>
#include "math.hpp"
#include "collision.hpp"
#include "viewport.hpp"

class CCamera final
{
public:
    CCamera() = default;
    ~CCamera() = default;

    void init();
    void update(float dt, const TVec3F& targetPos, joypad_inputs_t& joypad);
    void applyCollision(CCollisionMesh& collision);
    void apply(CViewport& viewport);

    TVec3F getPosition() const { return mPosition; }
    TVec3F getTarget() const { return mLookTarget; }
    float getOrbitAngle() const { return mOrbitAngle; }
    float getDistance() const { return mDistance; }
    float getHeight() const { return mHeight; }

    void setOrbitAngle(float angle) { mOrbitAngle = angle; mOrbitTarget = angle; }
    void setDistance(float dist) { mDistance = dist; mDistanceTarget = dist; }
    void setHeight(float height) { mHeight = height; }
    void setTargetOffset(float offsetY) { mTargetOffsetY = offsetY; }
    
    void setDistanceLimits(float minDist, float maxDist) { mMinDistance = minDist; mMaxDistance = maxDist; }
    void setOrbitSpeed(float speed) { mOrbitSpeed = speed; }
    void setZoomSpeed(float speed) { mZoomSpeed = speed; }
    void setSmoothSpeed(float speed) { mSmoothSpeed = speed; }

    void startConversation(const TVec3F& npcPos);
    void endConversation();
    bool isInConversation() const { return mInConversation; }
    
    void startItemGet();
    void endItemGet();
    bool isInItemGet() const { return mInItemGet; }

private:
    TVec3F mPosition{0.0f, 45.0f, 65.0f};
    TVec3F mLookTarget{0.0f, 10.0f, 0.0f};
    
    float mOrbitAngle = 0.0f;
    float mOrbitTarget = 0.0f;
    float mDistance = 65.0f;
    float mDistanceTarget = 65.0f;
    float mHeight = 45.0f;
    float mTargetOffsetY = 10.0f;
    
    bool mInConversation = false;
    TVec3F mSavedPosition{0,0,0};
    TVec3F mSavedTarget{0,0,0};
    float mSavedOrbitAngle = 0.0f;
    float mSavedDistance = 0.0f;
    TVec3F mConvNpcPos{0,0,0};
    
    bool mInItemGet = false;
    float mItemGetSavedDistance = 0.0f;

    float mMinDistance = 30.0f;
    float mMaxDistance = 120.0f;
    
    float mOrbitSpeed = 2.5f;
    float mZoomSpeed = 40.0f;
    float mSmoothSpeed = 5.0f;
};