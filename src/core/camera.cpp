#include "camera.hpp"
#include <t3d/t3dmath.h>
#include <cmath>

static float normalizeAngle(float angle)
{
    while (angle > T3D_PI) angle -= (T3D_PI * 2.0f);
    while (angle < -T3D_PI) angle += (T3D_PI * 2.0f);
    return angle;
}

static float lerpAngle(float from, float to, float t)
{
    float diff = normalizeAngle(to - from);
    return from + diff * t;
}

void CCamera::init()
{
    mOrbitAngle = 0.0f;
    mOrbitTarget = 0.0f;
    mDistance = 65.0f;
    mDistanceTarget = 65.0f;
    mHeight = 45.0f;
    mTargetOffsetY = 10.0f;
}

void CCamera::update(float dt, const TVec3F& targetPos, joypad_inputs_t& joypad)
{
    if (mInItemGet) {
        mDistanceTarget = 35.0f;
    } else if (mInConversation) {
        TVec3F dir(mConvNpcPos.x() - targetPos.x(), 0.0f, mConvNpcPos.z() - targetPos.z());
        float distToNpc = sqrtf(dir.x()*dir.x() + dir.z()*dir.z());
        
        if (distToNpc > 0.1f) {
            dir.x() /= distToNpc;
            dir.z() /= distToNpc;
        }

        float npcAngle = atan2f(dir.x(), dir.z());
        float targetAngle = npcAngle + T3D_PI + 0.3f;
        
        mOrbitTarget = targetAngle;
        mDistanceTarget = 40.0f;
    } else {
        if (joypad.btn.c_left) {
            mOrbitTarget += mOrbitSpeed * dt;
        }
        if (joypad.btn.c_right) {
            mOrbitTarget -= mOrbitSpeed * dt;
        }
        
        if (joypad.btn.c_up) {
            mDistanceTarget -= mZoomSpeed * dt;
            if (mDistanceTarget < mMinDistance) mDistanceTarget = mMinDistance;
        }
        if (joypad.btn.c_down) {
            mDistanceTarget += mZoomSpeed * dt;
            if (mDistanceTarget > mMaxDistance) mDistanceTarget = mMaxDistance;
        }
    }

    float smoothFactor = mSmoothSpeed * dt;
    mOrbitAngle = lerpAngle(mOrbitAngle, mOrbitTarget, smoothFactor);
    mDistance = t3d_lerp(mDistance, mDistanceTarget, smoothFactor);

    mLookTarget = {targetPos.x(), targetPos.y() + mTargetOffsetY, targetPos.z()};

    mPosition = {
        mLookTarget.x() + sinf(mOrbitAngle) * mDistance,
        mLookTarget.y() + mHeight,
        mLookTarget.z() + cosf(mOrbitAngle) * mDistance
    };
}

void CCamera::startConversation(const TVec3F& npcPos)
{
    if (mInConversation) return;
    
    mInConversation = true;
    mConvNpcPos = npcPos;
    
    mSavedOrbitAngle = mOrbitTarget;
    mSavedDistance = mDistanceTarget;
}

void CCamera::endConversation()
{
    if (!mInConversation) return;
    
    mInConversation = false;
    
    mOrbitTarget = mSavedOrbitAngle;
    mDistanceTarget = mSavedDistance;
}

void CCamera::startItemGet()
{
    if (mInItemGet) return;
    
    mInItemGet = true;
    
    mItemGetSavedDistance = mDistanceTarget;
}

void CCamera::endItemGet()
{
    if (!mInItemGet) return;
    
    mInItemGet = false;
    
    mDistanceTarget = mItemGetSavedDistance;
}

void CCamera::applyCollision(CCollisionMesh& collision)
{
    if (!collision.isLoaded()) return;

    float dx = mPosition.x() - mLookTarget.x();
    float dy = mPosition.y() - mLookTarget.y();
    float dz = mPosition.z() - mLookTarget.z();
    float dist = sqrtf(dx*dx + dy*dy + dz*dz);
    
    if (dist > 0.1f) {
        dx /= dist; 
        dy /= dist; 
        dz /= dist;
        
        float hitDist;
        float startOffset = 2.0f;
        if (collision.raycast(
            mLookTarget.x() + dx * startOffset, 
            mLookTarget.y() + dy * startOffset, 
            mLookTarget.z() + dz * startOffset,
            dx, dy, dz, 
            dist - startOffset, 
            &hitDist, nullptr)) 
        {
            float safeDistance = startOffset + hitDist - 8.0f;
            if (safeDistance < 8.0f) safeDistance = 8.0f;
            
            mPosition = {
                mLookTarget.x() + dx * safeDistance,
                mLookTarget.y() + dy * safeDistance,
                mLookTarget.z() + dz * safeDistance
            };
        }
    }
}

void CCamera::apply(CViewport& viewport)
{
    viewport.lookAt(mPosition, mLookTarget);
}