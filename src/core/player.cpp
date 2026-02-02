#include "player.hpp"
#include "menu.hpp"
#include "sound.hpp"
#include "util.hpp"
#include "rdpq_mode.h"
#include <cmath>
#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dskeleton.h>

rspq_block_t *dplDraw = NULL;

void CPlayer::init(TVec3F const& startPos)
{
	mPosition = startPos;
	mPrevPos = startPos;
	if (!mTailYaw) {
		mTailYaw = new float[4];
		mTailPitch = new float[4];
		mTailRoll = new float[4];
		mTailYawVel = new float[4];
		mTailPitchVel = new float[4];
		mTailRollVel = new float[4];
		for (int i = 0; i < 4; ++i) {
			mTailYaw[i] = 0.0f;
			mTailPitch[i] = 0.0f;
			mTailRoll[i] = 0.0f;
			mTailYawVel[i] = 0.0f;
			mTailPitchVel[i] = 0.0f;
			mTailRollVel[i] = 0.0f;
		}
	}
	for (int i = 0; i < 4; ++i) {
		mTailIdx[i] = -1;
	}
	
	mModel.load("rom:/snep.t3dm");
	mModel.setScale({0.1f, 0.1f, 0.1f});
	mModel.setPosition(mPosition);
	mModel.createSkeleton();
	
	mModel.addAnimation("idle", 0);
	mModel.addAnimation("walk", 1);
	mModel.addAnimation("run", 2);
	mModel.addAnimation("prep", 2);
	mModel.addAnimation("throw", 2);
	mModel.addAnimation("hold", 2);
	mModel.addAnimation("reel", 2);
	
	mAnimController.init(&mModel);
	mAnimController.setBaseAnimation("idle", {.loops = true, .speed = 1.0f});
	mAnimController.setMovementAnimation("walk", {.loops = true, .speed = 1.0f});
	mAnimController.registerAction("run", {.loops = true, .speed = 1.3f});
	mAnimController.registerAction("prep", {.loops = false, .speed = 1.4f}); 
	mAnimController.registerAction("throw", {.loops = false, .speed = 1.0f});
	mAnimController.registerAction("hold", {.loops = true, .speed = 1.5f});
	mAnimController.registerAction("reel", {.loops = true, .speed = 1.0f});
	
	mStateMachine.init(this, "idle"); 
	
	mModel.buildSkinnedDisplayList();
	
	mShadow.load("rom:/shadow.t3dm");
	mShadow.setColor(0, 0, 0, 120);
	mShadow.setScale({0.125f, 0.125f, 0.125f});
	mShadow.setPosition(mPosition);
	mShadow.buildDisplayList();

	mThrowIndicator.load("rom:/shadow.t3dm");
	mThrowIndicator.setColor(100, 200, 255, 180);
	mThrowIndicator.setScale({0.08f, 0.08f, 0.08f});
	mThrowIndicator.buildDisplayList();

	mRodEquipped = false;
	mRodRootBoneIdx = -1;
	mRodTipBoneIdx = -1;
	
	mBobber.load("rom:/bobber.t3dm");
	mBobber.setScale({0.1f, 0.1f, 0.1f});
	mBobber.buildDisplayList();

	if (auto skel = mModel.getSkeleton()) {
		mTailIdx[0] = t3d_skeleton_find_bone(skel, "tail0");
		mTailIdx[1] = t3d_skeleton_find_bone(skel, "tail1");
		mTailIdx[2] = t3d_skeleton_find_bone(skel, "tail2");
		
		mHandBoneIdx = t3d_skeleton_find_bone(skel, "handR");
		
		mChestBoneIdx = t3d_skeleton_find_bone(skel, "chest");
	}
	
	int numVerts = FISHING_LINE_SEGMENTS + 1;
	int numPacked = (numVerts + 1) / 2;
	mLineVerts = (T3DVertPacked*)malloc_uncached(sizeof(T3DVertPacked) * numPacked * 2);
	mLineMatFP = (T3DMat4FP*)malloc_uncached(sizeof(T3DMat4FP));
	
	T3DMat4 identityMat;
	t3d_mat4_identity(&identityMat);
	t3d_mat4_to_fixed(mLineMatFP, &identityMat);
}

void CPlayer::update(float dt)
{
	mTime += dt;
	
	if (mInputFreezeTimer > 0.0f) {
		mInputFreezeTimer -= dt;
		mSpeed = 0.0f;
	} else if (!mInConversation) {
		handleInput();
	} else {
		mRotY = MathUtil::lerpAngle(mRotY, mTargetRotY, 5.0f * dt);
		mSpeed = 0.0f;
	}
	updateMovement(dt);
	updateAnimations(dt);
	//clampPosition();
	
	if (mStateMachine.isInState("item_get") && mItemTextBoxInitialized) {
		mItemGetTextBox.update(dt);
	}
	
	if (mExpAnimTimer > 0.0f) {
		mExpAnimTimer -= dt;
		mExpAnimY += dt * 30.0f;
		if (mExpAnimTimer < 0.5f) {
			mExpAnimAlpha = mExpAnimTimer / 0.5f;
		}
	}
	
	if (mLevelUpAnimTimer > 0.0f) {
		mLevelUpAnimTimer -= dt;
		
		if (mLevelUpAnimTimer < 0.8f) {
			mLevelUpAlpha = mLevelUpAnimTimer / 0.8f;
		}
	}
	
	mModel.setPosition(mPosition);
	mModel.setRotation({0.0f, mRotY, 0.0f});
	mModel.updateMatrix();
	
	mShadow.setPosition(mPosition);
	mShadow.setRotation({0.0f, mRotY, 0.0f});
	mShadow.updateMatrix();

	if (mItemModelLoaded && mItemIsSkinned) {
		float itemRot = getItemGetRotation();
		mItemGetSkinnedModel.setPosition({0.0f, 0.0f, 0.0f});
		mItemGetSkinnedModel.setRotation({0.0f, itemRot, 0.0f});
		mItemGetSkinnedModel.updateMatrix();
		
		setSkeletonToIdentity(mItemGetSkinnedModel.getSkeleton());
		mItemGetSkinnedModel.updateSkeleton();
	}

	if (mRodEquipped && mHandBoneIdx >= 0 && mRodRootBoneIdx >= 0) {
		T3DSkeleton* playerSkel = mModel.getSkeleton();
		T3DSkeleton* rodSkel = mFishingRod.getSkeleton();
		
		if (playerSkel && rodSkel) {
			T3DMat4& handMatrix = playerSkel->bones[mHandBoneIdx].matrix;
			
			T3DMat4 playerWorldMat;
			float scale[3] = {0.1f, 0.1f, 0.1f};
			float rot[3] = {0.0f, -mRotY, 0.0f};
			float pos[3] = {mPosition.x(), mPosition.y(), mPosition.z()};
			t3d_mat4_from_srt_euler(&playerWorldMat, scale, rot, pos);
			
			T3DMat4 handWorldMat;
			t3d_mat4_mul(&handWorldMat, &playerWorldMat, &handMatrix);
			
			T3DQuat offsetQuat;
			float offsetEuler[3] = {0.0f, 0.5f, 2.4f};
			t3d_quat_from_euler(&offsetQuat, offsetEuler);
			float offsetScale[3] = {1.50f, 1.50f, 1.50f};
			float offsetPos[3] = {-4.0f, 50.0f, 0.0f};
			T3DMat4 localOffset;
			t3d_mat4_from_srt(&localOffset, offsetScale, offsetQuat.v, offsetPos);

			T3DMat4 finalTransform;
			t3d_mat4_mul(&finalTransform, &handWorldMat, &localOffset);
			
			mRodWorldMatrix = finalTransform;

			T3DMat4 identity;
			t3d_mat4_identity(&identity);
			rodSkel->bones[mRodRootBoneIdx].matrix = identity;
			rodSkel->bones[mRodRootBoneIdx].hasChanged = 1;

			mFishingRod.setBufferedMatrixFromMat4(&finalTransform);

			mFishingRod.updateSkeleton();
		}
	}

	updateFishingLine(dt);

	mPrevPos = mPosition;
	mPrevRotY = mRotY;
}

