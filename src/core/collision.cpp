#include "../../include/collision.hpp"
#include "debug.h"
#include <cmath>
#include <cstring>

CCollisionMesh::~CCollisionMesh() {
    unload();
}

bool CCollisionMesh::load(const char* path) {

    unload();

    FILE* file = asset_fopen(path, nullptr);
    if (!file) {
        assert(false && "Failed to open collision file");
        return false;
    }

    ColHeader header{};
    fread(&header, sizeof(ColHeader), 1, file);
    
    if (memcmp(header.magic, "COL1", 4) != 0) {
        assert(false && "Invalid collision file magic");
        fclose(file);
        return false;
    }

    mTriangleCount = header.triangleCount;
    
    debugf("ColHeader raw: tris=%lu, min=(%d,%d,%d), max=(%d,%d,%d)\n",
           (unsigned long)header.triangleCount,
           (int)header.aabbMin[0], (int)header.aabbMin[1], (int)header.aabbMin[2],
           (int)header.aabbMax[0], (int)header.aabbMax[1], (int)header.aabbMax[2]);
    
    mMinX = header.aabbMin[0] / COL_POSITION_SCALE;
    mMinY = header.aabbMin[1] / COL_POSITION_SCALE;
    mMinZ = header.aabbMin[2] / COL_POSITION_SCALE;
    mMaxX = header.aabbMax[0] / COL_POSITION_SCALE;
    mMaxY = header.aabbMax[1] / COL_POSITION_SCALE;
    mMaxZ = header.aabbMax[2] / COL_POSITION_SCALE;

    mTriangles = new ColTriangle[mTriangleCount];
    fread(mTriangles, sizeof(ColTriangle), mTriangleCount, file);

    fclose(file);

    buildGrid();

    debugf("Collision loaded: %lu tris, AABB: (%.1f,%.1f,%.1f)-(%.1f,%.1f,%.1f), Grid: %dx%d\n", 
           (unsigned long)mTriangleCount, mMinX, mMinY, mMinZ, mMaxX, mMaxY, mMaxZ,
           mGridWidth, mGridHeight);
    return true;
}

void CCollisionMesh::debugPrint() const {
    debugf("=== Collision Debug ===\n");
    debugf("Triangles: %lu\n", (unsigned long)mTriangleCount);
    debugf("AABB: (%.2f, %.2f, %.2f) - (%.2f, %.2f, %.2f)\n", 
           mMinX, mMinY, mMinZ, mMaxX, mMaxY, mMaxZ);
    debugf("Grid: %d x %d cells, origin: (%.2f, %.2f), cell size: %.2f\n",
           mGridWidth, mGridHeight, mGridOriginX, mGridOriginZ, COL_GRID_CELL_SIZE);
    
    int walkable = 0, walls = 0, ceilings = 0;
    for (uint32_t i = 0; i < mTriangleCount; i++) {
        if (mTriangles[i].flags & COL_FLAG_WALKABLE) walkable++;
        if (mTriangles[i].flags & COL_FLAG_WALL) walls++;
        if (mTriangles[i].flags & COL_FLAG_CEILING) ceilings++;
    }

    debugf("Walkable: %d, Walls: %d, Ceilings: %d\n", walkable, walls, ceilings);
    

    int printCount = mTriangleCount < 5 ? mTriangleCount : 5;
    for (int i = 0; i < printCount; i++) {
        const ColTriangle& t = mTriangles[i];
        if (i == 0) {
            debugf("Tri[0] raw: v0=(%d,%d,%d) n=(%d,%d,%d) flags=%04x\n",
                   (int)t.v0[0], (int)t.v0[1], (int)t.v0[2],
                   (int)t.normal[0], (int)t.normal[1], (int)t.normal[2], t.flags);
        }
        debugf("Tri[%d]: v0=(%.1f,%.1f,%.1f) n=(%.2f,%.2f,%.2f) flags=%04x\n",
               i, t.getV0X(), t.getV0Y(), t.getV0Z(),
               t.getNormalX(), t.getNormalY(), t.getNormalZ(), t.flags);
    }
}

