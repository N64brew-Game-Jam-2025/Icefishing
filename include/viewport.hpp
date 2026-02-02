#pragma once

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include "math.hpp"

class CViewport final
{
public:
    CViewport() = default;
    ~CViewport() = default;

    void init();
    void setProjection(float fov, float nearPlane, float farPlane);
    void lookAt(TVec3F const& camPos, TVec3F const& target);
    void attach();

    T3DViewport* getViewport() { return &mViewport; }

private:
    T3DViewport mViewport{};
    float mFov{85.0f};
    float mNear{10.0f};
    float mFar{150.0f};
};
