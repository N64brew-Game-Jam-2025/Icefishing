#pragma once

#include <libdragon.h>
#include <string>
#include <vector>
#include <functional>
#include "math.hpp"
#include "model.hpp"
#include "skinned_model.hpp"
#include "collision.hpp"
#include "player.hpp"
#include "viewport.hpp"
#include "light.hpp"
#include "textbox.hpp"
#include "shop.hpp"

constexpr int SCENE_MAX_OBJECTS = 32;
constexpr int CUTSCENE_MAX_FRAMES = 64;

class CScene;
class CSceneManager;
class CCamera;

enum class EDialogueAction
{
    None = 0,
    GiveItem,
    TakeItem,
    StartQuest,
    CompleteQuest,
    OpenShop,
    SetStoryFlag,
    Custom
};

struct SDialogueChoice
{
    std::string text;
    EDialogueAction action;
    const void* actionData;
    
    SDialogueChoice(const char* txt, EDialogueAction act = EDialogueAction::None, const void* data = nullptr)
        : text(txt), action(act), actionData(data) {}
};

struct SDialogueLine
{
    std::string text;
    EDialogueAction action;
    const void* actionData;
    std::vector<SDialogueChoice> choices;
    
    SDialogueLine(const char* txt) : text(txt), action(EDialogueAction::None), actionData(nullptr) {}
    SDialogueLine(const std::string& txt) : text(txt), action(EDialogueAction::None), actionData(nullptr) {}
    
    SDialogueLine(const char* txt, EDialogueAction act, const void* data = nullptr) 
        : text(txt), action(act), actionData(data) {}
    
    SDialogueLine(const char* txt, const std::vector<SDialogueChoice>& choiceList)
        : text(txt), action(EDialogueAction::None), actionData(nullptr), choices(choiceList) {}
};

enum class ECutsceneAction {
    None = 0,
    PlaySound,
    TransitionToScene,
    FadeOut,
    FadeIn,
    EndCutscene
};

struct SCutsceneCameraFrame
{
    float time;
    TVec3F position;
    TVec3F rotation;
    ECutsceneAction actionId;
    const void* actionData;
    bool lerp = true;
};

struct SCutsceneObjectDef
{
    const char* modelPath;
    const char* animationName;
    TVec3F position;
    TVec3F rotation;
    TVec3F scale;
};

struct SCutsceneDef
{
    const char* name;
    
    const SCutsceneObjectDef* objects;
    int objectCount;
    
    const SCutsceneCameraFrame* cameraFrames;
    int frameCount;
    
    void (*onInit)(void);
    void (*onEnd)(void);
};

enum class ESceneObjectType {
    Base,
    Npc
};

struct SSceneObjectDef
{
    ESceneObjectType type;
    const char* name;
    const char* modelPath;
    const char* animationName;
    TVec3F position;
    TVec3F rotation;
    TVec3F scale;
    float collisionRadius;
    bool hasInteraction;
};

class CSceneObject
{
public:
    CSceneObject() = default;
    virtual ~CSceneObject();

    virtual void init(const SSceneObjectDef& def);
    virtual void update(float dt);
    virtual void draw();
    virtual void destroy();
    
    virtual void setFrameIndex(uint32_t frameIndex);
    virtual void updateBufferedMatrix(uint32_t frameIndex);

    bool checkPlayerInRange(const TVec3F& playerPos) const;
    void setInteractionCallback(std::function<void(CSceneObject&, CPlayer&)> callback);
    void triggerInteraction(CPlayer& player);

    const char* getName() const { return mName; }
    TVec3F getPosition() const { return mPosition; }
    float getCollisionRadius() const { return mCollisionRadius; }
    bool hasInteraction() const { return mHasInteraction; }
    bool isLoaded() const { return mLoaded; }

    CSkinnedModel* getSkinnedModel() { return mIsAnimated ? &mSkinnedModel : nullptr; }
    CModel* getModel() { return mIsAnimated ? nullptr : &mModel; }

protected:
    const char* mName = nullptr;
    TVec3F mPosition{0, 0, 0};
    TVec3F mRotation{0, 0, 0};
    TVec3F mScale{1, 1, 1};
    float mCollisionRadius = 0.0f;
    bool mHasInteraction = false;
    bool mLoaded = false;
    bool mIsAnimated = false;

    CModel mModel{};
    CSkinnedModel mSkinnedModel{};
    