void CCollisionMesh::unload() {
    if (mTriangles) {
        delete[] mTriangles;
        mTriangles = nullptr;
    }
    if (mGrid) {
        delete[] mGrid;
        mGrid = nullptr;
    }
    if (mFileData) {
        free(mFileData);
        mFileData = nullptr;
    }
    mTriangleCount = 0;
}

void CCollisionMesh::getAABB(float& minX, float& minY, float& minZ,
                             float& maxX, float& maxY, float& maxZ) const {
    minX = mMinX; minY = mMinY; minZ = mMinZ;
    maxX = mMaxX; maxY = mMaxY; maxZ = mMaxZ;
}

void CCollisionMesh::buildGrid() {
    
    float worldWidth = mMaxX - mMinX;
    float worldDepth = mMaxZ - mMinZ;
    
    mGridWidth = (int)ceilf(worldWidth / COL_GRID_CELL_SIZE);
    mGridHeight = (int)ceilf(worldDepth / COL_GRID_CELL_SIZE);
    
    if (mGridWidth < 1) mGridWidth = 1;
    if (mGridHeight < 1) mGridHeight = 1;
    
    mGridOriginX = mMinX;
    mGridOriginZ = mMinZ;
    
    int gridSize = mGridWidth * mGridHeight;
    mGrid = new ColGridCell[gridSize];
    
    for (int i = 0; i < gridSize; i++) {
        mGrid[i].triangleCount = 0;
    }
    
    for (uint32_t t = 0; t < mTriangleCount; t++) {
        const ColTriangle& tri = mTriangles[t];
        
        float triMinX = fminf(fminf(tri.getV0X(), tri.getV1X()), tri.getV2X());
        float triMaxX = fmaxf(fmaxf(tri.getV0X(), tri.getV1X()), tri.getV2X());
        float triMinZ = fminf(fminf(tri.getV0Z(), tri.getV1Z()), tri.getV2Z());
        float triMaxZ = fmaxf(fmaxf(tri.getV0Z(), tri.getV1Z()), tri.getV2Z());
        
        int minCellX = (int)((triMinX - mGridOriginX) / COL_GRID_CELL_SIZE);
        int maxCellX = (int)((triMaxX - mGridOriginX) / COL_GRID_CELL_SIZE);
        int minCellZ = (int)((triMinZ - mGridOriginZ) / COL_GRID_CELL_SIZE);
        int maxCellZ = (int)((triMaxZ - mGridOriginZ) / COL_GRID_CELL_SIZE);
        
        if (minCellX < 0) minCellX = 0;
        if (maxCellX >= mGridWidth) maxCellX = mGridWidth - 1;
        if (minCellZ < 0) minCellZ = 0;
        if (maxCellZ >= mGridHeight) maxCellZ = mGridHeight - 1;
        
        for (int z = minCellZ; z <= maxCellZ; z++) {
            for (int x = minCellX; x <= maxCellX; x++) {
                ColGridCell& cell = mGrid[z * mGridWidth + x];
                if (cell.triangleCount < COL_MAX_TRIS_PER_CELL) {
                    cell.triangleIndices[cell.triangleCount++] = (uint16_t)t;
                }
            }
        }
    }
}

int CCollisionMesh::getGridIndex(float x, float z) const {
    int cellX = (int)((x - mGridOriginX) / COL_GRID_CELL_SIZE);
    int cellZ = (int)((z - mGridOriginZ) / COL_GRID_CELL_SIZE);
    
    if (cellX < 0 || cellX >= mGridWidth || cellZ < 0 || cellZ >= mGridHeight) {
        return -1;
    }
    
    return cellZ * mGridWidth + cellX;
}

