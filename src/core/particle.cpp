#include "particle.hpp"
#include <cstdlib>
#include <cmath>
#include <libdragon.h>

static bool sTpxInitialized = false;
static int sTpxRefCount = 0;

static float randFloat(float min, float max)
{
    float t = (float)rand() / (float)RAND_MAX;
    return min + t * (max - min);
}

CParticleEmitter::~CParticleEmitter()
{
    destroy();
}

void CParticleEmitter::init(uint32_t maxParticles, int matrixStackSize)
{
    if (mInitialized) {
        destroy();
    }

    mMaxParticles = (maxParticles + 1) & ~1u;

    if (!sTpxInitialized) {
        tpx_init({.matrixStackSize = matrixStackSize});
        sTpxInitialized = true;
    }
    ++sTpxRefCount;

    mParticleBuffer = static_cast<TPXParticle*>(
        malloc_uncached(sizeof(TPXParticle) * (mMaxParticles / 2))
    );

    mMatrixFP = static_cast<T3DMat4FP*>(malloc_uncached(sizeof(T3DMat4FP)));
    t3d_mat4fp_identity(mMatrixFP);

    mParticles.resize(mMaxParticles);
    for (auto& p : mParticles) {
        p.active = false;
    }

    for (uint32_t i = 0; i < mMaxParticles / 2; ++i) {
        mParticleBuffer[i] = {};
    }

    mActiveCount = 0;
    mInitialized = true;
}

void CParticleEmitter::destroy()
{
    if (!mInitialized) return;

    if (mParticleBuffer) {
        free_uncached(mParticleBuffer);
        mParticleBuffer = nullptr;
    }

    if (mMatrixFP) {
        free_uncached(mMatrixFP);
        mMatrixFP = nullptr;
    }

    mParticles.clear();

    --sTpxRefCount;
    if (sTpxRefCount <= 0 && sTpxInitialized) {
        tpx_destroy();
        sTpxInitialized = false;
        sTpxRefCount = 0;
    }

    mInitialized = false;
    mActiveCount = 0;
}

void CParticleEmitter::update(float dt)
{
    if (!mInitialized) return;

    mActiveCount = 0;

    for (uint32_t i = 0; i < mMaxParticles; ++i) {
        CParticleData& p = mParticles[i];
        if (!p.active) continue;

        p.life -= dt;
        if (p.life <= 0.0f) {
            p.active = false;
            onParticleDeath(i);
            continue;
        }

        p.velocity.x() += mGravity.x() * dt;
        p.velocity.y() += mGravity.y() * dt;
        p.velocity.z() += mGravity.z() * dt;

        p.position.x() += p.velocity.x() * dt;
        p.position.y() += p.velocity.y() * dt;
        p.position.z() += p.velocity.z() * dt;

        float lifeRatio = p.life / p.maxLife;

        if (mFadeOverLife) {
            p.color[3] = static_cast<uint8_t>(255.0f * lifeRatio);
        }

        if (mShrinkOverLife) {
            p.size = p.size * lifeRatio;
        }

        ++mActiveCount;
    }

    syncToBuffer();
}

void CParticleEmitter::draw()
{
    if (!mInitialized || mActiveCount == 0) return;

    rdpq_sync_pipe();
    rdpq_sync_tile();
    rdpq_set_mode_standard();
    rdpq_mode_zbuf(true, true);
    rdpq_mode_zoverride(true, 0, 0);
    rdpq_mode_combiner(RDPQ_COMBINER1((PRIM,0,ENV,0), (0,0,0,PRIM)));
    rdpq_set_env_color(RGBA32(255, 255, 255, 255));

    tpx_state_from_t3d();

    tpx_matrix_push(mMatrixFP);

    tpx_state_set_scale(mScaleX, mScaleY);

    uint32_t drawCount = (mActiveCount + 1) & ~1u;
    if (drawCount > 0) {
        tpx_particle_draw(mParticleBuffer, drawCount);
    }

    tpx_matrix_pop(1);
}

int CParticleEmitter::emit(TVec3F const& position, TVec3F const& velocity,
                           float size, float life,
                           uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    if (!mInitialized) return -1;

    int idx = findFreeSlot();
    if (idx < 0) return -1;

    CParticleData& p = mParticles[idx];
    p.position = position;
    p.velocity = velocity;
    p.size = size;
    p.life = life;
    p.maxLife = life;
    p.color[0] = r;
    p.color[1] = g;
    p.color[2] = b;
    p.color[3] = a;
    p.active = true;

    ++mActiveCount;
    return idx;
}

