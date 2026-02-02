#include "actor.hpp"
#include <string>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>

CActor::CActor()
{
    mPosition = {0.0f, 0.0f, 0.0f};
    mRotation = {0.0f, 0.0f, 0.0f};
    mScale = {1.0f, 1.0f, 1.0f};
}

void CActor::init(TVec3F const & pos, std::string const & modelPath)
{
    mPosition = pos;

    if (mModel != nullptr)
    {
        mModel = t3d_model_load(modelPath.c_str());
    }
}

void CActor::update(float const dt)
{
    
}

void CActor::lateUpdate(float const dt)
{
    if (mHasSkel) {
        t3d_skeleton_update(&mSkeleton);
    }
    
    // update object matrix
    t3d_mat4fp_from_srt_euler((T3DMat4FP *)&mModelMtx,
      (float[]){mScale.x(),mScale.y(),mScale.z()},
      (float[]){mRotation.x(),-mRotation.y(),mRotation.z()},
      (float[]){mPosition.x(),mPosition.y(),mPosition.z()}
    );
}

void CActor::draw()
{
    
}

void CActor::createSkeleton()
{
    mSkeleton = t3d_skeleton_create(mModel);
    mSkeletonBlend = t3d_skeleton_clone(&mSkeleton, false);
    mHasSkel = true;
}