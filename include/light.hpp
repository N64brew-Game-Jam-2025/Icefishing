#pragma once

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include "math.hpp"

class CLight final
{
public:
    CLight() = default;
    ~CLight() = default;

    void setAmbient(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    void setDirectional(int index, uint8_t r, uint8_t g, uint8_t b, TVec3F const& direction);
    void setCount(int count);
    void apply();
    
    void lerpColors(uint8_t ambR1, uint8_t ambG1, uint8_t ambB1,
                    uint8_t ambR2, uint8_t ambG2, uint8_t ambB2,
                    uint8_t dirR1, uint8_t dirG1, uint8_t dirB1,
                    uint8_t dirR2, uint8_t dirG2, uint8_t dirB2,
                    float t);

private:
    uint8_t mAmbientColor[4]{0xAA, 0xAA, 0xAA, 0xFF};
    uint8_t mDirColor[4]{0xFF, 0xAA, 0xAA, 0xFF};
    T3DVec3 mLightDir{{1.0f, 1.0f, 1.0f}};
    int mLightCount{1};
};
