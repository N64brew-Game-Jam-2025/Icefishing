#pragma once

#include <libdragon.h>
#include <cstdint>

constexpr uint16_t COL_FLAG_WALKABLE = 0x0001;
constexpr uint16_t COL_FLAG_WALL     = 0x0002;
constexpr uint16_t COL_FLAG_CEILING  = 0x0004;
constexpr uint16_t COL_FLAG_TRIGGER  = 0x0008;

constexpr uint16_t COL_FLAG_WATER    = 0x0010;
constexpr uint16_t COL_FLAG_SNOW     = 0x0020;
constexpr uint16_t COL_FLAG_WOOD     = 0x0040;
constexpr uint16_t COL_FLAG_CEMENT   = 0x0080;

constexpr float COL_POSITION_SCALE = 16.0f;
constexpr float COL_NORMAL_SCALE = 127.0f;

constexpr float COL_GRID_CELL_SIZE = 32.0f;

constexpr int COL_MAX_TRIS_PER_CELL = 32;

struct ColTriangle {
    int16_t v0[3];
    int16_t v1[3];
    int16_t v2[3];
    
    int8_t normal[3];
    
    uint8_t material;
    uint16_t flags;
    
    uint8_t reserved[8];
    
    inline float getV0X() const { return v0[0] / COL_POSITION_SCALE; }
    inline float getV0Y() const { return v0[1] / COL_POSITION_SCALE; }
    inline float getV0Z() const { return v0[2] / COL_POSITION_SCALE; }
    inline float getV1X() const { return v1[0] / COL_POSITION_SCALE; }
    inline float getV1Y() const { return v1[1] / COL_POSITION_SCALE; }
    inline float getV1Z() const { return v1[2] / COL_POSITION_SCALE; }
    inline float getV2X() const { return v2[0] / COL_POSITION_SCALE; }
    inline float getV2Y() const { return v2[1] / COL_POSITION_SCALE; }
    inline float getV2Z() const { return v2[2] / COL_POSITION_SCALE; }
    
    inline float getNormalX() const { return normal[0] / COL_NORMAL_SCALE; }
    inline float getNormalY() const { return normal[1] / COL_NORMAL_SCALE; }
    inline float getNormalZ() const { return normal[2] / COL_NORMAL_SCALE; }
    
    inline bool hasFlag(uint16_t flag) const { return (flags & flag) != 0; }
} __attribute__((packed));

struct ColHeader {
    char magic[4];
    uint16_t version;
    uint16_t fileFlags;
    uint32_t triangleCount;
    int16_t aabbMin[3];
    int16_t aabbMax[3];
    uint32_t reserved;
} __attribute__((packed));

struct ColGridCell {
    uint16_t triangleIndices[COL_MAX_TRIS_PER_CELL];
    uint8_t triangleCount;
};

struct ColFloorResult {
    bool found;
    float floorY;
    float normalX, normalY, normalZ;
    const ColTriangle* triangle;
    uint16_t flags;
};

struct ColPushResult {
    bool collided;
    float pushX;
    float pushY;
    float pushZ;
    int hitCount;
    uint16_t flags;
};

class CCollisionMesh {
public:
    CCollisionMesh() = default;
    ~CCollisionMesh();
    
    bool load(const char* path);
    void unload();
    bool isLoaded() const { return mTriangles != nullptr; }
    uint32_t getTriangleCount() const { return mTriangleCount; }
    
    void getAABB(float& minX, float& minY, float& minZ,
                 float& maxX, float& maxY, float& maxZ) const;
    
    ColFloorResult findFloor(float x, float y, float z, float maxDrop = 100.0f) const;
    ColFloorResult findCeiling(float x, float y, float z, float maxHeight = 100.0f) const;
    ColPushResult checkSphere(float x, float y, float z, float radius,
                              uint16_t flagMask = 0) const;
    bool isPointInside(float x, float y, float z) const;
    bool raycast(float ox, float oy, float oz,
                 float dx, float dy, float dz,
                 float maxDist,
                 float* outDist = nullptr,
                 const ColTriangle** outTri = nullptr) const;

    void debugPrint() const;
    
    void getGridInfo(int& outWidth, int& outHeight, float& outOriginX, float& outOriginZ, float& outCellSize) const {
        outWidth = mGridWidth;
        outHeight = mGridHeight;
        outOriginX = mGridOriginX;
        outOriginZ = mGridOriginZ;
        outCellSize = COL_GRID_CELL_SIZE;
    }
    
    int getGridCellTriCount(int cellX, int cellZ) const {
        if (!mGrid || cellX < 0 || cellX >= mGridWidth || cellZ < 0 || cellZ >= mGridHeight)
            return 0;
        return mGrid[cellZ * mGridWidth + cellX].triangleCount;
    }

private:
    void buildGrid();
    int getGridIndex(float x, float z) const;
    void getOverlappingCells(float x, float z, float radius,
                             int* outIndices, int* outCount, int maxCount) const;
    bool rayTriangleIntersect(const ColTriangle& tri,
                              float ox, float oy, float oz,
                              float dx, float dy, float dz,
                              float* outT) const;
    bool sphereTriangleIntersect(const ColTriangle& tri,
                                 float cx, float cy, float cz, float radius,
                                 float* outPushX, float* outPushY, float* outPushZ) const;
    bool pointInTriangleXZ(const ColTriangle& tri, float x, float z) const;
    
    uint8_t* mFileData = nullptr;
    ColTriangle* mTriangles = nullptr;
    uint32_t mTriangleCount = 0;
    
    float mMinX = 0, mMinY = 0, mMinZ = 0;
    float mMaxX = 0, mMaxY = 0, mMaxZ = 0;
    
    ColGridCell* mGrid = nullptr;
    int mGridWidth = 0;
    int mGridHeight = 0;
    float mGridOriginX = 0;
    float mGridOriginZ = 0;
};
