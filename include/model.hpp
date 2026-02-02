#pragma once

#include <string>
#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>
#include "math.hpp"

class CModel
{
public:
    CModel() = default;
    virtual ~CModel();

    virtual void load(std::string const& path);
    virtual void unload(); 
    virtual void draw();
    
    void setPosition(TVec3F const& pos) { mPosition = pos; mDirty = true; }
    void setRotation(TVec3F const& rot) { mRotation = rot; mDirty = true; }
    void setScale(TVec3F const& scale) { mScale = scale; mDirty = true; }
    void setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    TVec3F const& getPosition() const { return mPosition; }
    TVec3F const& getRotation() const { return mRotation; }
    TVec3F const& getScale() const { return mScale; }
    
    T3DModel* getModel() { return mModel; }
    T3DMat4FP* getMatrix() { return mMatrixFP; }

    void updateMatrix();
    void buildDisplayList();

protected:
    T3DModel* mModel{nullptr};
    T3DMat4FP* mMatrixFP{nullptr};
    rspq_block_t* mDisplayList{nullptr};

    TVec3F mPosition{0.0f, 0.0f, 0.0f};
    TVec3F mRotation{0.0f, 0.0f, 0.0f};
    TVec3F mScale{1.0f, 1.0f, 1.0f};
    
    uint8_t mColor[4]{255, 255, 255, 255};
    bool mDirty{true};
};