void CPlayer::rotateTowards(const TVec3F& targetPos)
{
	float dx = targetPos.x() - mPosition.x();
	float dz = targetPos.z() - mPosition.z();
	mTargetRotY = atan2f(dx, dz);
}

void CPlayer::triggerItemGet(const SItemGetData& itemData, const std::string& returnState)
{
	mCurrentItem = &itemData;
	mItemGetReturnState = returnState;
	
	if (itemData.modelPath && itemData.modelPath[0] != '\0') {
		const char* path = itemData.modelPath;
		bool isSkinned = itemData.inventoryTab == EMenuTab::FishingRods;
		
		if (isSkinned) {
			mItemGetSkinnedModel.load(path);
			mItemGetSkinnedModel.setScale({0.05f, 0.05f, 0.05f});
			mItemGetSkinnedModel.setPosition({0.0f, 0.0f, 0.0f});
			mItemGetSkinnedModel.setRotation({0.0f, 0.0f, 0.0f});
			mItemGetSkinnedModel.createSkeleton();
			
			setSkeletonToIdentity(mItemGetSkinnedModel.getSkeleton());
			mItemGetSkinnedModel.updateSkeleton();
			
			initializeSkinnedModelBuffers(mItemGetSkinnedModel);
			
			mItemGetSkinnedModel.buildSkinnedDisplayList();
			mItemIsSkinned = true;
		} else {
			mItemGetModel.load(path);
			mItemGetModel.setScale({0.15f, 0.15f, 0.15f});
			mItemGetModel.buildDisplayList();
			mItemIsSkinned = false;
		}
		
		mItemModelLoaded = true;
		
		mItemGetViewport.init();
		mItemGetViewport.setProjection(45.0f, 5.0f, 100.0f);
	} else {
		mItemModelLoaded = false;
	}
	
	if (!mItemTextBoxInitialized) {
		mItemGetTextBox.init(1, 20, 160, 216, 60);
		mItemGetTextBox.setAnimation(ETextBoxAnim::ScaleCenter, 0.2f);
		mItemGetTextBox.setBackgroundGradient(0x1A, 0x3A, 0x5C, 0xE0, 0x0F, 0x1E, 0x3C, 0xE0);
		mItemGetTextBox.setBorderColor(0xAA, 0xDD, 0xFF, 0xFF);
		mItemGetTextBox.setPadding(8);
		mItemGetTextBox.setSpeed(40.0f);
		mItemTextBoxInitialized = true;
	}
	
	char itemText[256];
	snprintf(itemText, sizeof(itemText), "%s\n%s", itemData.name, itemData.description);
	mItemGetTextBox.clear();
	mItemGetTextBox.setText(itemText);
	
	mStateMachine.transitionTo(this, "item_get");
}

void CPlayer::equipFishingRod(const char* modelPath)
{
	if (!modelPath || modelPath[0] == '\0') return;
	
	mFishingRod.unload();
	
	mFishingRod.load(modelPath);
	mFishingRod.setScale({0.125f, 0.125f, 0.125f});
	mFishingRod.createSkeleton();
	mFishingRod.buildSkinnedDisplayList();
	
	if (auto rodSkel = mFishingRod.getSkeleton()) {
		mRodRootBoneIdx = t3d_skeleton_find_bone(rodSkel, "Root");
		mRodTipBoneIdx = t3d_skeleton_find_bone(rodSkel, "Pole_3");
	}
	
	mRodEquipped = true;
}

