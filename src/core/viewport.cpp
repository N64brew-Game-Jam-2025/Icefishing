#include "viewport.hpp"

void CViewport::init()
{
    mViewport = t3d_viewport_create();
}

void CViewport::setProjection(float fov, float nearPlane, float farPlane)
{
    mFov = fov;
    mNear = nearPlane;
    mFar = farPlane;
    t3d_viewport_set_projection(&mViewport, T3D_DEG_TO_RAD(mFov), mNear, mFar);
}

void CViewport::lookAt(TVec3F const& camPos, TVec3F const& target)
{
    T3DVec3 pos = {{camPos.x(), camPos.y(), camPos.z()}};
    T3DVec3 tgt = {{target.x(), target.y(), target.z()}};
    T3DVec3 up = {{0.0f, 1.0f, 0.0f}};
    t3d_viewport_look_at(&mViewport, &pos, &tgt, &up);
}

void CViewport::attach()
{
    t3d_viewport_attach(&mViewport);
}
