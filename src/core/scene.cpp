#include "scene.hpp"
#include "camera.hpp"
#include "wipe.hpp"
#include <t3d/t3dmath.h>
#include <cstring>
#include <cmath>

static CCircleWipe gSceneStarWipe;

CSceneObject::~CSceneObject()
{
    destroy();
}

void CSceneObject::init(const SSceneObjectDef& def)
{
    mName = def.name;
    mPosition = def.position;
    mRotation = def.rotation;
    mScale = def.scale;
    mCollisionRadius = def.collisionRadius;
    mHasInteraction = def.hasInteraction;

    if (def.modelPath != nullptr) {
        if (def.animationName != nullptr) {
            mIsAnimated = true;
            mSkinnedModel.load(def.modelPath);
            mSkinnedModel.setPosition(mPosition);
            mSkinnedModel.setRotation(mRotation);
            mSkinnedModel.setScale(mScale);
            mSkinnedModel.createSkeleton();
            mSkinnedModel.addAnimation(def.animationName, false);
            mSkinnedModel.buildSkinnedDisplayList();
        } else {
            mIsAnimated = false;
            mModel.load(def.modelPath);
            mModel.setPosition(mPosition);
            mModel.setRotation(mRotation);
            mModel.setScale(mScale);
            mModel.updateMatrix();
            mModel.buildDisplayList();
        }
    }

    mLoaded = true;
}

void CSceneObject::update(float dt)
{
    if (!mLoaded) return;

    if (mIsAnimated) {
        mSkinnedModel.updateAnimations(dt);
        mSkinnedModel.updateSkeleton();
    }
}

void CSceneObject::setFrameIndex(uint32_t frameIndex)
{
    if (mIsAnimated) {
        mSkinnedModel.setFrameIndex(frameIndex);
    }
}

void CSceneObject::updateBufferedMatrix(uint32_t frameIndex)
{
    if (mIsAnimated) {
        mSkinnedModel.updateBufferedMatrix(frameIndex);
    }
}

void CSceneObject::draw()
{
    if (!mLoaded) return;

    if (mIsAnimated) {
        mSkinnedModel.draw();
    } else if (mModel.getModel() != nullptr) {
        mModel.draw();
    }
}

void CSceneObject::destroy()
{
    mLoaded = false;
    mInteractionCallback = nullptr;
}

bool CSceneObject::checkPlayerInRange(const TVec3F& playerPos) const
{
    if (mCollisionRadius <= 0.0f) return false;

    float dx = playerPos.x() - mPosition.x();
    float dy = playerPos.y() - mPosition.y();
    float dz = playerPos.z() - mPosition.z();
    float distSq = dx * dx + dy * dy + dz * dz;
    
    return distSq <= (mCollisionRadius * mCollisionRadius);
}

void CSceneObject::setInteractionCallback(std::function<void(CSceneObject&, CPlayer&)> callback)
{
    mInteractionCallback = callback;
}

void CSceneObject::triggerInteraction(CPlayer& player)
{
    if (mInteractionCallback) {
        mInteractionCallback(*this, player);
    }
}

void CNpcObject::init(const SSceneObjectDef& def)
{
    CSceneObject::init(def);
    if (mIsAnimated) {
        T3DSkeleton* skel = mSkinnedModel.getSkeleton();
        if (skel) {
            mHeadBoneIdx = t3d_skeleton_find_bone(skel, "Neck");
        }
    }
}

void CNpcObject::update(float dt)
{
    if (!mLoaded) return;

    if (mIsAnimated) {
        mSkinnedModel.updateMatrix();
        mSkinnedModel.updateAnimations(dt);
        
        updateHeadLookAt(dt);

        mSkinnedModel.updateSkeleton();
    } else {
        mModel.updateMatrix();
    }

    if (!CSceneManager::instance().isInConversation()) {
        if (checkPlayerInRange(CSceneManager::instance().getPlayer()->getPosition())) {
            joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);
            if (btn.a) {
                CSceneManager::instance().startConversation(mPosition, mDialogueLines);
            }
        }
    }
}