void CCollisionMesh::getOverlappingCells(float x, float z, float radius,
                                         int* outIndices, int* outCount, int maxCount) const {
    *outCount = 0;
    
    if (x + radius < mGridOriginX || x - radius > mGridOriginX + mGridWidth * COL_GRID_CELL_SIZE ||
        z + radius < mGridOriginZ || z - radius > mGridOriginZ + mGridHeight * COL_GRID_CELL_SIZE) {
        return;
    }
    
    int minCellX = (int)floorf((x - radius - mGridOriginX) / COL_GRID_CELL_SIZE);
    int maxCellX = (int)floorf((x + radius - mGridOriginX) / COL_GRID_CELL_SIZE);
    int minCellZ = (int)floorf((z - radius - mGridOriginZ) / COL_GRID_CELL_SIZE);
    int maxCellZ = (int)floorf((z + radius - mGridOriginZ) / COL_GRID_CELL_SIZE);
    
    if (minCellX < 0) minCellX = 0;
    if (maxCellX >= mGridWidth) maxCellX = mGridWidth - 1;
    if (minCellZ < 0) minCellZ = 0;
    if (maxCellZ >= mGridHeight) maxCellZ = mGridHeight - 1;
    
    for (int cz = minCellZ; cz <= maxCellZ && *outCount < maxCount; cz++) {
        for (int cx = minCellX; cx <= maxCellX && *outCount < maxCount; cx++) {
            outIndices[(*outCount)++] = cz * mGridWidth + cx;
        }
    }
}

bool CCollisionMesh::pointInTriangleXZ(const ColTriangle& tri, float x, float z) const {
    float ax = tri.getV0X(), az = tri.getV0Z();
    float bx = tri.getV1X(), bz = tri.getV1Z();
    float cx = tri.getV2X(), cz = tri.getV2Z();
    
    float v0x = cx - ax, v0z = cz - az;
    float v1x = bx - ax, v1z = bz - az;
    float v2x = x - ax, v2z = z - az;
    
    float dot00 = v0x * v0x + v0z * v0z;
    float dot01 = v0x * v1x + v0z * v1z;
    float dot02 = v0x * v2x + v0z * v2z;
    float dot11 = v1x * v1x + v1z * v1z;
    float dot12 = v1x * v2x + v1z * v2z;
    
    float denom = dot00 * dot11 - dot01 * dot01;
    
    if (fabsf(denom) < 0.0001f) return false;
    
    float invDenom = 1.0f / denom;
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;
    
    constexpr float eps = 0.01f;
    return (u >= -eps) && (v >= -eps) && (u + v <= 1.0f + eps);
}

bool CCollisionMesh::rayTriangleIntersect(const ColTriangle& tri,
                                          float ox, float oy, float oz,
                                          float dx, float dy, float dz,
                                          float* outT) const {
    float v0x = tri.getV0X(), v0y = tri.getV0Y(), v0z = tri.getV0Z();
    float v1x = tri.getV1X(), v1y = tri.getV1Y(), v1z = tri.getV1Z();
    float v2x = tri.getV2X(), v2y = tri.getV2Y(), v2z = tri.getV2Z();
    
    float e1x = v1x - v0x, e1y = v1y - v0y, e1z = v1z - v0z;
    float e2x = v2x - v0x, e2y = v2y - v0y, e2z = v2z - v0z;
    
    float hx = dy * e2z - dz * e2y;
    float hy = dz * e2x - dx * e2z;
    float hz = dx * e2y - dy * e2x;
    
    float a = e1x * hx + e1y * hy + e1z * hz;
    if (fabsf(a) < 0.00001f) return false;
    
    float f = 1.0f / a;
    float sx = ox - v0x, sy = oy - v0y, sz = oz - v0z;
    float u = f * (sx * hx + sy * hy + sz * hz);
    if (u < 0.0f || u > 1.0f) return false;
    
    float qx = sy * e1z - sz * e1y;
    float qy = sz * e1x - sx * e1z;
    float qz = sx * e1y - sy * e1x;
    float v = f * (dx * qx + dy * qy + dz * qz);
    if (v < 0.0f || u + v > 1.0f) return false;
    
    float t = f * (e2x * qx + e2y * qy + e2z * qz);
    if (t > 0.0001f) {
        *outT = t;
        return true;
    }
    
    return false;
}

