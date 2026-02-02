#pragma once

#include <string>
#include <unordered_map>
#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include "model.hpp"

constexpr int SKINNED_MODEL_SEGMENT_ID = 1;

class CSkinnedModel : public CModel
{
public:
    CSkinnedModel() = default;
    ~CSkinnedModel() override;

    void load(std::string const& path) override;
    void unload();
    void draw() override;
    
    void setFrameIndex(uint32_t frameIndex) { mFrameIndex = frameIndex; }

    void createSkeleton();
    void addAnimation(std::string const& name, int skeletonType = 0);
    void playAnimation(std::string const& name);
    void stopAnimation(std::string const& name);
    void setAnimationSpeed(std::string const& name, float speed);
    void setAnimationLooping(std::string const& name, bool loop);
    void resetAnimation(std::string const& name);
    bool isAnimationFinished(std::string const& name) const;
    bool isAnimationPlaying(std::string const& name) const;
    void updateAnimation(std::string const& name, float dt);
    void updateAnimations(float dt);
    void blendAnimations(std::string const& animA, std::string const& animB, float blendFactor);
    void updateSkeleton();
    
    float getAnimationFrame(std::string const& name) const;

    T3DSkeleton* getSkeleton() { return &mSkeleton; }
    T3DSkeleton* getBlendSkeleton() { return &mSkeletonBlend; }
    T3DSkeleton* getActionSkeleton() { return &mSkeletonAction; }

    void buildSkinnedDisplayList();
    
    void blendAction(float weight);
    
    void updateBufferedMatrix(uint32_t frameIndex);
    
    void setBufferedMatrixFromMat4(const T3DMat4* mat);

private:
    T3DSkeleton mSkeleton{};
    T3DSkeleton mSkeletonBlend{};
    T3DSkeleton mSkeletonAction{};
    std::unordered_map<std::string, T3DAnim> mAnimations{};
    bool mHasSkeleton{false};
    bool mHasBlendSkeleton{false};
    bool mHasActionSkeleton{false};
    
    T3DMat4FP* mBufferedMatrices{nullptr};
    uint32_t mNumBuffers{0};
    uint32_t mFrameIndex{0};
};