void CNpcObject::updateHeadLookAt(float dt)
{
    if (mHeadBoneIdx < 0) return;

    TVec3F playerPos = CSceneManager::instance().getPlayer()->getPosition();
    TVec3F headPos = {mPosition.x(), mPosition.y() + 30.0f, mPosition.z()};
    
    TVec3F dir = {playerPos.x() - headPos.x(), 0.0f, playerPos.z() - headPos.z()};
    float distSq = dir.x()*dir.x() + dir.z()*dir.z();
    
    float targetYaw = 0.0f;

    if (checkPlayerInRange(playerPos) && distSq > 1.0f) {
        float worldTargetYaw = atan2f(dir.x(), dir.z());
        
        targetYaw = worldTargetYaw - mRotation.y();
        
        while (targetYaw > T3D_PI) targetYaw -= 2.0f * T3D_PI;
        while (targetYaw < -T3D_PI) targetYaw += 2.0f * T3D_PI;

        if (targetYaw > 1.2f) targetYaw = 1.2f;
        if (targetYaw < -1.2f) targetYaw = -1.2f;
    }

    mHeadYaw = t3d_lerp(mHeadYaw, targetYaw, 5.0f * dt);

    T3DSkeleton* skel = mSkinnedModel.getSkeleton();
    T3DQuat lookRot;
    float euler[3] = {mHeadYaw, 0.0f, 0.0f};
    t3d_quat_from_euler(&lookRot, euler);
    
    T3DQuat animRot = skel->bones[mHeadBoneIdx].rotation;
    t3d_quat_mul(&skel->bones[mHeadBoneIdx].rotation, &animRot, &lookRot);
    skel->bones[mHeadBoneIdx].hasChanged = 1;
}

void CNpcObject::draw()
{
    CSceneObject::draw();
}

CScene::~CScene()
{
    exit();
}

void CScene::init(const SSceneDef& def, CPlayer& player, CViewport& viewport, CLight& light, CCamera& camera)
{
    mDef = &def;

    if (def.mapModelPath != nullptr) {
        mMapModel.load(def.mapModelPath);
        mMapModel.setScale({1.0f, 1.0f, 1.0f});
        mMapModel.setPosition({0.0f, 0.0f, 0.0f});
        mMapModel.updateMatrix();
        mMapModel.buildDisplayList();
    }

    if (def.collisionPath != nullptr) {
        mCollision.load(def.collisionPath);
        camera.applyCollision(mCollision);
    }

    player.init(def.playerSpawnPos);
    player.setRotY(def.playerSpawnRotY);
    
    camera.setOrbitAngle(def.playerSpawnRotY + T3D_PI);

    mObjectCount = 0;
    if (def.objects != nullptr && def.objectCount > 0) {
        int count = def.objectCount;
        if (count > SCENE_MAX_OBJECTS) count = SCENE_MAX_OBJECTS;

        for (int i = 0; i < count; i++) {
            const SSceneObjectDef& objDef = def.objects[i];
            CSceneObject* obj = nullptr;
            
            switch (objDef.type) {
                case ESceneObjectType::Npc:
                    obj = new CNpcObject();
                    break;
                case ESceneObjectType::Base:
                default:
                    obj = new CSceneObject();
                    break;
            }
            
            if (obj) {
                obj->init(objDef);
                mObjects[mObjectCount++] = obj;
            }
        }
    }

    mLoaded = true;

    if (def.onInit != nullptr) {
        def.onInit(*this);
    }
}

void CScene::update(float dt, CPlayer& player)
{
    if (!mLoaded || mDef == nullptr) return;

    for (int i = 0; i < mObjectCount; i++) {
        mObjects[i]->update(dt);
        if (mObjects[i]->hasInteraction()) {
            if (mObjects[i]->checkPlayerInRange(player.getPosition())) {
                mObjects[i]->triggerInteraction(player);
            }
        }
    }
    if (mDef->onUpdate != nullptr) {
        mDef->onUpdate(*this, dt);
    }
}

void CScene::draw()
{
    if (!mLoaded) return;

    mMapModel.draw();

    for (int i = 0; i < mObjectCount; i++) {
        mObjects[i]->draw();
    }
}

void CScene::setFrameIndex(uint32_t frameIndex)
{
    if (!mLoaded) return;
    
    for (int i = 0; i < mObjectCount; i++) {
        mObjects[i]->setFrameIndex(frameIndex);
    }
}

void CScene::updateBufferedMatrix(uint32_t frameIndex)
{
    if (!mLoaded) return;
    
    for (int i = 0; i < mObjectCount; i++) {
        mObjects[i]->updateBufferedMatrix(frameIndex);
    }
}

void CScene::exit()
{
    if (!mLoaded) return;

    if (mDef != nullptr && mDef->onExit != nullptr) {
        mDef->onExit(*this);
    }

    for (int i = 0; i < mObjectCount; i++) {
        if (mObjects[i]) {
            mObjects[i]->destroy();
            delete mObjects[i];
            mObjects[i] = nullptr;
        }
    }
    mObjectCount = 0;

    mMapModel.unload();
    mCollision.unload();

    mLoaded = false;
    mDef = nullptr;
}

CSceneObject* CScene::getObject(const char* name)
{
    for (int i = 0; i < mObjectCount; i++) {
        if (mObjects[i]->getName() != nullptr && strcmp(mObjects[i]->getName(), name) == 0) {
            return mObjects[i];
        }
    }
    return nullptr;
}

