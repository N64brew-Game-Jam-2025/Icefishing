#include "skinned_model.hpp"

CSkinnedModel::~CSkinnedModel()
{
    unload();
}

void CSkinnedModel::unload()
{
    if (mHasSkeleton) {
        t3d_skeleton_destroy(&mSkeleton);
        mHasSkeleton = false;
    }
    if (mHasBlendSkeleton) {
        t3d_skeleton_destroy(&mSkeletonBlend);
        mHasBlendSkeleton = false;
    }
    if (mHasActionSkeleton) {
        t3d_skeleton_destroy(&mSkeletonAction);
        mHasActionSkeleton = false;
    }
    for (auto& [name, anim] : mAnimations) {
        t3d_anim_destroy(&anim);
    }
    mAnimations.clear();
    if (mBufferedMatrices) {
        free_uncached(mBufferedMatrices);
        mBufferedMatrices = nullptr;
        mNumBuffers = 0;
    }
    CModel::unload();
}

void CSkinnedModel::load(std::string const& path)
{
    CModel::load(path);
}

void CSkinnedModel::draw()
{
    if (mHasSkeleton && mBufferedMatrices && mNumBuffers > 0) {
        uint32_t bufferIdx = mFrameIndex % mNumBuffers;
        t3d_segment_set(SKINNED_MODEL_SEGMENT_ID, &mBufferedMatrices[bufferIdx]);
        t3d_skeleton_use(&mSkeleton);
    }
    CModel::draw();
}

void CSkinnedModel::createSkeleton()
{
    if (!mModel) return;
    
    mNumBuffers = display_get_num_buffers();
    
    mSkeleton = t3d_skeleton_create_buffered(mModel, mNumBuffers);
    mHasSkeleton = true;
    
    mSkeletonBlend = t3d_skeleton_clone(&mSkeleton, false);
    mHasBlendSkeleton = true;
    
    mSkeletonAction = t3d_skeleton_clone(&mSkeleton, false);
    mHasActionSkeleton = true;
    
    mBufferedMatrices = static_cast<T3DMat4FP*>(malloc_uncached(sizeof(T3DMat4FP) * mNumBuffers));
}

void CSkinnedModel::addAnimation(std::string const& name, int skeletonType)
{
    if (!mModel) return;

    auto existing = mAnimations.find(name);
    if (existing != mAnimations.end()) {
        t3d_anim_destroy(&existing->second);
        mAnimations.erase(existing);
    }
    
    T3DAnim anim = t3d_anim_create(mModel, name.c_str());
    
    if (skeletonType == 1 && mHasBlendSkeleton) {
        t3d_anim_attach(&anim, &mSkeletonBlend);
    } else if (skeletonType == 2 && mHasActionSkeleton) {
        t3d_anim_attach(&anim, &mSkeletonAction);
    } else if (mHasSkeleton) {
        t3d_anim_attach(&anim, &mSkeleton);
    }
    
    mAnimations[name] = anim;
}

void CSkinnedModel::playAnimation(std::string const& name)
{
    auto it = mAnimations.find(name);
    if (it != mAnimations.end()) {
        t3d_anim_set_playing(&it->second, true);
    }
}

void CSkinnedModel::stopAnimation(std::string const& name)
{
    auto it = mAnimations.find(name);
    if (it != mAnimations.end()) {
        t3d_anim_set_playing(&it->second, false);
    }
}

void CSkinnedModel::setAnimationSpeed(std::string const& name, float speed)
{
    auto it = mAnimations.find(name);
    if (it != mAnimations.end()) {
        t3d_anim_set_speed(&it->second, speed);
    }
}

void CSkinnedModel::setAnimationLooping(std::string const& name, bool loop)
{
    auto it = mAnimations.find(name);
    if (it != mAnimations.end()) {
        t3d_anim_set_looping(&it->second, loop);
    }
}

void CSkinnedModel::resetAnimation(std::string const& name)
{
    auto it = mAnimations.find(name);
    if (it != mAnimations.end()) {
        t3d_anim_set_time(&it->second, 0.0f);
    }
}

bool CSkinnedModel::isAnimationFinished(std::string const& name) const
{
    auto it = mAnimations.find(name);
    if (it != mAnimations.end()) {
        const T3DAnim& anim = it->second;
        return !anim.isPlaying && !anim.isLooping;
    }
    return false;
}

bool CSkinnedModel::isAnimationPlaying(std::string const& name) const
{
    auto it = mAnimations.find(name);
    if (it != mAnimations.end()) {
        return it->second.isPlaying;
    }
    return false;
}

void CSkinnedModel::updateAnimation(std::string const& name, float dt)
{
    auto it = mAnimations.find(name);
    if (it != mAnimations.end() && it->second.animRef != nullptr) {
        if (it->second.animRef->duration > 0.0f && dt > 0.0f) {
            if (dt > it->second.animRef->duration) {
                dt = it->second.animRef->duration * 0.5f;
            }
            t3d_anim_update(&it->second, dt);
        }
    }
}

void CSkinnedModel::updateAnimations(float dt)
{
    for (auto& [name, anim] : mAnimations) {
        t3d_anim_update(&anim, dt);
    }
}

void CSkinnedModel::blendAnimations(std::string const& animA, std::string const& animB, float blendFactor)
{
    if (!mHasSkeleton || !mHasBlendSkeleton) return;
    t3d_skeleton_blend(&mSkeleton, &mSkeleton, &mSkeletonBlend, blendFactor);
}

void CSkinnedModel::updateSkeleton()
{
    if (mHasSkeleton) {
        t3d_skeleton_update(&mSkeleton);
    }
}

void CSkinnedModel::blendAction(float weight)
{
    if (!mHasSkeleton || !mHasActionSkeleton) return;
    if (weight <= 0.0f) return;
    
    t3d_skeleton_blend(&mSkeleton, &mSkeleton, &mSkeletonAction, weight);
}

float CSkinnedModel::getAnimationFrame(std::string const& name) const
{
    auto it = mAnimations.find(name);
    if (it != mAnimations.end()) {
        return it->second.time * 60.0f;
    }
    return 0.0f;
}

void CSkinnedModel::buildSkinnedDisplayList()
{
    if (mDisplayList) {
        rspq_block_free(mDisplayList);
    }

    rspq_block_begin();
    t3d_matrix_push((const T3DMat4FP*)t3d_segment_placeholder(SKINNED_MODEL_SEGMENT_ID));
    rdpq_set_prim_color(RGBA32(mColor[0], mColor[1], mColor[2], mColor[3]));
    t3d_model_draw_skinned(mModel, &mSkeleton);
    t3d_matrix_pop(1);
    mDisplayList = rspq_block_end();
}

void CSkinnedModel::updateBufferedMatrix(uint32_t frameIndex)
{
    if (!mBufferedMatrices || mNumBuffers == 0) return;
    
    uint32_t bufferIdx = frameIndex % mNumBuffers;
    
    t3d_mat4fp_from_srt_euler(
        &mBufferedMatrices[bufferIdx],
        (float[3]){mScale.x(), mScale.y(), mScale.z()},
        (float[3]){mRotation.x(), -mRotation.y(), mRotation.z()},
        (float[3]){mPosition.x(), mPosition.y(), mPosition.z()}
    );
}

void CSkinnedModel::setBufferedMatrixFromMat4(const T3DMat4* mat)
{
    if (!mBufferedMatrices || mNumBuffers == 0 || !mat) return;
    
    uint32_t bufferIdx = mFrameIndex % mNumBuffers;
    
    t3d_mat4_to_fixed(&mBufferedMatrices[bufferIdx], mat);
}
