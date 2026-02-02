#include "light.hpp"

void CLight::setAmbient(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    mAmbientColor[0] = r;
    mAmbientColor[1] = g;
    mAmbientColor[2] = b;
    mAmbientColor[3] = a;
}

void CLight::setDirectional(int index, uint8_t r, uint8_t g, uint8_t b, TVec3F const& direction)
{
    mDirColor[0] = r;
    mDirColor[1] = g;
    mDirColor[2] = b;
    mDirColor[3] = 0xFF;
    
    mLightDir = {{direction.x(), direction.y(), direction.z()}};
    t3d_vec3_norm(&mLightDir);
}

void CLight::setCount(int count)
{
    mLightCount = count;
}

void CLight::apply()
{
    t3d_light_set_ambient(mAmbientColor);
    t3d_light_set_directional(0, mDirColor, &mLightDir);
    t3d_light_set_count(mLightCount);
}

void CLight::lerpColors(uint8_t ambR1, uint8_t ambG1, uint8_t ambB1,
                        uint8_t ambR2, uint8_t ambG2, uint8_t ambB2,
                        uint8_t dirR1, uint8_t dirG1, uint8_t dirB1,
                        uint8_t dirR2, uint8_t dirG2, uint8_t dirB2,
                        float t)
{
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    
    mAmbientColor[0] = (uint8_t)(ambR1 + (ambR2 - ambR1) * t);
    mAmbientColor[1] = (uint8_t)(ambG1 + (ambG2 - ambG1) * t);
    mAmbientColor[2] = (uint8_t)(ambB1 + (ambB2 - ambB1) * t);
    
    mDirColor[0] = (uint8_t)(dirR1 + (dirR2 - dirR1) * t);
    mDirColor[1] = (uint8_t)(dirG1 + (dirG2 - dirG1) * t);
    mDirColor[2] = (uint8_t)(dirB1 + (dirB2 - dirB1) * t);
}