bool CCollisionMesh::sphereTriangleIntersect(const ColTriangle& tri,
                                             float cx, float cy, float cz, float radius,
                                             float* outPushX, float* outPushY, float* outPushZ) const {
    float v0x = tri.getV0X(), v0y = tri.getV0Y(), v0z = tri.getV0Z();
    float v1x = tri.getV1X(), v1y = tri.getV1Y(), v1z = tri.getV1Z();
    float v2x = tri.getV2X(), v2y = tri.getV2Y(), v2z = tri.getV2Z();
    
    float e1x = v1x - v0x, e1y = v1y - v0y, e1z = v1z - v0z;
    float e2x = v2x - v0x, e2y = v2y - v0y, e2z = v2z - v0z;
    float nx = e1y * e2z - e1z * e2y;
    float ny = e1z * e2x - e1x * e2z;
    float nz = e1x * e2y - e1y * e2x;
    float nlen = sqrtf(nx*nx + ny*ny + nz*nz);
    if (nlen < 0.0001f) return false;
    nx /= nlen; ny /= nlen; nz /= nlen;
    
    float d = nx * v0x + ny * v0y + nz * v0z;
    float dist = nx * cx + ny * cy + nz * cz - d;
    
    if (fabsf(dist) > radius) return false;
    
    float px = cx - dist * nx;
    float py = cy - dist * ny;
    float pz = cz - dist * nz;
    
    float v0px = px - v0x, v0py = py - v0y, v0pz = pz - v0z;
    
    float dot00 = e2x*e2x + e2y*e2y + e2z*e2z;
    float dot01 = e2x*e1x + e2y*e1y + e2z*e1z;
    float dot02 = e2x*v0px + e2y*v0py + e2z*v0pz;
    float dot11 = e1x*e1x + e1y*e1y + e1z*e1z;
    float dot12 = e1x*v0px + e1y*v0py + e1z*v0pz;
    
    float denom = dot00 * dot11 - dot01 * dot01;
    if (fabsf(denom) < 0.0001f) return false;
    
    float invDenom = 1.0f / denom;
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;
    
    bool insideTriangle = (u >= -0.01f) && (v >= -0.01f) && (u + v <= 1.01f);
    
    if (insideTriangle) {
        float pen = radius - fabsf(dist);
        if (dist < 0) {
            *outPushX = -nx * pen;
            *outPushY = -ny * pen;
            *outPushZ = -nz * pen;
        } else {
            *outPushX = nx * pen;
            *outPushY = ny * pen;
            *outPushZ = nz * pen;
        }
        return true;
    }
    
    float closestDist2 = radius * radius + 1.0f;
    float closestX = 0, closestY = 0, closestZ = 0;
    
    auto closestOnSegment = [&](float ax, float ay, float az, float bx, float by, float bz) {
        float abx = bx - ax, aby = by - ay, abz = bz - az;
        float acx = cx - ax, acy = cy - ay, acz = cz - az;
        float t = (abx*acx + aby*acy + abz*acz) / (abx*abx + aby*aby + abz*abz + 0.0001f);
        t = fmaxf(0.0f, fminf(1.0f, t));
        float qx = ax + t * abx, qy = ay + t * aby, qz = az + t * abz;
        float dx = cx - qx, dy = cy - qy, dz = cz - qz;
        float d2 = dx*dx + dy*dy + dz*dz;
        if (d2 < closestDist2) {
            closestDist2 = d2;
            closestX = qx; closestY = qy; closestZ = qz;
        }
    };
    
    closestOnSegment(v0x, v0y, v0z, v1x, v1y, v1z);
    closestOnSegment(v1x, v1y, v1z, v2x, v2y, v2z);
    closestOnSegment(v2x, v2y, v2z, v0x, v0y, v0z);
    
    if (closestDist2 <= radius * radius) {
        float closestDist = sqrtf(closestDist2);
        if (closestDist < 0.0001f) return false;
        float pen = radius - closestDist;
        float dirX = (cx - closestX) / closestDist;
        float dirY = (cy - closestY) / closestDist;
        float dirZ = (cz - closestZ) / closestDist;
        *outPushX = dirX * pen;
        *outPushY = dirY * pen;
        *outPushZ = dirZ * pen;
        return true;
    }
    
    return false;
}