void CPlayer::applyCollision(CCollisionMesh& collision)
{
	if (!collision.isLoaded()) return;
	
	mCollisionMesh = &collision;
	
	ColFloorResult floor = collision.findFloor(mPosition.x(), mPosition.y() + 5.0f, mPosition.z(), 50.0f);
	if (floor.found) {
		bool isWalkable = (floor.flags & COL_FLAG_WALKABLE) != 0 || floor.normalY >= 0.5f;
		
		if (isWalkable) {
			mFloorNormal = {floor.normalX, floor.normalY, floor.normalZ};
			mFloorFlags = floor.flags;
			
			float targetY = floor.floorY;
			float currentY = mPosition.y();
			float diff = targetY - currentY;
			
			if (fabsf(diff) < 0.5f) {
				mPosition = {mPosition.x(), targetY, mPosition.z()};
			} else if (diff < 0) {
				mPosition = {mPosition.x(), currentY + diff * 0.3f, mPosition.z()};
			} else {
				mPosition = {mPosition.x(), targetY, mPosition.z()};
			}
		} else {
			float slideForce = (1.0f - floor.normalY) * 0.5f;
			mPosition.x() += floor.normalX * slideForce;
			mPosition.z() += floor.normalZ * slideForce;
		}
	}
	
	constexpr float PLAYER_RADIUS = 4.0f;
	constexpr int MAX_ITERATIONS = 3;
	constexpr float STEP_HEIGHT = 5.0f;
	
	for (int iter = 0; iter < MAX_ITERATIONS; iter++) {
		ColPushResult pushLow = collision.checkSphere(
			mPosition.x(), mPosition.y() + 2.0f, mPosition.z(), 
			PLAYER_RADIUS, COL_FLAG_WALL
		);
		
		ColPushResult pushMid = collision.checkSphere(
			mPosition.x(), mPosition.y() + STEP_HEIGHT + 2.0f, mPosition.z(), 
			PLAYER_RADIUS, COL_FLAG_WALL
		);
		
		ColPushResult pushHigh = collision.checkSphere(
			mPosition.x(), mPosition.y() + STEP_HEIGHT + 6.0f, mPosition.z(), 
			PLAYER_RADIUS, COL_FLAG_WALL
		);
		
		if (pushLow.collided && !pushMid.collided && !pushHigh.collided) {
			float probeX = mPosition.x() + mMoveDir.x() * PLAYER_RADIUS;
			float probeZ = mPosition.z() + mMoveDir.z() * PLAYER_RADIUS;
			
			ColFloorResult stepFloor = collision.findFloor(
				probeX, mPosition.y() + STEP_HEIGHT + 5.0f, probeZ, 
				STEP_HEIGHT + 5.0f
			);
			
			if (stepFloor.found && stepFloor.floorY > mPosition.y() && 
			    stepFloor.floorY - mPosition.y() <= STEP_HEIGHT) {
				mPosition = {mPosition.x(), stepFloor.floorY, mPosition.z()};
				break;
			}
		}
		
		float pushX = (fabsf(pushMid.pushX) > fabsf(pushHigh.pushX)) ? pushMid.pushX : pushHigh.pushX;
		float pushZ = (fabsf(pushMid.pushZ) > fabsf(pushHigh.pushZ)) ? pushMid.pushZ : pushHigh.pushZ;
		
		if (!pushMid.collided && !pushHigh.collided) break;
		
		mPosition = {
			mPosition.x() + pushX,
			mPosition.y(),
			mPosition.z() + pushZ
		};
	}
	
	mModel.setPosition(mPosition);

	float shadowPitch = -asinf(mFloorNormal.z());
	float shadowRoll = asinf(mFloorNormal.x());
	
	float shadowOffset = 0.15f;
	float shadowX = mPosition.x() + mFloorNormal.x() * shadowOffset;
	float shadowY = (floor.found ? floor.floorY : mPosition.y()) + mFloorNormal.y() * shadowOffset;
	float shadowZ = mPosition.z() + mFloorNormal.z() * shadowOffset;
	
	mShadow.setPosition({shadowX, shadowY, shadowZ});
	mShadow.setRotation({shadowPitch, 0.0f, shadowRoll});
	mShadow.updateMatrix();
}

void CPlayer::playFootstepSound(float prevFrame, float currentFrame, float step1Frame, float step2Frame)
{
	const char* step1 = "sstep1";
	const char* step2 = "sstep2";
	
	if (mFloorFlags & COL_FLAG_WOOD) {
		step1 = "wstep1";
		step2 = "wstep2";
	} else if (mFloorFlags & COL_FLAG_CEMENT) {
		step1 = "rstep1";
		step2 = "rstep2";
	}
	
	if (prevFrame < step1Frame && currentFrame >= step1Frame) {
		CSoundMgr::play(step1);
	}
	else if (prevFrame < step2Frame && currentFrame >= step2Frame) {
		CSoundMgr::play(step2);
	}
}

void CPlayer::setSkeletonToIdentity(T3DSkeleton* skel)
{
	if (!skel || !skel->skeletonRef) return;
	
	T3DMat4 identity;
	t3d_mat4_identity(&identity);
	for (uint32_t i = 0; i < skel->skeletonRef->boneCount; ++i) {
		skel->bones[i].matrix = identity;
		skel->bones[i].hasChanged = 1;
	}
}

void CPlayer::initializeSkinnedModelBuffers(CSkinnedModel& model)
{
	model.setFrameIndex(0);
	model.updateBufferedMatrix(0);
	model.setFrameIndex(1);
	model.updateBufferedMatrix(1);
}

void CPlayer::draw()
{
	mModel.draw();
	
	if (mRodEquipped) {
		mFishingRod.draw();
	}
	
	mShadow.draw();
	
	if (mStateMachine.isInState("prep")) {
		drawThrowIndicator();
	}
	
	if (mStateMachine.isInState("throw") || mStateMachine.isInState("hold") || mStateMachine.isInState("reel")) {
		drawBobber();
	}
}

void CPlayer::updateBufferedMatrix(uint32_t frameIndex)
{
	mModel.updateBufferedMatrix(frameIndex);
	
	if (mItemModelLoaded && mItemIsSkinned) {
		mItemGetSkinnedModel.updateBufferedMatrix(frameIndex);
	}
}

void CPlayer::setFrameIndex(uint32_t frameIndex)
{
	mCurrentFrameIndex = frameIndex;
	
	mModel.setFrameIndex(frameIndex);
	
	if (mRodEquipped) {
		mFishingRod.setFrameIndex(frameIndex);
	}
	
	if (mItemModelLoaded && mItemIsSkinned) {
		mItemGetSkinnedModel.setFrameIndex(frameIndex);
	}
}