CSceneObject* CScene::getObjectAt(int index)
{
    if (index >= 0 && index < mObjectCount) {
        return mObjects[index];
    }
    return nullptr;
}

CSceneObject* CScene::checkPlayerInteractions(const TVec3F& playerPos)
{
    for (int i = 0; i < mObjectCount; i++) {
        if (mObjects[i]->hasInteraction() && mObjects[i]->checkPlayerInRange(playerPos)) {
            return mObjects[i];
        }
    }
    return nullptr;
}

CSceneManager& CSceneManager::instance()
{
    static CSceneManager mgr;
    return mgr;
}

void CSceneManager::init(CPlayer& player, CViewport& viewport, CLight& light, CCamera& camera)
{
    mPlayer = &player;
    mViewport = &viewport;
    mLight = &light;
    mCamera = &camera;
    mCurrentScene = nullptr;
    mNextScene = nullptr;
    mTransitioning = false;
    mInConversation = false;
    mInShop = false;

    mTextBox.init(2, 20, 170, 216, 60);
    mTextBox.setBackgroundGradient(20, 20, 60, 200, 10, 10, 30, 220);
    mTextBox.setBorderColor(200, 200, 255, 255);
    mTextBox.setPadding(8);
    mTextBox.setAnimation(ETextBoxAnim::SlideBottom, 0.3f);
    
    mShop.init(FONT_BUILTIN_DEBUG_MONO);
}