ColFloorResult CCollisionMesh::findFloor(float x, float y, float z, float maxDrop) const {
    ColFloorResult result = { false, y - maxDrop, 0.0f, 1.0f, 0.0f, nullptr, 0 };
    
    if (!mTriangles) return result;
    
    int cellIndices[16];
    int cellCount = 0;
    getOverlappingCells(x, z, 2.0f, cellIndices, &cellCount, 16);
    
    if (cellCount == 0) {
        return result;
    }
    
    float closestY = y - maxDrop;
    
    uint16_t checkedTris[64];
    int checkedCount = 0;
    
    for (int c = 0; c < cellCount; c++) {
        const ColGridCell& cell = mGrid[cellIndices[c]];
        
        for (int i = 0; i < cell.triangleCount; i++) {
            uint16_t triIdx = cell.triangleIndices[i];
            
            bool alreadyChecked = false;
            for (int j = 0; j < checkedCount; j++) {
                if (checkedTris[j] == triIdx) {
                    alreadyChecked = true;
                    break;
                }
            }
            if (alreadyChecked) continue;
            if (checkedCount < 64) checkedTris[checkedCount++] = triIdx;
            
            const ColTriangle& tri = mTriangles[triIdx];
            
            float v0x = tri.getV0X(), v0y = tri.getV0Y(), v0z = tri.getV0Z();
            float v1x = tri.getV1X(), v1y = tri.getV1Y(), v1z = tri.getV1Z();
            float v2x = tri.getV2X(), v2y = tri.getV2Y(), v2z = tri.getV2Z();
            
            float e1x = v1x - v0x, e1y = v1y - v0y, e1z = v1z - v0z;
            float e2x = v2x - v0x, e2y = v2y - v0y, e2z = v2z - v0z;
            float nx = e1y * e2z - e1z * e2y;
            float ny = e1z * e2x - e1x * e2z;
            float nz = e1x * e2y - e1y * e2x;
            
            float len = sqrtf(nx*nx + ny*ny + nz*nz);
            if (len < 0.0001f) continue;
            nx /= len; ny /= len; nz /= len;
            
            bool isWalkable = (tri.flags & COL_FLAG_WALKABLE) != 0;
            if (!isWalkable && ny < 0.3f) continue;
            
            if (!pointInTriangleXZ(tri, x, z)) continue;
            
            float floorY = v0y - (nx * (x - v0x) + nz * (z - v0z)) / ny;
            
            if (floorY <= y + 0.5f && floorY > closestY) {
                closestY = floorY;
                result.found = true;
                result.floorY = floorY;
                result.normalX = nx;
                result.normalY = ny;
                result.normalZ = nz;
                result.triangle = &tri;
                result.flags = tri.flags;
            }
        }
    }
    
    return result;
}

