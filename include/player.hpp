#pragma once

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include "skinned_model.hpp"
#include "model.hpp"
#include "math.hpp"
#include "viewport.hpp"
#include "collision.hpp"
#include "anim_controller.hpp"
#include "player_state.hpp"
#include "menu.hpp"
#include "textbox.hpp"
#include <vector>

constexpr int FISHING_LINE_SEGMENTS = 4;

struct SItemGetData
{
    const char* name;
    const char* description;
    const char* modelPath;
    EMenuTab inventoryTab;
    int quantity;
    int iconIndex;
    int exp;
    int fishIndex;
};

class CPlayer final
{
public:
    CPlayer() = default;
    ~CPlayer() {
        delete[] mTailYaw; mTailYaw = nullptr;
        delete[] mTailPitch; mTailPitch = nullptr;
        delete[] mTailRoll; mTailRoll = nullptr;
        delete[] mTailYawVel; mTailYawVel = nullptr;
        delete[] mTailPitchVel; mTailPitchVel = nullptr;
        delete[] mTailRollVel; mTailRollVel = nullptr;
    }

    void init(TVec3F const& startPos);
    void update(float dt);
    void setFrameIndex(uint32_t frameIndex);
    void updateBufferedMatrix(uint32_t frameIndex);
    void draw();
    void drawFishingLine(CViewport& viewport, TVec3F const& cameraPos);
    void drawItemGetOverlay(int fontId);
    void drawReelMeter(int fontId);
    
    void applyCollision(CCollisionMesh& collision);

    TVec3F const& getPosition() const { return mPosition; }
    void setPosition(TVec3F const& pos) { mPosition = pos; }
    float getSpeed() const { return mSpeed; }
    float getAnimBlend() const { return mAnimBlend; }
    
    CAnimController& getAnimController() { return mAnimController; }
    CSkinnedModel& getModel() { return mModel; }
    float getPrevWalkFrame() const { return mPrevWalkFrame; }
    void setPrevWalkFrame(float frame) { mPrevWalkFrame = frame; }
    float getPrevRunFrame() const { return mPrevRunFrame; }
    void setPrevRunFrame(float frame) { mPrevRunFrame = frame; }
    void setSpeedMultiplier(float mult) { mSpeedMultiplier = mult; }
    CPlayerStateMachine& getStateMachine() { return mStateMachine; }
    
    float getRotY() const { return mRotY; }
    void setRotY(float rotY) { mRotY = rotY; }
    float getCameraAngle() const { return mCameraAngle; }
    uint16_t getFloorFlags() const { return mFloorFlags; }
    TVec3F getThrowTarget() const { return mThrowTarget; }
    ColFloorResult const& getThrowFloorResult() const { return mThrowFloorResult; }
    bool hasValidThrowTarget() const { return mThrowFloorResult.found; }
    void setCollisionMesh(CCollisionMesh* col) { mCollisionMesh = col; }
    void updateThrowTarget();
    void drawThrowIndicator();
    float getThrowDistance() const { return mThrowDistance; }
    void setThrowDistance(float dist) { mThrowDistance = dist; }
    
    void startBobberThrow();
    void updateBobber(float dt);
    void drawBobber();
    bool isBobberFlying() const { return mBobberFlying; }
    bool hasBobberLanded() const { return mBobberLanded; }
    CSkinnedModel& getFishingRod() { return mFishingRod; }
    
    void equipFishingRod(const char* modelPath);
    bool hasRodEquipped() const { return mRodEquipped; }

    void setBounds(float boxSize) { mBoundsSize = boxSize; }
    void setCameraAngle(float angle) { mCameraAngle = angle; }

    void setInConversation(bool inConv) { mInConversation = inConv; }
    void rotateTowards(const TVec3F& targetPos);
    
    void freezeInput(float duration) { mInputFreezeTimer = duration; }
    
    void triggerItemGet(const SItemGetData& itemData, const std::string& returnState);
    void closeItemGetTextBox();
    const SItemGetData* getCurrentItem() const { return mCurrentItem; }
    const std::string& getItemGetReturnState() const { return mItemGetReturnState; }
    void setItemGetReturnState(const std::string& state) { mItemGetReturnState = state; }
    float getItemGetRotation() const;
    float getItemGetFadeAlpha() const;
    void setCamera(class CCamera* cam) { mCamera = cam; }
    void setMenu(class CMenu* menu) { mMenu = menu; }
    class CCamera* getCamera() { return mCamera; }
    class CMenu* getMenu() { return mMenu; }
    