void CPlayer::handleInput()
{
	if (mStateMachine.isInState("prep") || mStateMachine.isInState("throw") || 
	    mStateMachine.isInState("hold") || mStateMachine.isInState("reel") || 
	    mStateMachine.isInState("item_get")) {
		mSpeed = 0.0f;
		return;
	}
	
	joypad_inputs_t joypad = joypad_get_inputs(JOYPAD_PORT_1);
	
	float stickX = (float)joypad.stick_x * 0.05f;
	float stickY = (float)joypad.stick_y * 0.05f;
	
	float speed = sqrtf(stickX * stickX + stickY * stickY);
	
	if (speed > 0.15f && !mIsAttacking) {
		stickX /= speed;
		stickY /= speed;
		
		float sinAngle = sinf(mCameraAngle);
		float cosAngle = cosf(mCameraAngle);
		
		float worldX = -stickY * sinAngle + stickX * cosAngle;
		float worldZ = -stickY * cosAngle - stickX * sinAngle;
		
		mMoveDir = {worldX, 0.0f, worldZ};
		
		float newAngle = atan2f(mMoveDir.x(), mMoveDir.z());
		mRotY = MathUtil::lerpAngle(mRotY, newAngle, 0.25f);
		mSpeed = MathUtil::lerp(mSpeed, speed * 0.15f, 0.15f);
	} else {
		mSpeed *= 0.8f;
	}
}

void CPlayer::updateMovement(float dt)
{
	(void)dt;
	mPosition.x() += mMoveDir.x() * mSpeed * mSpeedMultiplier;
	mPosition.z() += mMoveDir.z() * mSpeed * mSpeedMultiplier;
}

void CPlayer::updateAnimations(float dt)
{
	mStateMachine.update(this, dt);
	
	mAnimController.update(dt);
	
	updateTailPhysics(dt);
	
	mModel.updateSkeleton();
}

void CPlayer::updateTailPhysics(float dt)
{
	T3DSkeleton* skel = mModel.getSkeleton();
	if (!skel || !skel->skeletonRef) return;

	if (!(dt > 0.0f) || dt > 0.5f) dt = 0.016f;

	float rotDelta = mRotY - mPrevRotY;
	while (rotDelta > T3D_PI) rotDelta -= 2.0f * T3D_PI;
	while (rotDelta < -T3D_PI) rotDelta += 2.0f * T3D_PI;
	float rotVel = rotDelta / dt;

	TVec3F velWorld = mPosition - mPrevPos;
	float worldSpeed = sqrtf(velWorld.x()*velWorld.x() + velWorld.z()*velWorld.z());

	float turnSwing = -rotVel * 0.25f;

	float moveDirX = 0.0f, moveDirZ = 0.0f;
	if (worldSpeed > 1e-4f) {
		moveDirX = velWorld.x() / worldSpeed;
		moveDirZ = velWorld.z() / worldSpeed;
	}
	float facingX = sinf(mRotY);
	float facingZ = cosf(mRotY);
	float lateralMove = 0.0f;
	if (worldSpeed > 1e-4f) {
		lateralMove = -(facingX * moveDirZ - facingZ * moveDirX);
	}
	float speedFactor = fminf(1.0f, worldSpeed * 40.0f);
	float moveSwing = lateralMove * 0.6f * speedFactor;

	float desiredYaw = turnSwing + moveSwing;
	
	float desiredPitch = -worldSpeed * 3.0f;
	
	float desiredRoll = 0.0f;

	const float stiffness = 45.0f;
	const float damping   = 3.0f;
	const float follow    = 1.2f;

	float targetYaw = desiredYaw;
	float targetPitch = desiredPitch;
	float targetRoll = desiredRoll;

	for (int i = 0; i < 3; ++i) {
		int idx = mTailIdx[i];
		if (idx < 0 || idx >= (int)skel->skeletonRef->boneCount) continue;

		float errY = targetYaw - mTailYaw[i];
		float errP = targetPitch - mTailPitch[i];
		float errR = targetRoll - mTailRoll[i];
		mTailYawVel[i]   += errY * stiffness * dt;
		mTailPitchVel[i] += errP * stiffness * dt;
		mTailRollVel[i]  += errR * stiffness * dt;

		float dampFactor = 1.0f / (1.0f + damping * dt);
		mTailYawVel[i]   *= dampFactor;
		mTailPitchVel[i] *= dampFactor;
		mTailRollVel[i]  *= dampFactor;

		mTailYaw[i]   += mTailYawVel[i] * dt;
		mTailPitch[i] += mTailPitchVel[i] * dt;
		mTailRoll[i]  += mTailRollVel[i] * dt;

		const float maxYaw = 0.8f;
		const float maxPitch = 0.5f;
		const float maxRoll = 0.4f;
		if (mTailYaw[i] > maxYaw) mTailYaw[i] = maxYaw;
		if (mTailYaw[i] < -maxYaw) mTailYaw[i] = -maxYaw;
		if (mTailPitch[i] > maxPitch) mTailPitch[i] = maxPitch;
		if (mTailPitch[i] < -maxPitch) mTailPitch[i] = -maxPitch;
		if (mTailRoll[i] > maxRoll) mTailRoll[i] = maxRoll;
		if (mTailRoll[i] < -maxRoll) mTailRoll[i] = -maxRoll;

		const float maxVel = 8.0f;
		if (mTailYawVel[i] > maxVel) mTailYawVel[i] = maxVel;
		if (mTailYawVel[i] < -maxVel) mTailYawVel[i] = -maxVel;
		if (mTailPitchVel[i] > maxVel) mTailPitchVel[i] = maxVel;
		if (mTailPitchVel[i] < -maxVel) mTailPitchVel[i] = -maxVel;

		float euler[3] = { mTailPitch[i], mTailYaw[i], mTailRoll[i] };
		T3DQuat physicsRot;
		t3d_quat_from_euler(&physicsRot, euler);

		T3DQuat animRot = skel->bones[idx].rotation;
		t3d_quat_mul(&skel->bones[idx].rotation, &animRot, &physicsRot);
		skel->bones[idx].hasChanged = 1;

		targetYaw   = mTailYaw[i] * follow;
		targetPitch = mTailPitch[i] * follow;
		targetRoll  = mTailRoll[i] * follow;
	}
	
	if (mStateMachine.isInState("run") && mChestBoneIdx >= 0 && mChestBoneIdx < (int)skel->skeletonRef->boneCount) {
		float chestTwist = -rotVel * 0.15f;
		
		const float maxChestTwist = 0.3f;
		if (chestTwist > maxChestTwist) chestTwist = maxChestTwist;
		if (chestTwist < -maxChestTwist) chestTwist = -maxChestTwist;
		
		float chestEuler[3] = { 0.0f, chestTwist, 0.0f };
		T3DQuat chestTwistQuat;
		t3d_quat_from_euler(&chestTwistQuat, chestEuler);
		
		T3DQuat animRot = skel->bones[mChestBoneIdx].rotation;
		t3d_quat_mul(&skel->bones[mChestBoneIdx].rotation, &animRot, &chestTwistQuat);
		skel->bones[mChestBoneIdx].hasChanged = 1;
	}
}