    std::function<void(CSceneObject&, CPlayer&)> mInteractionCallback;
};

class CNpcObject : public CSceneObject
{
public:
    CNpcObject() = default;
    virtual ~CNpcObject() = default;

    virtual void init(const SSceneObjectDef& def) override;
    virtual void update(float dt) override;
    virtual void draw() override;

    void setDialogueLines(const std::vector<SDialogueLine>& lines) {
        mDialogueLines = lines;
    }

    const std::vector<SDialogueLine>& getDialogueLines() const {
        return mDialogueLines;
    }
private:
    void updateHeadLookAt(float dt);

    std::vector<SDialogueLine> mDialogueLines{};
    int mHeadBoneIdx = -1;
    float mHeadYaw = 0.0f;
    float mHeadPitch = 0.0f;
};

struct SSceneDef
{
    const char* name;
    const char* mapModelPath;
    const char* collisionPath;
    TVec3F playerSpawnPos;
    float playerSpawnRotY;
    
    void (*onInit)(CScene& scene);
    void (*onUpdate)(CScene& scene, float dt);
    void (*onExit)(CScene& scene);
    
    const SSceneObjectDef* objects;
    int objectCount;
};

class CScene
{
public:
    CScene() = default;
    ~CScene();

    void init(const SSceneDef& def, CPlayer& player, CViewport& viewport, CLight& light, CCamera& camera);
    void update(float dt, CPlayer& player);
    void draw();
    void exit();
    
    void setFrameIndex(uint32_t frameIndex);
    void updateBufferedMatrix(uint32_t frameIndex);

    CSceneObject* getObject(const char* name);
    CSceneObject* getObjectAt(int index);
    int getObjectCount() const { return mObjectCount; }

    CModel* getMapModel() { return &mMapModel; }
    CCollisionMesh* getCollision() { return &mCollision; }

    const char* getName() const { return mDef ? mDef->name : nullptr; }
    bool isLoaded() const { return mLoaded; }

    CSceneObject* checkPlayerInteractions(const TVec3F& playerPos);

private:
    const SSceneDef* mDef = nullptr;
    
    CModel mMapModel{};
    CCollisionMesh mCollision{};
    
    CSceneObject* mObjects[SCENE_MAX_OBJECTS];
    int mObjectCount = 0;
    
    bool mLoaded = false;
};

class CLogoScene
{
public:
    CLogoScene() = default;
    ~CLogoScene();

    void init();
    void update(float dt);
    void draw();
    
    bool isComplete() const { return mComplete; }
    bool isFadingToCutscene() const { return mState == ELogoState::FadeToCutscene; }

private:
    enum class ELogoState {
        StartWhite,
        FadeInN64,
        StayN64,
        FadeOutN64,
        FadeInZenden,
        StayZenden,
        FadeOutZenden,
        FadeToCutscene,
        Done
    };

    sprite_t* mN64Logo = nullptr;
    sprite_t* mZendenLogo = nullptr;
    ELogoState mState = ELogoState::StartWhite;
    float mTimer = 0.0f;
    float mAlpha = 0.0f;
    bool mComplete = false;
};

class CCutsceneScene
{
public:
    CCutsceneScene() = default;
    ~CCutsceneScene();

    void init(const SCutsceneDef& def, CViewport& viewport, uint32_t frameIndex);
    void update(float dt);
    void draw();
    void exit();
    
    void setFrameIndex(uint32_t frameIndex);
    void updateBufferedMatrix(uint32_t frameIndex);
    
    bool isComplete() const { return mComplete; }
    const SCutsceneDef* getDef() const { return mDef; }
    
    TVec3F getCameraPosition() const { return mCameraPos; }
    
    ECutsceneAction getCurrentAction() const { return mCurrentAction; }
    const void* getCurrentActionData() const { return mCurrentActionData; }
    
private:
    void updateCamera(float dt);
    void checkActions();
    
    const SCutsceneDef* mDef = nullptr;
    CModel* mStaticModels = nullptr;
    CSkinnedModel* mSkinnedModels = nullptr;
    bool* mIsAnimated = nullptr;
    int mModelCount = 0;
    CViewport* mViewport = nullptr;
    
    float mTime = 0.0f;
    int mCurrentFrameIndex = 0;
    bool mLoaded = false;
    bool mComplete = false;
    uint32_t mFrameIndex = 0;
    float mLastActionCheckTime = -1.0f;
    