ColFloorResult CCollisionMesh::findCeiling(float x, float y, float z, float maxHeight) const {
    ColFloorResult result = { false, y + maxHeight, 0.0f, -1.0f, 0.0f, nullptr, 0 };
    
    if (!mTriangles) return result;
    
    int gridIdx = getGridIndex(x, z);
    if (gridIdx < 0) return result;
    
    const ColGridCell& cell = mGrid[gridIdx];
    float closestY = y + maxHeight;
    
    for (int i = 0; i < cell.triangleCount; i++) {
        const ColTriangle& tri = mTriangles[cell.triangleIndices[i]];
        
        if (tri.getNormalY() > -0.5f) continue;
        
        if (!pointInTriangleXZ(tri, x, z)) continue;
        
        float nx = tri.getNormalX(), ny = tri.getNormalY(), nz = tri.getNormalZ();
        float v0x = tri.getV0X(), v0y = tri.getV0Y(), v0z = tri.getV0Z();
        
        if (fabsf(ny) < 0.001f) continue;
        
        float d = nx * v0x + ny * v0y + nz * v0z;
        float ceilY = (d - nx * x - nz * z) / ny;
        
        if (ceilY >= y && ceilY < closestY) {
            closestY = ceilY;
            result.found = true;
            result.floorY = ceilY;
            result.triangle = &tri;
            result.flags = tri.flags;
        }
    }
    
    return result;
}

ColPushResult CCollisionMesh::checkSphere(float x, float y, float z, float radius,
                                          uint16_t flagMask) const {
    ColPushResult result = { false, 0, 0, 0, 0, 0 };
    
    if (!mTriangles || !mGrid) return result;
    
    int cellIndices[16];
    int cellCount = 0;
    getOverlappingCells(x, z, radius + 2.0f, cellIndices, &cellCount, 16);
    
    uint16_t checkedTris[64];
    int checkedCount = 0;
    
    for (int c = 0; c < cellCount; c++) {
        const ColGridCell& cell = mGrid[cellIndices[c]];
        
        for (int i = 0; i < cell.triangleCount; i++) {
            uint16_t triIdx = cell.triangleIndices[i];
            
            bool alreadyChecked = false;
            for (int j = 0; j < checkedCount; j++) {
                if (checkedTris[j] == triIdx) {
                    alreadyChecked = true;
                    break;
                }
            }
            if (alreadyChecked) continue;
            if (checkedCount < 64) checkedTris[checkedCount++] = triIdx;
            
            const ColTriangle& tri = mTriangles[triIdx];
            
            if (flagMask != 0 && (tri.flags & flagMask) == 0) continue;
            
            float pushX, pushY, pushZ;
            if (sphereTriangleIntersect(tri, x, y, z, radius, &pushX, &pushY, &pushZ)) {
                result.collided = true;
                result.pushX += pushX;
                result.pushY += pushY;
                result.pushZ += pushZ;
                result.hitCount++;
                result.flags |= tri.flags;
            }
        }
    }
    
    return result;
}

bool CCollisionMesh::isPointInside(float x, float y, float z) const {
    ColFloorResult floor = findFloor(x, y, z, 1000.0f);
    ColFloorResult ceil = findCeiling(x, y, z, 1000.0f);
    return floor.found && ceil.found;
}

bool CCollisionMesh::raycast(float ox, float oy, float oz,
                             float dx, float dy, float dz,
                             float maxDist,
                             float* outDist,
                             const ColTriangle** outTri) const {
    if (!mTriangles) return false;
    
    float len = sqrtf(dx*dx + dy*dy + dz*dz);
    if (len < 0.0001f) return false;
    dx /= len; dy /= len; dz /= len;
    
    float closestT = maxDist;
    const ColTriangle* hitTri = nullptr;
    
    for (uint32_t t = 0; t < mTriangleCount; t++) {
        float tVal;
        if (rayTriangleIntersect(mTriangles[t], ox, oy, oz, dx, dy, dz, &tVal)) {
            if (tVal < closestT) {
                closestT = tVal;
                hitTri = &mTriangles[t];
            }
        }
    }
    
    if (hitTri) {
        if (outDist) *outDist = closestT;
        if (outTri) *outTri = hitTri;
        return true;
    }
    
    return false;
}
