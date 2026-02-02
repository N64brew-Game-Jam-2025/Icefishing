#pragma once

#include <cstdint>
#include <vector>
#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/tpx.h>
#include "math.hpp"

struct CParticleData
{
    TVec3F position{0.0f, 0.0f, 0.0f};
    TVec3F velocity{0.0f, 0.0f, 0.0f};
    float size{1.0f};
    float life{1.0f};
    float maxLife{1.0f};
    uint8_t color[4]{255, 255, 255, 255};
    bool active{false};
};

class CParticleEmitter
{
public:
    CParticleEmitter() = default;
    virtual ~CParticleEmitter();

    void init(uint32_t maxParticles, int matrixStackSize = 8);
    void destroy();

    virtual void update(float dt);
    virtual void draw();

    int emit(TVec3F const& position, TVec3F const& velocity, 
             float size, float life,
             uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

    void burst(uint32_t count, TVec3F const& position, float spread,
               TVec3F const& minVelocity, TVec3F const& maxVelocity,
               float minSize, float maxSize,
               float minLife, float maxLife,
               uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

    void clear();
    void setScale(float scaleX, float scaleY);
    void setPosition(TVec3F const& pos);
    void setGravity(TVec3F const& gravity) { mGravity = gravity; }
    void setWorldScale(float scale) { mWorldScale = scale; }
    void setFadeOverLife(bool fade) { mFadeOverLife = fade; }
    void setShrinkOverLife(bool shrink) { mShrinkOverLife = shrink; }

    uint32_t getActiveCount() const { return mActiveCount; }
    uint32_t getMaxParticles() const { return mMaxParticles; }
    bool isInitialized() const { return mInitialized; }

protected:
    void syncToBuffer();
    int findFreeSlot();

    virtual void onParticleDeath(uint32_t index) {}

    TPXParticle* mParticleBuffer{nullptr};
    std::vector<CParticleData> mParticles{};
    T3DMat4FP* mMatrixFP{nullptr};

    TVec3F mPosition{0.0f, 0.0f, 0.0f};
    TVec3F mGravity{0.0f, -9.8f, 0.0f};

    uint32_t mMaxParticles{0};
    uint32_t mActiveCount{0};

    float mScaleX{1.0f};
    float mScaleY{1.0f};
    float mWorldScale{1.0f};

    bool mInitialized{false};
    bool mFadeOverLife{true};
    bool mShrinkOverLife{false};
};

class CContinuousEmitter : public CParticleEmitter
{
public:
    CContinuousEmitter() = default;
    ~CContinuousEmitter() override = default;

    void update(float dt) override;
    void setEmissionRate(float particlesPerSecond) { mEmissionRate = particlesPerSecond; }
    void start() { mEmitting = true; }
    void stop() { mEmitting = false; }

    bool isEmitting() const { return mEmitting; }

    void setSpawnVelocity(TVec3F const& min, TVec3F const& max) { mMinVelocity = min; mMaxVelocity = max; }
    void setSpawnSize(float min, float max) { mMinSize = min; mMaxSize = max; }
    void setSpawnLife(float min, float max) { mMinLife = min; mMaxLife = max; }
    void setSpawnColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    void setSpawnSpread(float spread) { mSpawnSpread = spread; }

private:
    float mEmissionRate{10.0f};
    float mEmissionAccumulator{0.0f};
    bool mEmitting{false};

    TVec3F mMinVelocity{-1.0f, 1.0f, -1.0f};
    TVec3F mMaxVelocity{1.0f, 3.0f, 1.0f};
    float mMinSize{1.0f};
    float mMaxSize{5.0f};
    float mMinLife{1.0f};
    float mMaxLife{2.0f};
    float mSpawnSpread{0.0f};
    uint8_t mSpawnColor[4]{255, 255, 255, 255};
};