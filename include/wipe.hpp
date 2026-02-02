#pragma once

#include <libdragon.h>

class CCircleWipe {
public:
    void init();
    void destroy();
    void wipeIn(float duration = 0.5f);
    void wipeOut(float duration = 0.5f);
    
    void update(float deltaTime);
    void draw();
    
    bool isActive() const { return mActive; }
    bool isClosed() const { return mRadius <= 0.0f && !mActive; }
    bool isOpen() const { return mRadius >= mMaxRadius && !mActive; }
    
    void setCenter(float x, float y) { mCenterX = x; mCenterY = y; }
    
private:
    sprite_t *mSprite = nullptr;
    bool mInitialized = false;
    bool mActive = false;
    
    float mRadius = 0.0f;
    float mTargetRadius = 0.0f;
    float mSpeed = 0.0f;
    float mMaxRadius = 0.0f;
    
    float mCenterX = 140.0f;
    float mCenterY = 120.0f;
};

class CWhiteFade {
public:
    void init() { mAlpha = 0.0f; mActive = false; }
    
    void fadeOut(float duration = 0.3f) {
        mActive = true;
        mFadingOut = true;
        mDuration = duration;
        mTimer = 0.0f;
    }
    
    void fadeIn(float duration = 0.3f) {
        mActive = true;
        mFadingOut = false;
        mAlpha = 1.0f;
        mDuration = duration;
        mTimer = 0.0f;
    }
    
    void update(float deltaTime) {
        if (!mActive) return;
        
        mTimer += deltaTime;
        float t = mTimer / mDuration;
        if (t >= 1.0f) {
            t = 1.0f;
            mActive = false;
        }
        
        if (mFadingOut) {
            mAlpha = t;
        } else {
            mAlpha = 1.0f - t;
        }
    }
    
    void draw() {
        if (mAlpha <= 0.0f) return;
        
        rdpq_sync_pipe();
        rdpq_set_mode_standard();
        rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        
        uint8_t alpha = (uint8_t)(mAlpha * 255);
        rdpq_set_prim_color(RGBA32(255, 255, 255, alpha));
        
        rdpq_fill_rectangle(0, 0, 640, 480);
    }
    
    bool isActive() const { return mActive; }
    bool isFadedOut() const { return !mActive && mAlpha >= 1.0f; }
    bool isFadedIn() const { return !mActive && mAlpha <= 0.0f; }
    
private:
    float mAlpha = 0.0f;
    float mDuration = 0.3f;
    float mTimer = 0.0f;
    bool mActive = false;
    bool mFadingOut = true;
};