void CSceneManager::update(float dt)
{
    if (mInLogoScene) {
        mLogoScene.update(dt);
        
        if (mLogoScene.isFadingToCutscene() && !mInCutscene) {
            if (mNextCutsceneAfterLogo != nullptr) {
                startCutscene(*mNextCutsceneAfterLogo);
            }
        }

        if (mLogoScene.isComplete()) {
            mInLogoScene = false;
        }

        if (mInCutscene) {
            mCutscene.update(dt);
        }
        return;
    }

    if (mInCutscene) {
        mCutscene.update(dt);
        
        ECutsceneAction action = mCutscene.getCurrentAction();
        if (action == ECutsceneAction::EndCutscene || mCutscene.isComplete()) {
            endCutscene();
        } else if (action == ECutsceneAction::TransitionToScene) {
            const SSceneDef* sceneDef = static_cast<const SSceneDef*>(mCutscene.getCurrentActionData());
            if (sceneDef != nullptr) {
                endCutscene();
                CSceneManager::instance().loadScene(*sceneDef);
            }
        }
        
        return;
    }

    if (mSceneTransitionState != ESceneTransitionState::None) {
        gSceneStarWipe.update(dt);

        if (mSceneTransitionState == ESceneTransitionState::StarWipeOut) {
            if (gSceneStarWipe.isClosed()) {
                if (mQueuedSceneDef != nullptr) {
                    loadScene(*mQueuedSceneDef);
                }
                mQueuedSceneDef = nullptr;
                mSceneTransitionState = ESceneTransitionState::StarWipeIn;
                gSceneStarWipe.wipeIn(mSceneWipeInDuration);
            }
        } else if (mSceneTransitionState == ESceneTransitionState::StarWipeIn) {
            if (gSceneStarWipe.isOpen()) {
                mSceneTransitionState = ESceneTransitionState::None;
                mTransitioning = false;
            }
        }
    }
    
    if (mCurrentScene != nullptr && mPlayer != nullptr) {
        mCurrentScene->update(dt, *mPlayer);
    }

    if (mInConversation && !mWaitingForAction && !mInShop) {
        mTextBox.update(dt);
        
        joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);
        
        if (mTextBox.hasChoices()) {
            if (mTextBox.handleChoiceInput(btn)) {
                int selectedChoice = mTextBox.getSelectedChoice();
                debugf("Choice selected: %d\n", selectedChoice);
                
                if (mCurrentDialogueLine < (int)mConversationLines.size()) {
                    const auto& line = mConversationLines[mCurrentDialogueLine];
                    debugf("Selected choice index: %d, total choices: %d\n", selectedChoice, (int)line.choices.size());
                    if (selectedChoice >= 0 && selectedChoice < (int)line.choices.size()) {
                        const auto& choice = line.choices[selectedChoice];
                        debugf("Choice text: %s, action data ptr: %p\n", choice.text, choice.actionData);
                        
                        if (choice.action == EDialogueAction::OpenShop && choice.actionData != nullptr) {
                            uintptr_t rawData = reinterpret_cast<uintptr_t>(choice.actionData);
                            debugf("Opening shop from choice! Raw data=%lu, mode will be=%lu\n", 
                                   (unsigned long)rawData, (unsigned long)(rawData - 1));
                            EShopMode mode = static_cast<EShopMode>(rawData - 1);
                            mShop.open(mode);
                            mInShop = true;
                            mWaitingForAction = true;
                        }
                        else if (choice.action == EDialogueAction::GiveItem && choice.actionData != nullptr) {
                            const SItemGetData* itemData = static_cast<const SItemGetData*>(choice.actionData);
                            mPlayer->triggerItemGet(*itemData, "conversation");
                            mWaitingForAction = true;
                        }
                        else if (choice.action == EDialogueAction::SetStoryFlag && choice.actionData != nullptr) {
                            uint32_t flag = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(choice.actionData));
                            CMenu* menu = mPlayer->getMenu();
                            if (menu) {
                                SPlayerStats stats = menu->getPlayerStats();
                                stats.setStoryFlag(flag);
                                menu->setPlayerStats(stats);
                            }
                            mTextBox.clear();
                            mCurrentDialogueLine++;
                        }
                        else {
                            mTextBox.clear();
                            mCurrentDialogueLine++;
                            
                            while (mCurrentDialogueLine < (int)mConversationLines.size()) {
                                const auto& nextLine = mConversationLines[mCurrentDialogueLine];
                                
                                if (nextLine.action != EDialogueAction::None || !nextLine.choices.empty()) {
                                    if (!nextLine.text.empty()) {
                                        mTextBox.setText(nextLine.text.c_str());
                                    }
                                    if (!nextLine.choices.empty()) {
                                        std::vector<std::string> choiceTexts;
                                        for (const auto& c : nextLine.choices) {
                                            choiceTexts.push_back(c.text);
                                        }
                                        mTextBox.setChoices(choiceTexts);
                                    }
                                    break;
                                }
                                
                                mTextBox.setText(nextLine.text.c_str());
                                mCurrentDialogueLine++;
                            }
                            
                            if (mCurrentDialogueLine >= (int)mConversationLines.size()) {
                                endConversation();
                            }
                        }
                    }
                }
            }
        }
        else if (!mTextBox.handleInput(btn.a)) {
            debugf("Textbox closed, current line: %d/%d\n", mCurrentDialogueLine, (int)mConversationLines.size());
            if (mCurrentDialogueLine < (int)mConversationLines.size()) {
                const auto& line = mConversationLines[mCurrentDialogueLine];
                debugf("Line %d: action=%d, text='%s'\n", mCurrentDialogueLine, (int)line.action, line.text.c_str());
                
                if (!line.choices.empty()) {
                    std::vector<std::string> choiceTexts;
                    for (const auto& choice : line.choices) {
                        choiceTexts.push_back(choice.text);
                    }
                    mTextBox.setChoices(choiceTexts);
                    debugf("Set %d choices for player\n", (int)choiceTexts.size());
                }
                else if (line.action == EDialogueAction::GiveItem && line.actionData != nullptr) {
                    const SItemGetData* itemData = static_cast<const SItemGetData*>(line.actionData);
                    mPlayer->triggerItemGet(*itemData, "conversation");
                    mWaitingForAction = true;
                }
                else if (line.action == EDialogueAction::OpenShop && line.actionData != nullptr) {
                    debugf("Opening shop!\n");
                    EShopMode mode = static_cast<EShopMode>(reinterpret_cast<uintptr_t>(line.actionData) - 1);
                    mShop.open(mode);
                    mInShop = true;
                    mWaitingForAction = true;
                    debugf("Shop opened, mInShop=%d\n", mInShop);
                }
                else if (line.action == EDialogueAction::SetStoryFlag && line.actionData != nullptr) {
                    uint32_t flag = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(line.actionData));
                    CMenu* menu = mPlayer->getMenu();
                    if (menu) {
                        SPlayerStats stats = menu->getPlayerStats();
                        stats.setStoryFlag(flag);
                        menu->setPlayerStats(stats);
                        debugf("Set story flag: 0x%08x\n", flag);
                    }
                    mCurrentDialogueLine++;
                    if (mCurrentDialogueLine >= (int)mConversationLines.size()) {
                        endConversation();
                    }
                }
                else if (line.action == EDialogueAction::None) {
                    mTextBox.clear();
                    mCurrentDialogueLine++;
                    
                    while (mCurrentDialogueLine < (int)mConversationLines.size()) {
                        const auto& nextLine = mConversationLines[mCurrentDialogueLine];
                        
                        if (nextLine.action != EDialogueAction::None || !nextLine.choices.empty()) {
                            if (!nextLine.text.empty()) {
                                mTextBox.setText(nextLine.text.c_str());
                            }
                            if (!nextLine.choices.empty()) {
                                std::vector<std::string> choiceTexts;
                                for (const auto& c : nextLine.choices) {
                                    choiceTexts.push_back(c.text);
                                }
                                mTextBox.setChoices(choiceTexts);
                            }
                            break;
                        }
                        
                        mTextBox.setText(nextLine.text.c_str());
                        mCurrentDialogueLine++;
                    }
                    
                    if (mCurrentDialogueLine >= (int)mConversationLines.size()) {
                        endConversation();
                    }
                }
            } else {
                endConversation();
            }
        }
    }
    
    if (mWaitingForAction && !mPlayer->getStateMachine().isInState("item_get") && !mInShop) {
        resumeConversation();
    }
    
    if (mInShop && !mShop.isOpen()) {
        mInShop = false;
        resumeConversation();
    }
}

