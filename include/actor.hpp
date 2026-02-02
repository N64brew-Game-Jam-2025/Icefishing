#pragma once

#include <memory>
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>
#include "t3d/t3danim.h"
#include <t3d/t3dskeleton.h>
#include <vector>

#include "math.hpp"

class CActor
{
    public:

    CActor();
    ~CActor() = default;

    virtual void init(TVec3F const & pos, std::string const & modelPath);
    virtual void update(float const dt);
    virtual void lateUpdate(float const dt);
    virtual void draw();

    TVec3F const & getPosition() const { return mPosition; }
    TVec3F const & getRotation() const { return mRotation; }
    TVec3F const & getScale() const { return mScale; }

    void createSkeleton();
    
    private:

    TVec3F mPosition{};
    TVec3F mRotation{};
    TVec3F mScale{};

    TMtx44 mModelMtxF{};
    TMtx44 mModelMtx{};

    T3DModel * mModel{};
    rspq_block_t * mDlBlock{};
    T3DSkeleton mSkeleton{};
    T3DSkeleton mSkeletonBlend{};

    std::vector<T3DAnim> mAnimList{};

    bool mHasSkel{false};
};