void CPlayer::clampPosition()
{
	if (mPosition.x() < -mBoundsSize) mPosition.x() = -mBoundsSize;
	if (mPosition.x() >  mBoundsSize) mPosition.x() =  mBoundsSize;
	if (mPosition.z() < -mBoundsSize) mPosition.z() = -mBoundsSize;
	if (mPosition.z() >  mBoundsSize) mPosition.z() =  mBoundsSize;
}

T3DVec3 CPlayer::getHandWorldPosition()
{
	T3DSkeleton* skel = mModel.getSkeleton();
	if (!skel || mHandBoneIdx < 0 || mHandBoneIdx >= (int)skel->skeletonRef->boneCount) {
		return (T3DVec3){{mPosition.x(), mPosition.y() + 5.0f, mPosition.z()}};
	}

	T3DBone& bone = skel->bones[mHandBoneIdx];
	
	float boneX = bone.matrix.m[3][0];
	float boneY = bone.matrix.m[3][1];
	float boneZ = bone.matrix.m[3][2];
	
	T3DMat4 modelMat;
	float scale[3] = {0.125f, 0.125f, 0.125f};
	float rot[3] = {0.0f, -mRotY, 0.0f};
	float pos[3] = {mPosition.x(), mPosition.y(), mPosition.z()};
	t3d_mat4_from_srt_euler(&modelMat, scale, rot, pos);
	
	T3DVec3 bonePos = {{boneX, boneY, boneZ}};
	T3DVec4 worldPos4;
	t3d_mat4_mul_vec3(&worldPos4, &modelMat, &bonePos);
	
	T3DVec3 worldPos = {{worldPos4.v[0], worldPos4.v[1], worldPos4.v[2]}};
	return worldPos;
}

void CPlayer::updateFishingLine(float dt)
{
	if (dt <= 0.0f || dt > 0.1f) dt = 0.016f;
	
	T3DVec3 rodTipPos = getRodTipWorldPosition();
	T3DVec3 targetPos;
	
	if (mBobberFlying || mBobberLanded) {
		targetPos = {{mBobberPos.x(), mBobberPos.y(), mBobberPos.z()}};
	} else {
		targetPos = {{rodTipPos.v[0], rodTipPos.v[1] - 2.0f, rodTipPos.v[2]}};
	}
	
	if (!mRopeInitialized) {
		for (int i = 0; i <= FISHING_LINE_SEGMENTS; ++i) {
			float t = (float)i / (float)FISHING_LINE_SEGMENTS;
			mRopePoints[i].v[0] = MathUtil::lerp(rodTipPos.v[0], targetPos.v[0], t);
			mRopePoints[i].v[1] = MathUtil::lerp(rodTipPos.v[1], targetPos.v[1], t);
			mRopePoints[i].v[2] = MathUtil::lerp(rodTipPos.v[2], targetPos.v[2], t);
			mRopePrevPoints[i] = mRopePoints[i];
		}
		mRopeInitialized = true;
	}
	
	float totalLength = t3d_vec3_distance(&rodTipPos, &targetPos);
	float restLength = totalLength / (float)FISHING_LINE_SEGMENTS;
	restLength *= 1.1f;
	
	const float gravity = -60.0f;
	const float damping = 0.98f;
	const int constraintIterations = 8;
	
	for (int i = 1; i < FISHING_LINE_SEGMENTS; ++i) {
		T3DVec3 current = mRopePoints[i];
		T3DVec3 prev = mRopePrevPoints[i];
		
		T3DVec3 velocity;
		t3d_vec3_diff(&velocity, &current, &prev);
		
		t3d_vec3_scale(&velocity, &velocity, damping);
		
		T3DVec3 newPos;
		newPos.v[0] = current.v[0] + velocity.v[0];
		newPos.v[1] = current.v[1] + velocity.v[1] + gravity * dt * dt;
		newPos.v[2] = current.v[2] + velocity.v[2];
		
		mRopePrevPoints[i] = current;
		mRopePoints[i] = newPos;
	}
	
	mRopePoints[0] = rodTipPos;
	mRopePoints[FISHING_LINE_SEGMENTS] = targetPos;
	mRopePrevPoints[0] = rodTipPos;
	mRopePrevPoints[FISHING_LINE_SEGMENTS] = targetPos;
	
	for (int iter = 0; iter < constraintIterations; ++iter) {
		mRopePoints[0] = rodTipPos;
		mRopePoints[FISHING_LINE_SEGMENTS] = targetPos;
		
		for (int i = 0; i < FISHING_LINE_SEGMENTS; ++i) {
			T3DVec3& p1 = mRopePoints[i];
			T3DVec3& p2 = mRopePoints[i + 1];
			
			T3DVec3 diff;
			t3d_vec3_diff(&diff, &p2, &p1);
			float dist = t3d_vec3_len(&diff);
			
			if (dist < 0.001f) continue;
			
			float error = (dist - restLength) / dist;
			
			float w1 = (i == 0) ? 0.0f : 0.5f;
			float w2 = (i == FISHING_LINE_SEGMENTS - 1) ? 0.0f : 0.5f;
			
			p1.v[0] += diff.v[0] * error * w1;
			p1.v[1] += diff.v[1] * error * w1;
			p1.v[2] += diff.v[2] * error * w1;
			
			p2.v[0] -= diff.v[0] * error * w2;
			p2.v[1] -= diff.v[1] * error * w2;
			p2.v[2] -= diff.v[2] * error * w2;
		}
	}
}