void CSceneManager::draw()
{
    if (mInCutscene) {
        mCutscene.draw();
    } else if (mCurrentScene != nullptr) {
        mCurrentScene->draw();
    }

    if (mInLogoScene) {
        mLogoScene.draw();
    }
}

void CSceneManager::drawUI()
{
    if (!mInLogoScene && mInConversation) {
        mTextBox.draw();
    }
}

void CSceneManager::drawTransitionOverlays()
{
    if (mSceneTransitionState != ESceneTransitionState::None || gSceneStarWipe.isActive()) {
        gSceneStarWipe.draw();
    }
}

void CSceneManager::setFrameIndex(uint32_t frameIndex)
{
    if (mInCutscene) {
        mCutscene.setFrameIndex(frameIndex);
    } else if (mCurrentScene != nullptr) {
        mCurrentScene->setFrameIndex(frameIndex);
    }
}

void CSceneManager::updateBufferedMatrix(uint32_t frameIndex)
{
    if (mInCutscene) {
        mCutscene.updateBufferedMatrix(frameIndex);
    } else if (mCurrentScene != nullptr) {
        mCurrentScene->updateBufferedMatrix(frameIndex);
    }
}

void CSceneManager::startConversation(const TVec3F& npcPos, const std::vector<SDialogueLine>& lines)
{
    if (mInConversation) return;

    debugf("Starting conversation with %d lines\n", (int)lines.size());
    mInConversation = true;
    mConversationNpcPos = npcPos;
    mConversationLines = lines;
    mCurrentDialogueLine = 0;
    mWaitingForAction = false;
    
    if (mCamera) mCamera->startConversation(npcPos);
    if (mPlayer) {
        mPlayer->setInConversation(true);
        mPlayer->rotateTowards(npcPos);
        mPlayer->getStateMachine().transitionTo(mPlayer, "idle");
        mPlayer->getAnimController().forceBlendFactor(0.0f);
    }

    mTextBox.clear();
    while (mCurrentDialogueLine < (int)mConversationLines.size()) {
        const auto& line = mConversationLines[mCurrentDialogueLine];
        
        debugf("Queueing line %d: action=%d, text='%s', has_choices=%d\n", 
               mCurrentDialogueLine, (int)line.action, line.text.c_str(), !line.choices.empty());
        
        if (line.action != EDialogueAction::None || !line.choices.empty()) {
            debugf("Hit action/choice line, stopping queue\n");
            if (!line.text.empty()) {
                mTextBox.setText(line.text.c_str());
            }
            if (!line.choices.empty()) {
                std::vector<std::string> choiceTexts;
                for (const auto& choice : line.choices) {
                    choiceTexts.push_back(choice.text);
                }
                mTextBox.setChoices(choiceTexts);
            }
            break;
        }
        
        mTextBox.setText(line.text.c_str());
        mCurrentDialogueLine++;
    }
    debugf("Initial queue done, current line now: %d\n", mCurrentDialogueLine);
}

void CSceneManager::resumeConversation()
{
    if (!mInConversation || !mWaitingForAction) return;
    
    mWaitingForAction = false;
    
    if (mCamera) mCamera->startConversation(mConversationNpcPos);
    if (mPlayer) {
        mPlayer->setInConversation(true);
        mPlayer->rotateTowards(mConversationNpcPos);
    }
    
    mTextBox.clear();
    mCurrentDialogueLine++;
    
    if (mCurrentDialogueLine < (int)mConversationLines.size()) {
        const auto& line = mConversationLines[mCurrentDialogueLine];
        
        if (!line.text.empty()) {
            mTextBox.setText(line.text.c_str());
        }
        
        if (!line.choices.empty()) {
            std::vector<std::string> choiceTexts;
            for (const auto& choice : line.choices) {
                choiceTexts.push_back(choice.text);
            }
            mTextBox.setChoices(choiceTexts);
        }
    } else {
        endConversation();
    }
}

void CSceneManager::endConversation()
{
    if (!mInConversation) return;

    mInConversation = false;
    
    if (mCamera) mCamera->endConversation();
    if (mPlayer) mPlayer->setInConversation(false);
}

