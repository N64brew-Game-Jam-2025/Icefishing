#include "wipe.hpp"
#include <math.h>
#include <string.h>

static void fillOutsideCircle(float centerX, float centerY, float radius)
{
    const int screenW = 256;
    const int screenH = 240;

    const float radiusSq = radius * radius;

    int topY = (int)floorf(centerY - radius);
    int bottomY = (int)ceilf(centerY + radius);
    if (topY < 0) topY = 0;
    if (bottomY > screenH) bottomY = screenH;

    // top / bottom bands
    if (topY > 0) rdpq_fill_rectangle(0, 0, screenW, topY);
    if (bottomY < screenH) rdpq_fill_rectangle(0, bottomY, screenW, screenH);

    bool hasRun = false;
    int runY = topY;
    int runHeight = 0;
    int runLeft = 0;
    int runRight = screenW;

    for (int y = topY; y < bottomY; y++) {
        const float dy = ((float)y + 0.5f) - centerY;
        float inside = radiusSq - dy * dy;
        if (inside < 0.0f) inside = 0.0f;

        const float halfWidth = sqrtf(inside);

        int leftX = (int)floorf(centerX - halfWidth);
        int rightX = (int)ceilf(centerX + halfWidth);
        if (leftX < 0) leftX = 0;
        if (rightX > screenW) rightX = screenW;

        if (!hasRun) {
            hasRun = true;
            runY = y;
            runHeight = 1;
            runLeft = leftX;
            runRight = rightX;
            continue;
        }

        if (leftX == runLeft && rightX == runRight) {
            runHeight++;
            continue;
        }

        if (runLeft > 0) rdpq_fill_rectangle(0, runY, runLeft, runY + runHeight);
        if (runRight < screenW) rdpq_fill_rectangle(runRight, runY, screenW, runY + runHeight);

        runY = y;
        runHeight = 1;
        runLeft = leftX;
        runRight = rightX;
    }

    if (hasRun) {
        if (runLeft > 0) rdpq_fill_rectangle(0, runY, runLeft, runY + runHeight);
        if (runRight < screenW) rdpq_fill_rectangle(runRight, runY, screenW, runY + runHeight);
    }
}

void CCircleWipe::init()
{
    if (mInitialized) return;
    
    mMaxRadius = sqrtf(256.0f * 256.0f + 240.0f * 240.0f) * 0.5f + 10.0f;
    mRadius = mMaxRadius;
    mTargetRadius = mMaxRadius;
    
    mSprite = sprite_load("rom:/circle_mask.sprite");
    
    mInitialized = true;
}

void CCircleWipe::destroy()
{
    if (mInitialized) {
        sprite_free(mSprite);
        mInitialized = false;
    }
}

void CCircleWipe::wipeIn(float duration)
{
    mTargetRadius = mMaxRadius;
    mSpeed = mMaxRadius / duration;
    mActive = true;
}

void CCircleWipe::wipeOut(float duration)
{
    mTargetRadius = 0.0f;
    mSpeed = mMaxRadius / duration;
    mActive = true;
}

void CCircleWipe::update(float deltaTime)
{
    if (!mActive) return;
    
    float diff = mTargetRadius - mRadius;
    float step = mSpeed * deltaTime;
    
    if (fabsf(diff) <= step) {
        mRadius = mTargetRadius;
        mActive = false;
    } else {
        mRadius += (diff > 0.0f) ? step : -step;
    }
}

void CCircleWipe::draw()
{
    if (!mInitialized) return;
    
    if (mRadius >= mMaxRadius) return;

    rdpq_sync_pipe();
    rdpq_mode_push();
    
    // if fully closed, just draw black screen
    if (mRadius <= 0.0f) {
        rdpq_set_mode_fill(RGBA32(0, 0, 0, 255));
        rdpq_fill_rectangle(0, 0, 256, 240);
        rdpq_mode_pop();
        return;
    }

    rdpq_set_mode_fill(RGBA32(0, 0, 0, 255));
    fillOutsideCircle(mCenterX, mCenterY, mRadius);
    rdpq_mode_pop();
}