void CPlayer::drawFishingLine(CViewport& viewport, TVec3F const& cameraPos)
{
	if (!mRopeInitialized || !mLineVerts) return;
	
	T3DVec3 camPos = {{cameraPos.x(), cameraPos.y(), cameraPos.z()}};
	
	constexpr float LINE_HALF_WIDTH = 0.35f;
	
	uint32_t lineColor = 0x000000FF;
	
	constexpr float VERT_SCALE = 64.0f;
	constexpr float INV_VERT_SCALE = 1.0f / VERT_SCALE;
	
	for (int i = 0; i <= FISHING_LINE_SEGMENTS; ++i) {
		T3DVec3& pt = mRopePoints[i];
		
		float toCamX = camPos.v[0] - pt.v[0];
		float toCamZ = camPos.v[2] - pt.v[2];
		float toCamLen = sqrtf(toCamX*toCamX + toCamZ*toCamZ);
		
		float perpX, perpZ;
		if (toCamLen > 0.01f) {
			perpX = -toCamZ / toCamLen;
			perpZ = toCamX / toCamLen;
		} else {
			perpX = 1.0f;
			perpZ = 0.0f;
		}
		
		perpX *= LINE_HALF_WIDTH;
		perpZ *= LINE_HALF_WIDTH;
		
		int16_t leftPos[3] = {
			(int16_t)((pt.v[0] - perpX) * VERT_SCALE),
			(int16_t)(pt.v[1] * VERT_SCALE),
			(int16_t)((pt.v[2] - perpZ) * VERT_SCALE)
		};
		
		int16_t rightPos[3] = {
			(int16_t)((pt.v[0] + perpX) * VERT_SCALE),
			(int16_t)(pt.v[1] * VERT_SCALE),
			(int16_t)((pt.v[2] + perpZ) * VERT_SCALE)
		};
		
		T3DVec3 normVec = {{0.0f, 1.0f, 0.0f}};
		uint16_t norm = t3d_vert_pack_normal(&normVec);
		
		mLineVerts[i] = (T3DVertPacked){
			.posA = {leftPos[0], leftPos[1], leftPos[2]},
			.normA = norm,
			.posB = {rightPos[0], rightPos[1], rightPos[2]},
			.normB = norm,
			.rgbaA = lineColor,
			.rgbaB = lineColor,
			.stA = {0, 0},
			.stB = {0, 0},
		};
	}
	
	rdpq_sync_pipe();
	data_cache_hit_writeback(mLineVerts, sizeof(T3DVertPacked) * (FISHING_LINE_SEGMENTS + 1));
	
	T3DMat4 lineMat{};
	t3d_mat4_identity(&lineMat);
	t3d_mat4_scale(&lineMat, INV_VERT_SCALE, INV_VERT_SCALE, INV_VERT_SCALE);
	t3d_mat4_to_fixed(mLineMatFP, &lineMat);

	rdpq_sync_pipe();
	rdpq_mode_push();
	rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
	rdpq_sync_pipe();
	rdpq_set_prim_color(RGBA32(0, 0, 0, 255));
	rdpq_sync_pipe();
	t3d_state_set_drawflags(T3D_FLAG_SHADED);
	rdpq_sync_pipe();
	rdpq_mode_zbuf(false, false);
	rdpq_sync_pipe();
	
	t3d_matrix_push(mLineMatFP);
	rdpq_sync_pipe();
	
	int numVerts = (FISHING_LINE_SEGMENTS + 1) * 2;
	t3d_vert_load(mLineVerts, 0, numVerts);
	
	t3d_matrix_pop(1);
	rdpq_sync_pipe();
	
	for (int i = 0; i < FISHING_LINE_SEGMENTS; ++i) {
		int v0 = i * 2;
		int v1 = i * 2 + 1;
		int v2 = i * 2 + 2;
		int v3 = i * 2 + 3;
		
		rdpq_sync_pipe();
		t3d_tri_draw(v0, v1, v2);
		t3d_tri_draw(v1, v3, v2);
		t3d_tri_draw(v2, v1, v0);
		t3d_tri_draw(v2, v3, v1);
		rdpq_sync_pipe();
	}
	rdpq_mode_pop();
	rdpq_sync_pipe();
	
	t3d_tri_sync();
	rdpq_sync_pipe();
}

void CPlayer::updateThrowTarget()
{
	if (!mCollisionMesh) return;
	
	constexpr float THROW_HEIGHT = 10.0f;
	
	float dirX = sinf(mRotY);
	float dirZ = cosf(mRotY);
	
	float targetX = mPosition.x() + dirX * mThrowDistance;
	float targetZ = mPosition.z() + dirZ * mThrowDistance;
	
	mThrowFloorResult = mCollisionMesh->findFloor(targetX, mPosition.y() + THROW_HEIGHT, targetZ, 50.0f);
	
	if (mThrowFloorResult.found) {
		mThrowTarget = {targetX, mThrowFloorResult.floorY, targetZ};
		
		float pitch = -asinf(mThrowFloorResult.normalZ);
		float roll = asinf(mThrowFloorResult.normalX);
		
		float offsetY = 0.1f;
		mThrowIndicator.setPosition({targetX, mThrowFloorResult.floorY + offsetY, targetZ});
		mThrowIndicator.setRotation({pitch, 0.0f, roll});
		mThrowIndicator.updateMatrix();
	}
}

void CPlayer::drawThrowIndicator()
{
	if (mThrowFloorResult.found) {
		mThrowIndicator.draw();
	}
}

T3DVec3 CPlayer::getRodTipWorldPosition()
{
	T3DSkeleton* rodSkel = mFishingRod.getSkeleton();
	if (!rodSkel || mRodTipBoneIdx < 0) {
		return {{mPosition.x(), mPosition.y() + 5.0f, mPosition.z()}};
	}
	
	T3DBone& bone = rodSkel->bones[mRodTipBoneIdx];
	float boneX = bone.matrix.m[3][0];
	float boneY = bone.matrix.m[3][1];
	float boneZ = bone.matrix.m[3][2];
	
	float boneYAxisX = bone.matrix.m[1][0];
	float boneYAxisY = bone.matrix.m[1][1];
	float boneYAxisZ = bone.matrix.m[1][2];
	
	const float boneTipOffset = 65.0f;
	boneX += boneYAxisX * boneTipOffset;
	boneY += boneYAxisY * boneTipOffset;
	boneZ += boneYAxisZ * boneTipOffset;
	
	T3DVec3 bonePos = {{boneX, boneY, boneZ}};
	T3DVec4 worldPos4;
	t3d_mat4_mul_vec3(&worldPos4, &mRodWorldMatrix, &bonePos);
	
	return {{worldPos4.v[0], worldPos4.v[1], worldPos4.v[2]}};
}