void CSceneManager::loadScene(const SSceneDef& def)
{
    if (mPlayer == nullptr || mViewport == nullptr || mLight == nullptr) {
        return;
    }

    if (mCurrentScene != nullptr) {
        mCurrentScene->exit();
    }

    if (mCurrentScene == &mSceneA || mCurrentScene == nullptr) {
        mCurrentScene = &mSceneB;
    } else {
        mCurrentScene = &mSceneA;
    }

    mCurrentScene->init(def, *mPlayer, *mViewport, *mLight, *mCamera);
    
    mPlayer->freezeInput(1.0f);
}

void CSceneManager::transitionToSceneStar(const SSceneDef& def, float wipeOutDuration, float wipeInDuration)
{
    if (mSceneTransitionState != ESceneTransitionState::None) return;
    if (wipeOutDuration <= 0.0f) wipeOutDuration = 0.01f;
    if (wipeInDuration <= 0.0f) wipeInDuration = 0.01f;

    gSceneStarWipe.init();

    if (mInConversation) {
        endConversation();
    }

    mQueuedSceneDef = &def;
    mSceneWipeOutDuration = wipeOutDuration;
    mSceneWipeInDuration = wipeInDuration;
    mSceneTransitionState = ESceneTransitionState::StarWipeOut;
    mTransitioning = true;

    gSceneStarWipe.wipeOut(mSceneWipeOutDuration);
}

void CSceneManager::reloadCurrentScene()
{
    if (mCurrentScene != nullptr && mCurrentScene->isLoaded()) {
    }
}

CLogoScene::~CLogoScene()
{
    if (mN64Logo) {
        sprite_free(mN64Logo);
        mN64Logo = nullptr;
    }
    if (mZendenLogo) {
        sprite_free(mZendenLogo);
        mZendenLogo = nullptr;
    }
}

void CLogoScene::init()
{
    if (!mN64Logo) mN64Logo = sprite_load("rom:/n64.sprite");
    if (!mZendenLogo) mZendenLogo = sprite_load("rom:/zenden.sprite");
    
    mState = ELogoState::StartWhite;
    mTimer = 1.0f;
    mAlpha = 0.0f;
    mComplete = false;
}

void CLogoScene::update(float dt)
{
    mTimer -= dt;
    
    switch (mState) {
        case ELogoState::StartWhite:
            if (mTimer <= 0.0f) {
                mState = ELogoState::FadeInN64;
                mTimer = 1.0f;
            }
            break;
            
        case ELogoState::FadeInN64:
            mAlpha = 1.0f - (mTimer / 1.0f);
            if (mTimer <= 0.0f) {
                mAlpha = 1.0f;
                mState = ELogoState::StayN64;
                mTimer = 2.5f;
            }
            break;
            
        case ELogoState::StayN64:
            if (mTimer <= 0.0f) {
                mState = ELogoState::FadeOutN64;
                mTimer = 1.0f;
            }
            break;
            
        case ELogoState::FadeOutN64:
            mAlpha = mTimer / 1.0f;
            if (mTimer <= 0.0f) {
                mAlpha = 0.0f;
                mState = ELogoState::FadeInZenden;
                mTimer = 1.0f;
            }
            break;
            
        case ELogoState::FadeInZenden:
            mAlpha = 1.0f - (mTimer / 1.0f);
            if (mTimer <= 0.0f) {
                mAlpha = 1.0f;
                mState = ELogoState::StayZenden;
                mTimer = 2.5f;
            }
            break;
            
        case ELogoState::StayZenden:
            if (mTimer <= 0.0f) {
                mState = ELogoState::FadeOutZenden;
                mTimer = 1.0f;
            }
            break;
            
        case ELogoState::FadeOutZenden:
            mAlpha = mTimer / 1.0f;
            if (mTimer <= 0.0f) {
                mAlpha = 0.0f;
                mState = ELogoState::FadeToCutscene;
                mTimer = 2.0f;
            }
            break;
            
        case ELogoState::FadeToCutscene:
            if (mTimer <= 0.0f) {
                mState = ELogoState::Done;
                mComplete = true;
            }
            break;

        case ELogoState::Done:
            mComplete = true;
            break;
    }
}

