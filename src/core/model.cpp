#include "model.hpp"

CModel::~CModel()
{
    unload();
}

void CModel::unload()
{
    if (mDisplayList) {
        rspq_block_free(mDisplayList);
        mDisplayList = nullptr;
    }
    if (mMatrixFP) {
        free_uncached(mMatrixFP);
        mMatrixFP = nullptr;
    }
    if (mModel) {
        t3d_model_free(mModel);
        mModel = nullptr;
    }
}

void CModel::load(std::string const& path)
{
    unload();
    mModel = t3d_model_load(path.c_str());
    mMatrixFP = static_cast<T3DMat4FP*>(malloc_uncached(sizeof(T3DMat4FP)));
    updateMatrix();
}

void CModel::draw()
{
    if (mDisplayList) {
        rspq_block_run(mDisplayList);
    }
}

void CModel::setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    mColor[0] = r;
    mColor[1] = g;
    mColor[2] = b;
    mColor[3] = a;
}

void CModel::updateMatrix()
{
    if (!mMatrixFP) return;
    
    t3d_mat4fp_from_srt_euler(mMatrixFP,
        (float[3]){mScale.x(), mScale.y(), mScale.z()},
        (float[3]){mRotation.x(), -mRotation.y(), mRotation.z()},
        (float[3]){mPosition.x(), mPosition.y(), mPosition.z()}
    );
    mDirty = false;
}

void CModel::buildDisplayList()
{
    if (mDisplayList) {
        rspq_block_free(mDisplayList);
    }

    rspq_block_begin();
    t3d_matrix_push(mMatrixFP);
    rdpq_set_prim_color(RGBA32(mColor[0], mColor[1], mColor[2], mColor[3]));
    t3d_model_draw(mModel);
    t3d_matrix_pop(1);
    mDisplayList = rspq_block_end();
}