    void awardExp(int amount);
    void triggerPendingLevelUp();
    void drawExpGainAnimation(int fontId);
    void drawLevelUpAnimation(int fontId);
    bool isExpAnimationActive() const { return mExpAnimTimer > 0.0f; }
    bool isLevelUpAnimationActive() const { return mLevelUpAnimTimer > 0.0f; }

    void playFootstepSound(float prevFrame, float currentFrame, float step1Frame, float step2Frame);

private:
    void handleInput();
    void updateMovement(float dt);
    void updateAnimations(float dt);
    void clampPosition();
    void updateTailPhysics(float dt);
    
    void setSkeletonToIdentity(T3DSkeleton* skel);
    void initializeSkinnedModelBuffers(CSkinnedModel& model);

    CSkinnedModel mModel{};
    CModel mShadow{};
    CSkinnedModel mFishingRod{};
    bool mRodEquipped{false};

    TVec3F mPosition{0.0f, 0.15f, 0.0f};
    TVec3F mMoveDir{0.0f, 0.0f, 0.0f};
    TVec3F mPrevPos{0.0f, 0.15f, 0.0f};
    TVec3F mFloorNormal{0.0f, 1.0f, 0.0f};
    uint16_t mFloorFlags{0};
    
    float mRotY{0.0f};
    float mPrevRotY{0.0f};
    float mTargetRotY{0.0f};
    float mSpeed{0.0f};
    float mAnimBlend{0.0f};
    float mBoundsSize{140.0f};
    float mCameraAngle{0.0f};
    float mTime{0.0f};
    
    uint32_t mCurrentFrameIndex{0};
    
    float mPrevWalkFrame{0.0f};
    float mPrevRunFrame{0.0f};
    float mSpeedMultiplier{1.0f};
    
    bool mIsAttacking{false};
    bool mInConversation{false};
    float mInputFreezeTimer{0.0f};
    
    TVec3F mThrowTarget{0.0f, 0.0f, 0.0f};
    ColFloorResult mThrowFloorResult{};
    CCollisionMesh* mCollisionMesh{nullptr};
    CModel mThrowIndicator{};
    float mThrowDistance{15.0f};
    
    CModel mBobber{};
    int mRodTipBoneIdx{-1};
    TVec3F mBobberPos{0.0f, 0.0f, 0.0f};
    TVec3F mBobberVel{0.0f, 0.0f, 0.0f};
    TVec3F mBobberStart{0.0f, 0.0f, 0.0f};
    TVec3F mBobberEnd{0.0f, 0.0f, 0.0f};
    float mBobberTime{0.0f};
    float mBobberDuration{1.0f};
    bool mBobberFlying{false};
    bool mBobberLanded{false};
    
    CAnimController mAnimController{};
    
    CPlayerStateMachine mStateMachine{};

    int mTailIdx[4] {-1, -1, -1, -1};
    float* mTailYaw = nullptr;
    float* mTailPitch = nullptr;
    float* mTailRoll = nullptr;
    float* mTailYawVel = nullptr;
    float* mTailPitchVel = nullptr;
    float* mTailRollVel = nullptr;
    
    int mChestBoneIdx{-1};
    float mChestTwist{0.0f};
    float mChestTwistVel{0.0f};

    int mHandBoneIdx{-1};
    int mRodRootBoneIdx{-1};
    T3DMat4 mRodWorldMatrix{};
    T3DVec3 mRopePoints[FISHING_LINE_SEGMENTS + 1]{};
    T3DVec3 mRopePrevPoints[FISHING_LINE_SEGMENTS + 1]{};
    bool mRopeInitialized{false};
    
    T3DVertPacked* mLineVerts{nullptr};
    T3DMat4FP* mLineMatFP{nullptr};
    
    const SItemGetData* mCurrentItem{nullptr};
    std::string mItemGetReturnState;
    class CCamera* mCamera{nullptr};
    class CMenu* mMenu{nullptr};
    CModel mItemGetModel{};
    CSkinnedModel mItemGetSkinnedModel{};
    CViewport mItemGetViewport{};
    CTextBox mItemGetTextBox{};
    bool mItemModelLoaded{false};
    bool mItemIsSkinned{false};
    bool mItemTextBoxInitialized{false};
    
    float mExpAnimTimer{0.0f};
    int mExpGainAmount{0};
    float mExpAnimY{0.0f};
    float mExpAnimAlpha{1.0f};
    
    float mLevelUpAnimTimer{0.0f};
    int mNewLevel{0};
    float mLevelUpScale{1.0f};
    float mLevelUpAlpha{1.0f};
    bool mPendingLevelUp{false};
    int mPendingLevel{0};
    
    void updateFishingLine(float dt);
    T3DVec3 getHandWorldPosition();
    T3DVec3 getRodTipWorldPosition();
};