void CLogoScene::draw()
{
    if (mState != ELogoState::FadeToCutscene) {
        rdpq_set_mode_fill(RGBA32(255, 255, 255, 255));
        rdpq_fill_rectangle(0, 0, 256, 240);
    }
    
    sprite_t* currentSprite = nullptr;
    if (mState == ELogoState::FadeInN64 || mState == ELogoState::StayN64 || mState == ELogoState::FadeOutN64) {
        currentSprite = mN64Logo;
    } else if (mState == ELogoState::FadeInZenden || mState == ELogoState::StayZenden || mState == ELogoState::FadeOutZenden) {
        currentSprite = mZendenLogo;
    }
    
    if (currentSprite) {
        rdpq_set_mode_standard();
        rdpq_mode_alphacompare(1);
        
        rdpq_sprite_blit(currentSprite, 
            (256 - currentSprite->width) / 2, 
            (240 - currentSprite->height) / 2, 
            NULL);
            
        if (mAlpha < 1.0f) {
            uint8_t overlayAlpha = (uint8_t)((1.0f - mAlpha) * 255.0f);
            
            rdpq_sync_pipe();
            rdpq_set_mode_standard();
            rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
            
            rdpq_set_prim_color(RGBA32(255, 255, 255, overlayAlpha));
            rdpq_fill_rectangle(0, 0, 256, 240);
        }
    }
    
    if (mState == ELogoState::FadeToCutscene) {
        float progress = mTimer / 2.0f;
        if (progress > 1.0f) progress = 1.0f;
        if (progress < 0.0f) progress = 0.0f;
        
        uint8_t overlayAlpha = (uint8_t)(progress * 255.0f);
        if (overlayAlpha > 0) {
            rdpq_sync_pipe();
            rdpq_set_mode_standard();
            rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
            
            rdpq_set_prim_color(RGBA32(255, 255, 255, overlayAlpha));
            rdpq_fill_rectangle(0, 0, 256, 240);
        }
    }
}

CCutsceneScene::~CCutsceneScene()
{
    exit();
    if (mStaticModels != nullptr) {
        delete[] mStaticModels;
        mStaticModels = nullptr;
    }
    if (mSkinnedModels != nullptr) {
        delete[] mSkinnedModels;
        mSkinnedModels = nullptr;
    }
    if (mIsAnimated != nullptr) {
        delete[] mIsAnimated;
        mIsAnimated = nullptr;
    }
}

void CCutsceneScene::init(const SCutsceneDef& def, CViewport& viewport, uint32_t frameIndex)
{
    mDef = &def;
    mViewport = &viewport;
    mFrameIndex = frameIndex;
    mTime = 0.0f;
    mCurrentFrameIndex = 0;
    mComplete = false;
    mCurrentAction = ECutsceneAction::None;
    mCurrentActionData = nullptr;
    mLastActionCheckTime = -1.0f;
    
    if (def.objects != nullptr && def.objectCount > 0) {
        mModelCount = def.objectCount;
        mStaticModels = new CModel[mModelCount];
        mSkinnedModels = new CSkinnedModel[mModelCount];
        mIsAnimated = new bool[mModelCount];
        
        for (int i = 0; i < mModelCount; i++) {
            const auto& objDef = def.objects[i];
            
            if (objDef.modelPath != nullptr) {
                if (objDef.animationName != nullptr) {
                    mIsAnimated[i] = true;
                    mSkinnedModels[i].load(objDef.modelPath);
                    mSkinnedModels[i].setPosition(objDef.position);
                    mSkinnedModels[i].setRotation(objDef.rotation);
                    mSkinnedModels[i].setScale(objDef.scale);
                    mSkinnedModels[i].createSkeleton();
                    mSkinnedModels[i].addAnimation(objDef.animationName, 0);
                    mSkinnedModels[i].playAnimation(objDef.animationName);
                    mSkinnedModels[i].setFrameIndex(frameIndex);
                    mSkinnedModels[i].updateBufferedMatrix(frameIndex);
                    mSkinnedModels[i].buildSkinnedDisplayList();
                } else {
                    mIsAnimated[i] = false;
                    mStaticModels[i].load(objDef.modelPath);
                    mStaticModels[i].setPosition(objDef.position);
                    mStaticModels[i].setRotation(objDef.rotation);
                    mStaticModels[i].setScale(objDef.scale);
                    mStaticModels[i].updateMatrix();
                    mStaticModels[i].buildDisplayList();
                }
            } else {
                mIsAnimated[i] = false;
            }
        }
    }
    
    if (def.cameraFrames != nullptr && def.frameCount > 0) {
        mCameraPos = def.cameraFrames[0].position;
        mCameraRot = def.cameraFrames[0].rotation;
        
        TVec3F forward = {
            sinf(mCameraRot.y()) * cosf(mCameraRot.x()),
            -sinf(mCameraRot.x()),
            cosf(mCameraRot.y()) * cosf(mCameraRot.x())
        };
        TVec3F target = mCameraPos + forward * 100.0f;
        viewport.lookAt(mCameraPos, target);
    }
    
    mLoaded = true;
    
    if (def.onInit != nullptr) {
        def.onInit();
    }
}