    TVec3F mCameraPos{0, 0, 0};
    TVec3F mCameraRot{0, 0, 0};
    
    ECutsceneAction mCurrentAction = ECutsceneAction::None;
    const void* mCurrentActionData = nullptr;
};

class CSceneManager
{
public:
    static CSceneManager& instance();

    void init(CPlayer& player, CViewport& viewport, CLight& light, CCamera& camera);
    void update(float dt);
    void draw();
    
    void drawUI();

    void drawTransitionOverlays();
    
    void setFrameIndex(uint32_t frameIndex);
    void updateBufferedMatrix(uint32_t frameIndex);

    void loadScene(const SSceneDef& def);
    void transitionToSceneStar(const SSceneDef& def, float wipeOutDuration = 0.5f, float wipeInDuration = 0.5f);
    void reloadCurrentScene();
    
    void startCutscene(const SCutsceneDef& def);
    void endCutscene();
    bool isInCutscene() const { return mInCutscene; }
    
    void startLogoScene(const SCutsceneDef* nextCutscene);
    bool isInLogoScene() const { return mInLogoScene; }
    
    CScene* getCurrentScene() { return mCurrentScene; }
    CPlayer* getPlayer() { return mPlayer; }
    CCamera* getCamera() { return mCamera; }
    CTextBox* getTextBox() { return &mTextBox; }
    CShop* getShop() { return &mShop; }
    
    TVec3F getFocusPosition() const {
        if (mInCutscene) return mCutscene.getCameraPosition();
        return mPlayer ? mPlayer->getPosition() : TVec3F{0,0,0};
    }

    void startConversation(const TVec3F& npcPos, const std::vector<SDialogueLine>& lines);
    void endConversation();
    bool isInConversation() const { return mInConversation; }
    void resumeConversation();

private:
    CSceneManager() = default;

    enum class ESceneTransitionState {
        None = 0,
        StarWipeOut,
        StarWipeIn,
    };
    
    CScene mSceneA{};
    CScene mSceneB{};
    CScene* mCurrentScene = nullptr;
    CScene* mNextScene = nullptr;
    
    CLogoScene mLogoScene{};
    bool mInLogoScene = false;
    const SCutsceneDef* mNextCutsceneAfterLogo = nullptr;
    
    CCutsceneScene mCutscene{};
    bool mInCutscene = false;
    
    CPlayer* mPlayer = nullptr;
    CViewport* mViewport = nullptr;
    CLight* mLight = nullptr;
    CCamera* mCamera = nullptr;
    
    CTextBox mTextBox{};
    CShop mShop{};
    bool mInConversation = false;
    bool mTransitioning = false;
    bool mInShop = false;

    ESceneTransitionState mSceneTransitionState = ESceneTransitionState::None;
    const SSceneDef* mQueuedSceneDef = nullptr;
    float mSceneWipeOutDuration = 0.5f;
    float mSceneWipeInDuration = 0.5f;
    
    TVec3F mConversationNpcPos{0,0,0};
    std::vector<SDialogueLine> mConversationLines;
    int mCurrentDialogueLine = 0;
    bool mWaitingForAction = false;
};

#define SCENE_OBJECT(objName, mdlPath, animName, px, py, pz, rx, ry, rz, sx, sy, sz, radius, interact) \
    { ESceneObjectType::Base, objName, mdlPath, animName, {px, py, pz}, {rx, ry, rz}, {sx, sy, sz}, radius, interact }

#define SCENE_OBJECT_NPC(objName, mdlPath, animName, px, py, pz, rx, ry, rz, sx, sy, sz, radius, interact) \
    { ESceneObjectType::Npc, objName, mdlPath, animName, {px, py, pz}, {rx, ry, rz}, {sx, sy, sz}, radius, interact }

#define SCENE_OBJECT_SIMPLE(objName, mdlPath, px, py, pz) \
    { ESceneObjectType::Base, objName, mdlPath, nullptr, {px, py, pz}, {0, 0, 0}, {1, 1, 1}, 0.0f, false }

#define SCENE_OBJECT_INTERACTABLE(objName, mdlPath, px, py, pz, radius) \
    { ESceneObjectType::Base, objName, mdlPath, nullptr, {px, py, pz}, {0, 0, 0}, {1, 1, 1}, radius, true }