void CPlayer::startBobberThrow()
{
	T3DVec3 tipPos = getRodTipWorldPosition();
	mBobberStart = {tipPos.v[0], tipPos.v[1], tipPos.v[2]};
	
	mBobberEnd = mThrowTarget;
	
	float dx = mBobberEnd.x() - mBobberStart.x();
	float dz = mBobberEnd.z() - mBobberStart.z();
	float dist = sqrtf(dx*dx + dz*dz);
	mBobberDuration = 0.05f + dist * 0.01f;
	
	mBobberPos = mBobberStart;
	mBobberTime = 0.0f;
	mBobberFlying = true;
	mBobberLanded = false;
}

void CPlayer::updateBobber(float dt)
{
	if (!mBobberFlying) return;
	
	mBobberTime += dt;
	
	float t = mBobberTime / mBobberDuration;
	if (t >= 1.0f) {
		t = 1.0f;
		mBobberFlying = false;
		mBobberLanded = true;
	}
	
	float x = mBobberStart.x() + (mBobberEnd.x() - mBobberStart.x()) * t;
	float z = mBobberStart.z() + (mBobberEnd.z() - mBobberStart.z()) * t;
	
	float startY = mBobberStart.y();
	float endY = mBobberEnd.y();
	
	float dx = mBobberEnd.x() - mBobberStart.x();
	float dz = mBobberEnd.z() - mBobberStart.z();
	float dist = sqrtf(dx*dx + dz*dz);
	float arcHeight = 50.0f + dist * 0.15f;
	
	float arc = -4.0f * arcHeight * t * (t - 1.0f);
	
	float y = startY + (endY - startY) * t + arc;
	
	mBobberPos = {x, y, z};
	
	mBobber.setPosition(mBobberPos);
	mBobber.updateMatrix();
}

void CPlayer::drawBobber()
{
	if (mBobberFlying || mBobberLanded) {
		mBobber.draw();
	}
}

void CPlayer::closeItemGetTextBox()
{
	if (mItemTextBoxInitialized) {
		mItemGetTextBox.close();
	}
}

float CPlayer::getItemGetRotation() const
{
	if (mStateMachine.isInState("item_get")) {
		return mStateMachine.getItemGetState().getItemRotation();
	}
	return 0.0f;
}

float CPlayer::getItemGetFadeAlpha() const
{
	if (mStateMachine.isInState("item_get")) {
		return mStateMachine.getItemGetState().getFadeAlpha();
	}
	return 0.0f;
}

void CPlayer::drawItemGetOverlay(int fontId)
{
	if (!mStateMachine.isInState("item_get") || !mCurrentItem) {
		return;
	}
	
	float alpha = getItemGetFadeAlpha();
	if (alpha <= 0.0f) return;
	
	uint8_t alphaVal = (uint8_t)(alpha * 255.0f);
	
	static sprite_t* itmgetSprite = nullptr;
	if (!itmgetSprite) {
		itmgetSprite = sprite_load("rom:/itmget.rgba32.sprite");
	}
	
	if (itmgetSprite) {
		rdpq_set_mode_standard();
		rdpq_mode_alphacompare(1);
		
		int bannerX = (256 - itmgetSprite->width) / 2;
		int bannerY = 20;
		
		rdpq_sync_pipe();
		rdpq_set_prim_color(RGBA32(255, 255, 255, alphaVal));
		rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
		rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
		
		rdpq_sprite_blit(itmgetSprite, bannerX, bannerY, NULL);
	}
	
	if (mItemModelLoaded) {
		rdpq_sync_pipe();
		rdpq_sync_tile();
		
		const int viewportSize = 100;
		const int viewportX = (256 - viewportSize) / 2;
		const int viewportY = 55;
		
		T3DViewport* vp = mItemGetViewport.getViewport();
		vp->size[0] = viewportSize;
		vp->size[1] = viewportSize;
		vp->offset[0] = viewportX;
		vp->offset[1] = viewportY;
		
		float camDist = 25.0f;
		float camX = 0.0f;
		float camZ = camDist;
		float camY = 8.0f;
		
		mItemGetViewport.lookAt({camX, camY, camZ}, {0.0f, 0.0f, 0.0f});
		mItemGetViewport.attach();
		
		uint8_t lightColor[4] = {255, 255, 255, 255};
		t3d_light_set_ambient(lightColor);
		
		if (mItemIsSkinned) {
			rdpq_sync_pipe();
			rdpq_set_mode_standard();
			rdpq_mode_alphacompare(1);
			rdpq_set_prim_color(RGBA32(255, 255, 255, alphaVal));
			
			mItemGetSkinnedModel.draw();
		} else {
			float itemRot = getItemGetRotation();
			mItemGetModel.setPosition({0.0f, 0.0f, 0.0f});
			mItemGetModel.setRotation(TVec3F(0.0f, itemRot, 0.0f));
			mItemGetModel.updateMatrix();
			
			rdpq_sync_pipe();
			rdpq_set_mode_standard();
			rdpq_mode_alphacompare(1);
			rdpq_set_prim_color(RGBA32(255, 255, 255, alphaVal));
			
			mItemGetModel.draw();
		}
		
		rdpq_sync_pipe();
	}
	
	mItemGetTextBox.draw();
}