void CCutsceneScene::update(float dt)
{
    if (!mLoaded || mComplete) return;
    
    mTime += dt;
    
    for (int i = 0; i < mModelCount; i++) {
        if (mIsAnimated[i]) {
            mSkinnedModels[i].updateAnimations(dt);
            mSkinnedModels[i].updateSkeleton();
        }
    }
    
    updateCamera(dt);
    
    if (mViewport != nullptr) {
        TVec3F forward = {
            sinf(mCameraRot.y()) * cosf(mCameraRot.x()),
            -sinf(mCameraRot.x()),
            cosf(mCameraRot.y()) * cosf(mCameraRot.x())
        };
        TVec3F target = mCameraPos + forward * 100.0f;
        mViewport->lookAt(mCameraPos, target);
    }
    
    checkActions();
    
    if (mDef->frameCount > 0 && mCurrentFrameIndex >= mDef->frameCount) {
        mComplete = true;
    }
}

void CCutsceneScene::updateCamera(float dt)
{
    if (mDef->cameraFrames == nullptr || mDef->frameCount == 0) return;
    
    while (mCurrentFrameIndex < mDef->frameCount - 1 && 
           mTime >= mDef->cameraFrames[mCurrentFrameIndex + 1].time) {
        mCurrentFrameIndex++;
    }
    
    if (mCurrentFrameIndex >= mDef->frameCount - 1) {
        const auto& frame = mDef->cameraFrames[mDef->frameCount - 1];
        mCameraPos = frame.position;
        mCameraRot = frame.rotation;
        return;
    }
    
    const auto& current = mDef->cameraFrames[mCurrentFrameIndex];
    const auto& next = mDef->cameraFrames[mCurrentFrameIndex + 1];
    
    float frameTime = next.time - current.time;
    float t = 0.0f;
    
    if (frameTime > 0.0f && current.lerp) {
        t = (mTime - current.time) / frameTime;
        if (t > 1.0f) t = 1.0f;
    } else if (frameTime > 0.0f && !current.lerp) {
        t = 0.0f;
    }
    
    mCameraPos.x() = current.position.x() + (next.position.x() - current.position.x()) * t;
    mCameraPos.y() = current.position.y() + (next.position.y() - current.position.y()) * t;
    mCameraPos.z() = current.position.z() + (next.position.z() - current.position.z()) * t;
    
    mCameraRot.x() = current.rotation.x() + (next.rotation.x() - current.rotation.x()) * t;
    mCameraRot.y() = current.rotation.y() + (next.rotation.y() - current.rotation.y()) * t;
    mCameraRot.z() = current.rotation.z() + (next.rotation.z() - current.rotation.z()) * t;
}

void CCutsceneScene::checkActions()
{
    mCurrentAction = ECutsceneAction::None;
    mCurrentActionData = nullptr;
    
    if (mDef->cameraFrames == nullptr || mDef->frameCount == 0) return;
    
    if (mCurrentFrameIndex < mDef->frameCount) {
        const auto& frame = mDef->cameraFrames[mCurrentFrameIndex];
        
        if (mLastActionCheckTime < frame.time && mTime >= frame.time) {
            if (frame.actionId != ECutsceneAction::None) {
                mCurrentAction = frame.actionId;
                mCurrentActionData = frame.actionData;
            }
            mLastActionCheckTime = frame.time;
        }
    }
}

void CCutsceneScene::draw()
{
    if (!mLoaded) return;
    
    for (int i = 0; i < mModelCount; i++) {
        if (mIsAnimated[i]) {
            mSkinnedModels[i].draw();
        } else {
            mStaticModels[i].draw();
        }
    }
}

void CCutsceneScene::exit()
{
    if (!mLoaded) return;
    
    if (mDef != nullptr && mDef->onEnd != nullptr) {
        mDef->onEnd();
    }
    
    mLoaded = false;
    mDef = nullptr;
}

void CCutsceneScene::setFrameIndex(uint32_t frameIndex)
{
    for (int i = 0; i < mModelCount; i++) {
        if (mIsAnimated[i]) {
            mSkinnedModels[i].setFrameIndex(frameIndex);
        }
    }
}

void CCutsceneScene::updateBufferedMatrix(uint32_t frameIndex)
{
    for (int i = 0; i < mModelCount; i++) {
        if (mIsAnimated[i]) {
            mSkinnedModels[i].updateBufferedMatrix(frameIndex);
        }
    }
}

void CSceneManager::startLogoScene(const SCutsceneDef* nextCutscene)
{
    mInLogoScene = true;
    mNextCutsceneAfterLogo = nextCutscene;
    mLogoScene.init();
}

void CSceneManager::startCutscene(const SCutsceneDef& def)
{
    if (mViewport == nullptr) return;
    
    uint32_t frameIndex = 0;
    
    mInCutscene = true;
    mCutscene.init(def, *mViewport, frameIndex);
}

void CSceneManager::endCutscene()
{
    if (!mInCutscene) return;
    
    mCutscene.exit();
    mInCutscene = false;
}