#pragma once

#include <libdragon.h>
#include <t3d/t3dmath.h>

namespace MathUtil {

    inline float lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }
    
    inline float lerpAngle(float a, float b, float t) {
        float diff = b - a;
        while (diff > T3D_PI) diff -= 2.0f * T3D_PI;
        while (diff < -T3D_PI) diff += 2.0f * T3D_PI;
        return a + diff * t;
    }

    inline float normalizeAngle(float angle) {
        while (angle > T3D_PI) angle -= 2.0f * T3D_PI;
        while (angle < -T3D_PI) angle += 2.0f * T3D_PI;
        return angle;
    }
    
    inline float clamp(float value, float min, float max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }
}

class TUtil
{
    public:

    float getTimeSeconds() {
        return (float)((double)get_ticks_us() / 1000000.0);
    }

    float getDeltaTime() {
        return sDeltaTime;
    }

    void updateTime();

    private:

    static float sLastTime;
    static float sCurrTime;
    static float sDeltaTime;
};