void CPlayer::drawReelMeter(int fontId)
{
	if (!mStateMachine.isInState("reel")) {
		return;
	}
	
	const CPlayerReelState& reelState = mStateMachine.getReelState();
	int currentTaps = reelState.getTapCount();
	int requiredTaps = reelState.getRequiredTaps();
	float timeElapsed = reelState.getTimeElapsed();
	float maxTapTime = reelState.getMaxTapTime();
	
	const int meterWidth = 200;
	const int meterHeight = 20;
	const int meterX = (256 - meterWidth) / 2;
	const int meterY = 200;
	
	rdpq_sync_pipe();
	rdpq_set_mode_fill(RGBA32(40, 40, 40, 200));
	rdpq_fill_rectangle(meterX, meterY, meterX + meterWidth, meterY + meterHeight);
	
	float tapProgress = (float)currentTaps / (float)requiredTaps;
	if (tapProgress > 1.0f) tapProgress = 1.0f;
	int greenWidth = (int)(tapProgress * (meterWidth - 4));
	
	if (greenWidth > 0) {
		rdpq_set_mode_fill(RGBA32(50, 220, 80, 255));
		rdpq_fill_rectangle(meterX + 2, meterY + 2, meterX + 2 + greenWidth, meterY + meterHeight - 2);
	}
	
	float timeProgress = timeElapsed / maxTapTime;
	if (timeProgress < 0.0f) timeProgress = 0.0f;
	if (timeProgress > 1.0f) timeProgress = 1.0f;
	int redWidth = (int)(timeProgress * (meterWidth - 4));
	
	if (redWidth > 0) {
		uint8_t redIntensity = (uint8_t)(200 + timeProgress * 55);
		rdpq_set_mode_fill(RGBA32(redIntensity, 50, 50, 180));
		rdpq_fill_rectangle(meterX + 2, meterY + 2, meterX + 2 + redWidth, meterY + meterHeight - 2);
	}
	
	rdpq_set_mode_standard();
	rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
	rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
	
	rdpq_fill_rectangle(meterX, meterY, meterX + meterWidth, meterY + 2);
	rdpq_fill_rectangle(meterX, meterY + meterHeight - 2, meterX + meterWidth, meterY + meterHeight);
	rdpq_fill_rectangle(meterX, meterY, meterX + 2, meterY + meterHeight);
	rdpq_fill_rectangle(meterX + meterWidth - 2, meterY, meterX + meterWidth, meterY + meterHeight);
	
	rdpq_sync_pipe();
	rdpq_set_mode_standard();
	rdpq_text_printf(NULL, fontId, meterX + 50, meterY - 15, "TAP A!");
	rdpq_text_printf(NULL, fontId, meterX + 70, meterY + 5, "%d / %d", currentTaps, requiredTaps);
}

void CPlayer::awardExp(int amount)
{
	if (!mMenu || amount <= 0) return;
	
	SPlayerStats stats = mMenu->getPlayerStats();
	
	stats.currentExp += amount;
	
	bool leveledUp = false;
	while (stats.currentExp >= stats.expToNextLevel) {
		stats.currentExp -= stats.expToNextLevel;
		stats.level++;
		stats.expToNextLevel = 100 + stats.level * 50;
		leveledUp = true;
	}
	
	mMenu->setPlayerStats(stats);
	
	mExpGainAmount = amount;
	mExpAnimTimer = 2.0f;
	mExpAnimY = 0.0f;
	mExpAnimAlpha = 1.0f;
	
	if (leveledUp) {
		mPendingLevelUp = true;
		mPendingLevel = stats.level;
	}
}

void CPlayer::triggerPendingLevelUp()
{
	if (!mPendingLevelUp) return;
	
	mPendingLevelUp = false;
	mNewLevel = mPendingLevel;
	mLevelUpAnimTimer = 3.0f;
	mLevelUpScale = 0.0f;
	mLevelUpAlpha = 1.0f;
	CSoundMgr::play("level-up");
}

void CPlayer::drawExpGainAnimation(int fontId)
{
	if (mExpAnimTimer <= 0.0f) return;
	
	int x = 128;
	int y = (int)(80 - mExpAnimY);
	
	uint8_t r, g, b;
	if (mExpGainAmount >= 100) {
		r = 255; g = 215; b = 0;
	} else if (mExpGainAmount >= 30) {
		r = 100; g = 200; b = 255;
	} else {
		r = 100; g = 255; b = 100;
	}
	
	uint8_t alpha = (uint8_t)(mExpAnimAlpha * 255);
	if (alpha < 1) return;
	
	float bounce = sinf(mExpAnimTimer * 10.0f) * 2.0f;
	
	rdpq_sync_pipe();
	rdpq_set_mode_standard();
	rdpq_set_prim_color(RGBA32(r, g, b, alpha));
	rdpq_text_printf(NULL, fontId, x - 25, y + (int)bounce, "+%d EXP", mExpGainAmount);
}

void CPlayer::drawLevelUpAnimation(int fontId)
{
	if (mLevelUpAnimTimer <= 0.0f) return;
	
	uint8_t alpha = (uint8_t)(mLevelUpAlpha * 255);
	if (alpha < 1) return;
	
	int baseX = 85;
	int baseY = 50;
	
	const char* text = "LEVEL UP!";
	int charCount = 9;
	int charWidth = 8;
	
	rdpq_sync_pipe();
	rdpq_set_mode_standard();
	rdpq_mode_alphacompare(1);
	rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
	rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
	
	float time = 3.0f - mLevelUpAnimTimer;
	for (int i = 0; i < charCount; i++) {
		float phase = time * 8.0f - (float)i * 0.8f;
		int yOffset = (int)(sinf(phase) * 4.0f);
		
		int charX = baseX + i * charWidth;
		int charY = baseY + yOffset;
		
		if (alpha > 50) {
			uint8_t glowAlpha = (uint8_t)(alpha / 3);
			rdpq_set_prim_color(RGBA32(255, 255, 100, glowAlpha));
			rdpq_text_printf(NULL, fontId, charX, charY - 1, "%c", text[i]);
			rdpq_text_printf(NULL, fontId, charX, charY + 1, "%c", text[i]);
		}
		
		rdpq_set_prim_color(RGBA32(255, 215, 0, alpha));
		rdpq_text_printf(NULL, fontId, charX, charY, "%c", text[i]);
	}
	
	rdpq_set_prim_color(RGBA32(255, 255, 255, alpha));
	rdpq_text_printf(NULL, fontId, baseX + 20, baseY + 15, "Lv. %d", mNewLevel);
}