void CParticleEmitter::burst(uint32_t count, TVec3F const& position, float spread,
                             TVec3F const& minVelocity, TVec3F const& maxVelocity,
                             float minSize, float maxSize,
                             float minLife, float maxLife,
                             uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    for (uint32_t i = 0; i < count; ++i) {
        TVec3F pos = {
            position.x() + randFloat(-spread, spread),
            position.y() + randFloat(-spread, spread),
            position.z() + randFloat(-spread, spread)
        };

        TVec3F vel = {
            randFloat(minVelocity.x(), maxVelocity.x()),
            randFloat(minVelocity.y(), maxVelocity.y()),
            randFloat(minVelocity.z(), maxVelocity.z())
        };

        float size = randFloat(minSize, maxSize);
        float life = randFloat(minLife, maxLife);

        emit(pos, vel, size, life, r, g, b, a);
    }
}

void CParticleEmitter::clear()
{
    for (auto& p : mParticles) {
        p.active = false;
    }
    mActiveCount = 0;
}

void CParticleEmitter::setScale(float scaleX, float scaleY)
{
    mScaleX = scaleX;
    mScaleY = scaleY;
}

void CParticleEmitter::setPosition(TVec3F const& pos)
{
    mPosition = pos;
    if (mMatrixFP) {
        t3d_mat4fp_from_srt_euler(mMatrixFP,
            (float[3]){mWorldScale, mWorldScale, mWorldScale},
            (float[3]){0.0f, 0.0f, 0.0f},
            (float[3]){pos.x(), pos.y(), pos.z()}
        );
    }
}

void CParticleEmitter::syncToBuffer()
{
    if (!mParticleBuffer) return;

    uint32_t bufferIdx = 0;
    for (uint32_t i = 0; i < mMaxParticles && bufferIdx < mActiveCount; ++i) {
        CParticleData& p = mParticles[i];
        if (!p.active) continue;

        int8_t* posPtr = tpx_buffer_get_pos(mParticleBuffer, bufferIdx);
        int8_t* sizePtr = tpx_buffer_get_size(mParticleBuffer, bufferIdx);
        uint8_t* colorPtr = tpx_buffer_get_rgba(mParticleBuffer, bufferIdx);

        float relX = p.position.x() - mPosition.x();
        float relY = p.position.y() - mPosition.y();
        float relZ = p.position.z() - mPosition.z();

        posPtr[0] = static_cast<int8_t>(TMath<float>::clamp(relX, -127.0f, 127.0f));
        posPtr[1] = static_cast<int8_t>(TMath<float>::clamp(relY, -127.0f, 127.0f));
        posPtr[2] = static_cast<int8_t>(TMath<float>::clamp(relZ, -127.0f, 127.0f));

        *sizePtr = static_cast<int8_t>(TMath<float>::clamp(p.size, 1.0f, 127.0f));

        colorPtr[0] = p.color[0];
        colorPtr[1] = p.color[1];
        colorPtr[2] = p.color[2];
        colorPtr[3] = p.color[3];

        ++bufferIdx;
    }

    for (uint32_t i = bufferIdx; i < mMaxParticles; ++i) {
        int8_t* sizePtr = tpx_buffer_get_size(mParticleBuffer, i);
        *sizePtr = 0;
    }
}

int CParticleEmitter::findFreeSlot()
{
    for (uint32_t i = 0; i < mMaxParticles; ++i) {
        if (!mParticles[i].active) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void CContinuousEmitter::update(float dt)
{
    if (mEmitting) {
        mEmissionAccumulator += mEmissionRate * dt;

        while (mEmissionAccumulator >= 1.0f) {
            TVec3F pos = {
                mPosition.x() + randFloat(-mSpawnSpread, mSpawnSpread),
                mPosition.y() + randFloat(-mSpawnSpread, mSpawnSpread),
                mPosition.z() + randFloat(-mSpawnSpread, mSpawnSpread)
            };

            TVec3F vel = {
                randFloat(mMinVelocity.x(), mMaxVelocity.x()),
                randFloat(mMinVelocity.y(), mMaxVelocity.y()),
                randFloat(mMinVelocity.z(), mMaxVelocity.z())
            };

            float size = randFloat(mMinSize, mMaxSize);
            float life = randFloat(mMinLife, mMaxLife);

            emit(pos, vel, size, life, 
                 mSpawnColor[0], mSpawnColor[1], mSpawnColor[2], mSpawnColor[3]);

            mEmissionAccumulator -= 1.0f;
        }
    }

    CParticleEmitter::update(dt);
}

void CContinuousEmitter::setSpawnColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    mSpawnColor[0] = r;
    mSpawnColor[1] = g;
    mSpawnColor[2] = b;
    mSpawnColor[3] = a;
}