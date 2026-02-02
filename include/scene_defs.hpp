#pragma once

#include "scene.hpp"
#include "player.hpp"
#include "player_state.hpp"
#include "shop.hpp"
#include "sound.hpp"

static const SFishData sFishPool[] = {
    // common (5-15 exp)
    {"Minnow", "Barely a snack. Even for other fish.", nullptr, 5, 5, EFishRarity::Common, 0, 5},
    {"Bluegill", "Blue and round. Like a swimming bruise.", nullptr, 8, 6, EFishRarity::Common, 1, 7},
    {"Perch", "Stripes are slimming, or so it thinks.", nullptr, 10, 7, EFishRarity::Common, 2, 8},
    {"Carp", "The participation trophy of fishing.", nullptr, 7, 5, EFishRarity::Common, 3, 6},
    {"Sunfish", "Shaped like the sun. Tastes like pond.", nullptr, 6, 5, EFishRarity::Common, 4, 5},
    {"Chub", "Chubby, grumpy, and not very fast.", nullptr, 9, 6, EFishRarity::Common, 5, 7},
    {"Shiner", "So shiny. So useless. So common.", nullptr, 4, 4, EFishRarity::Common, 6, 5},
    {"Dace", "Runs away from everything. Smart fish.", nullptr, 6, 5, EFishRarity::Common, 7, 6},
    {"Roach", "Not the bug. Somehow less appealing.", nullptr, 5, 5, EFishRarity::Common, 8, 5},
    {"Bream", "Dreams of being a bass. Won't happen.", nullptr, 8, 6, EFishRarity::Common, 9, 7},
    {"Goldfish", "Someone's ex-pet. Thriving out of spite.", nullptr, 3, 4, EFishRarity::Common, 10, 5},
    {"Gudgeon", "Looks perpetually confused. Fair enough.", nullptr, 4, 4, EFishRarity::Common, 11, 5},
    
    // uncommon (20-40 exp)
    {"Bass", "Thinks it's better than other fish. It's not.", nullptr, 25, 10, EFishRarity::Uncommon, 12, 20},
    {"Trout", "Cold water snob with fancy spots.", nullptr, 30, 12, EFishRarity::Uncommon, 13, 25},
    {"Catfish", "Has whiskers. Still can't find its keys.", nullptr, 35, 14, EFishRarity::Uncommon, 14, 30},
    {"Walleye", "Those eyes have seen things. Terrible things.", nullptr, 40, 13, EFishRarity::Uncommon, 15, 32},
    {"Crappie", "Terrible name. Decent fish. Life's unfair.", nullptr, 28, 11, EFishRarity::Uncommon, 16, 22},
    {"Eel", "A snake that chose violence and water.", nullptr, 45, 15, EFishRarity::Uncommon, 17, 35},
    {"Tench", "Slimy green doctor fish. No medical license.", nullptr, 32, 12, EFishRarity::Uncommon, 18, 28},
    {"Zander", "European walleye. Equally judgmental.", nullptr, 38, 13, EFishRarity::Uncommon, 19, 30},
    {"Grayling", "Smells like thyme. Nature is weird.", nullptr, 35, 12, EFishRarity::Uncommon, 20, 28},
    {"Barbel", "Those barbels aren't just for show. Okay, they are.", nullptr, 42, 14, EFishRarity::Uncommon, 21, 35},
    
    // rare (50-100 exp)
    {"Pike", "All teeth and bad attitude.", nullptr, 75, 18, EFishRarity::Rare, 22, 50},
    {"Salmon", "Swam upstream its whole life. For what?", nullptr, 100, 20, EFishRarity::Rare, 23, 65},
    {"Sturgeon", "Older than your grandma. Uglier too.", nullptr, 120, 22, EFishRarity::Rare, 24, 75},
    {"Muskie", "The fish equivalent of a final boss.", nullptr, 150, 24, EFishRarity::Rare, 25, 90},
    {"Arctic Char", "Too cool for warm water. Literally.", nullptr, 90, 19, EFishRarity::Rare, 26, 55},
    {"Lake Trout", "Regular trout's successful older sibling.", nullptr, 110, 21, EFishRarity::Rare, 27, 70},
    {"Arapaima", "Breathes air. Still drowning in debt.", nullptr, 200, 25, EFishRarity::Rare, 28, 100},
    {"Taimen", "Siberian river monster. Very hangry.", nullptr, 180, 24, EFishRarity::Rare, 29, 85},
    
    // legendary (150-300 exp)
    {"Golden Carp", "Grants wishes. None of them good.", nullptr, 500, 30, EFishRarity::Legendary, 30, 150},
    {"Ghost Koi", "Died and came back. Still got caught.", nullptr, 750, 35, EFishRarity::Legendary, 31, 200},
    {"Ancient Lungfish", "Remembers when water was invented.", nullptr, 1000, 40, EFishRarity::Legendary, 32, 250},
    {"Void Bass", "Stares into you. You blink first.", nullptr, 888, 38, EFishRarity::Legendary, 33, 225},
    {"Crystal Perch", "Made of ice. Melts your heart. And hands.", nullptr, 666, 33, EFishRarity::Legendary, 34, 175},
    {"The Old One", "Was here before the lake. Will outlast it.", nullptr, 1500, 45, EFishRarity::Legendary, 35, 300},
};

static const int sFishPoolCount = sizeof(sFishPool) / sizeof(sFishPool[0]);

static const SItemGetData sExampleBait = {
    .name = "Earthworm",
    .description = "A wriggly earthworm. Fish love these!",
    .modelPath = nullptr,  // TODO: add worm model......
    .inventoryTab = EMenuTab::Bait,
    .quantity = 5,
    .iconIndex = 0,
    .exp = 0,
    .fishIndex = -1
};

static const SItemGetData sExampleRod = {
    .name = "Wooden Rod",
    .description = "A basic fishing rod made of oak.",
    .modelPath = "rom:/rod-redgolden.t3dm",
    .inventoryTab = EMenuTab::FishingRods,
    .quantity = 1,
    .iconIndex = 0,
    .exp = 0,
    .fishIndex = -1
};

static const SShopItem sShopItems[] = {
    {"Oak Rod", "A sturdy fishing rod.", "rom:/rod-redgolden.t3dm", 50, EMenuTab::FishingRods, 0},
    {"Steel Rod", "A premium fishing rod.", "rom:/rod-redgolden.t3dm", 150, EMenuTab::FishingRods, 1},
    {"Worm", "Basic bait for small fish.", nullptr, 5, EMenuTab::Bait, 0},
    {"Minnow", "Live bait for bigger fish.", nullptr, 15, EMenuTab::Bait, 1},
    {"Map", "Shows the fishing areas.", nullptr, 25, EMenuTab::MiscItems, 7},
    
    {"Engine Belt", "Replacement belt for snowmobile.", nullptr, 500, EMenuTab::MiscItems, 8},
    {"Spark Plugs", "High-performance spark plugs.", nullptr, 250, EMenuTab::MiscItems, 9},
    {"Carburetor Kit", "Complete carburetor rebuild kit.", nullptr, 800, EMenuTab::MiscItems, 10},
    {"Drive Chain", "Heavy-duty drive chain.", nullptr, 650, EMenuTab::MiscItems, 11},
    {"Fuel Line", "Reinforced fuel line assembly.", nullptr, 300, EMenuTab::MiscItems, 12},
    {"Skis", "Replacement skis for steering.", nullptr, 1200, EMenuTab::MiscItems, 13},
    {"Track", "New rubber track.", nullptr, 2500, EMenuTab::MiscItems, 14},
};


void villageOnInit(CScene& scene);
void villageOnUpdate(CScene& scene, float dt);
void villageOnExit(CScene& scene);

void shopCabinOnInit(CScene& scene);
void shopCabinOnUpdate(CScene& scene, float dt);
void shopCabinOnExit(CScene& scene);


static const SSceneObjectDef sVillageObjects[] = {

    
    /*
    SCENE_OBJECT_NPC("npc1", "rom:/npc-tiger.t3dm", "idle",
        18.2f, -10.8f, 185.6f,     // position
        0.0f, -90.0f, 0.0f,       // rotation
        0.125f, 0.125f, 0.125f, // scale
        50.0f,                    // collision radius
        true),                   // has interaction
    

    */

    SCENE_OBJECT_INTERACTABLE("teleport-cabin", nullptr, 669.7f, 14.2f, 133.1f, 30.0f),
    
};

static const SSceneObjectDef sCabinObjects[] = {


    SCENE_OBJECT_NPC("shopkeep", "rom:/shopkeep.t3dm", "idle",
        -120.8f, 0.0f, 71.7f,     // position
        0.0f, 90.0f, 0.0f,       // rotation
        0.125f, 0.125f, 0.125f, // scale
        100.0f,                    // collision radius
        true),                   // has interaction
    


    SCENE_OBJECT_INTERACTABLE("teleport-village", nullptr, -11.9f, 0.0f, -221.2f, 30.0f),
};


static const SSceneDef sVillageSceneDef = {
    .name = "village",
    .mapModelPath = "rom:/village.t3dm",
    .collisionPath = "rom:/village.bcol",
    .playerSpawnPos = {-50.4f, 0.0f, -230.1f},
    .playerSpawnRotY = 0.0f,
    .onInit = villageOnInit,
    .onUpdate = villageOnUpdate,
    .onExit = villageOnExit,
    .objects = sVillageObjects,
    .objectCount = sizeof(sVillageObjects) / sizeof(sVillageObjects[0]),
};

static const SSceneDef sVillageCabinExitSceneDef = {
    .name = "village",
    .mapModelPath = "rom:/village.t3dm",
    .collisionPath = "rom:/village.bcol",
    .playerSpawnPos = {589.3f, 14.2f, 133.0f},
    .playerSpawnRotY = 80.0f,
    .onInit = villageOnInit,
    .onUpdate = villageOnUpdate,
    .onExit = villageOnExit,
    .objects = sVillageObjects,
    .objectCount = sizeof(sVillageObjects) / sizeof(sVillageObjects[0]),
};

static const SSceneDef sTestSceneDef = {
    .name = "testmap00",
    .mapModelPath = "rom:/scabin.t3dm",
    .collisionPath = "rom:/scabin.bcol",
    .playerSpawnPos = {-13.4f, 0.0f, -188.9f},
    .playerSpawnRotY = 0.0f,
    .onInit = shopCabinOnInit,
    .onUpdate = shopCabinOnUpdate,
    .onExit = shopCabinOnExit,
    .objects = sCabinObjects,
    .objectCount = sizeof(sCabinObjects) / sizeof(sCabinObjects[0]),
};

inline void villageOnInit(CScene& scene)
{
    debugf("Village scene initialized!\n");
}

inline void villageOnUpdate(CScene& scene, float dt)
{
    CSceneObject* teleportCabin = scene.getObject("teleport-cabin");
    if (teleportCabin && teleportCabin->hasInteraction()) {
        CPlayer* player = CSceneManager::instance().getPlayer();
        if (player) {
            TVec3F playerPos = player->getPosition();
            
            if (teleportCabin->checkPlayerInRange(playerPos)) {
                CSoundMgr::stop(2);
                CSceneManager::instance().transitionToSceneStar(sTestSceneDef, 0.5f, 0.5f);
            }
        }
    }
}

inline void villageOnExit(CScene& scene)
{
    //debugf("Leaving village scene\n");
}

inline void shopCabinOnInit(CScene& scene)
{
    mixer_ch_set_vol(2, 1.0f, 1.0f);
    CSoundMgr::play("p2m", true, 2);
    
    CSceneManager& mgr = CSceneManager::instance();
    CShop* shop = mgr.getShop();
    if (shop) {
        shop->clearShopItems();
        shop->addShopItems(sShopItems);
    }
    
    CSceneObject* obj = scene.getObject("shopkeep");
    if (obj) {
        CNpcObject* npc = dynamic_cast<CNpcObject*>(obj);
        if (npc) {
            // check if player has met the shopkeeper before
            CPlayer* player = mgr.getPlayer();
            CMenu* menu = player ? player->getMenu() : nullptr;
            bool metBefore = menu && menu->getPlayerStats().hasStoryFlag(EStoryFlag::MetShopkeeper);
            
            if (metBefore) {
                npc->setDialogueLines({
                    "Welcome to my shop!",
                    SDialogueLine("Would you like to buy or sell?", {
                        SDialogueChoice("Buy", EDialogueAction::OpenShop, (void*)((uintptr_t)EShopMode::Buying + 1)),
                        SDialogueChoice("Sell", EDialogueAction::OpenShop, (void*)((uintptr_t)EShopMode::Selling + 1))
                    }),
                    "Come back anytime!"
                });
            } else {
                npc->setDialogueLines({
                    "Ah, hello there! You seem new around here...",
                    "Let me introduce myself. I'm Wolly, the shopkeeper.",
                    "If you're looking for fishing gear, you've come to the right place!",
                    "...Oh, I see. You need new parts to repair your snowmobile.",
                    "You're in luck! I happen to have some in stock, but they're a bit pricey.",
                    "Tell you what, let me help you out.",
                    { SDialogueLine("", EDialogueAction::GiveItem, (void*)&sExampleRod) },
                    "You can exchange fish for gold at my shop. Come here whenever you need equipment.",
                    "...",
                    "Oops, almost forgot...",
                    { SDialogueLine("", EDialogueAction::GiveItem, (void*)&sExampleBait) },
                    { SDialogueLine("Good luck out there!", EDialogueAction::SetStoryFlag, (void*)(uintptr_t)EStoryFlag::MetShopkeeper) }
                });
            }
        }
    }
}

inline void shopCabinOnUpdate(CScene& scene, float dt)
{
    // check if shopkeeper dialogue needs updating after first meeting
    static bool dialogueUpdated = false;
    CSceneManager& mgr = CSceneManager::instance();
    CPlayer* player = mgr.getPlayer();
    CMenu* menu = player ? player->getMenu() : nullptr;
    
    if (!dialogueUpdated && menu && menu->getPlayerStats().hasStoryFlag(EStoryFlag::MetShopkeeper)) {
        CSceneObject* obj = scene.getObject("shopkeep");
        if (obj) {
            CNpcObject* npc = dynamic_cast<CNpcObject*>(obj);
            if (npc) {
                npc->setDialogueLines({
                    "Welcome to my shop!",
                    SDialogueLine("Would you like to buy or sell?", {
                        SDialogueChoice("Buy", EDialogueAction::OpenShop, (void*)((uintptr_t)EShopMode::Buying + 1)),
                        SDialogueChoice("Sell", EDialogueAction::OpenShop, (void*)((uintptr_t)EShopMode::Selling + 1))
                    }),
                    "Come back anytime!"
                });
                dialogueUpdated = true;
            }
        }
    }
    
    CSceneObject* teleportCabin = scene.getObject("teleport-village");
    if (teleportCabin && teleportCabin->hasInteraction()) {
        if (player) {
            TVec3F playerPos = player->getPosition();
            
            if (teleportCabin->checkPlayerInRange(playerPos)) {
                CSoundMgr::stop(2);
                dialogueUpdated = false;  // reset for next visit
                CSceneManager::instance().transitionToSceneStar(sVillageCabinExitSceneDef, 0.5f, 0.5f);
            }
        }
    }
}

inline void shopCabinOnExit(CScene& scene)
{
    //debugf("Leaving village scene\n");
}

static const SSceneDef sEmptySceneDef = {
    .name = "empty",
    .mapModelPath = nullptr,
    .collisionPath = nullptr,
    .playerSpawnPos = {0.0f, 0.0f, 0.0f},
    .playerSpawnRotY = 0.0f,
    .onInit = nullptr,
    .onUpdate = nullptr,
    .onExit = nullptr,
    .objects = nullptr,
    .objectCount = 0,
};

inline void exampleCutsceneInit()
{
    debugf("Cutscene started!\n");
}

inline void exampleCutsceneEnd()
{
    debugf("Cutscene ended!\n");
}

// automatically exported from blender:
static const SCutsceneCameraFrame sExampleCutsceneFrames[] = {
    // Frame 1: 0.00s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.000f,
        .position = {-53.073f, 28.904f, -329.122f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 2: 0.03s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.033f,
        .position = {-53.071f, 28.900f, -329.032f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 3: 0.07s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.067f,
        .position = {-53.069f, 28.896f, -328.941f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 4: 0.10s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.100f,
        .position = {-53.067f, 28.892f, -328.851f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 5: 0.13s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.133f,
        .position = {-53.065f, 28.889f, -328.760f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 6: 0.17s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.167f,
        .position = {-53.063f, 28.885f, -328.670f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 7: 0.20s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.200f,
        .position = {-53.061f, 28.881f, -328.579f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 8: 0.23s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.233f,
        .position = {-53.059f, 28.877f, -328.489f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 9: 0.27s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.267f,
        .position = {-53.057f, 28.874f, -328.398f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 10: 0.30s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.300f,
        .position = {-53.055f, 28.870f, -328.308f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 11: 0.33s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.333f,
        .position = {-53.053f, 28.866f, -328.217f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 12: 0.37s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.367f,
        .position = {-53.051f, 28.862f, -328.127f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 13: 0.40s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.400f,
        .position = {-53.049f, 28.858f, -328.036f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 14: 0.43s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.433f,
        .position = {-53.047f, 28.855f, -327.946f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 15: 0.47s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.467f,
        .position = {-53.046f, 28.851f, -327.855f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 16: 0.50s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.500f,
        .position = {-53.044f, 28.847f, -327.765f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 17: 0.53s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.533f,
        .position = {-53.042f, 28.843f, -327.674f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 18: 0.57s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.567f,
        .position = {-53.040f, 28.839f, -327.584f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 19: 0.60s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.600f,
        .position = {-53.038f, 28.836f, -327.493f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 20: 0.63s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.633f,
        .position = {-53.036f, 28.832f, -327.403f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 21: 0.67s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.667f,
        .position = {-53.034f, 28.828f, -327.312f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 22: 0.70s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.700f,
        .position = {-53.032f, 28.824f, -327.222f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 23: 0.73s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.733f,
        .position = {-53.030f, 28.820f, -327.131f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 24: 0.77s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.767f,
        .position = {-53.028f, 28.817f, -327.041f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 25: 0.80s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.800f,
        .position = {-53.026f, 28.813f, -326.950f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 26: 0.83s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.833f,
        .position = {-53.024f, 28.809f, -326.860f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 27: 0.87s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.867f,
        .position = {-53.022f, 28.805f, -326.769f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 28: 0.90s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.900f,
        .position = {-53.020f, 28.801f, -326.679f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 29: 0.93s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.933f,
        .position = {-53.018f, 28.798f, -326.588f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 30: 0.97s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 0.967f,
        .position = {-53.017f, 28.794f, -326.498f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 31: 1.00s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.000f,
        .position = {-53.015f, 28.790f, -326.407f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 32: 1.03s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.033f,
        .position = {-53.013f, 28.786f, -326.317f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 33: 1.07s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.067f,
        .position = {-53.011f, 28.782f, -326.226f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 34: 1.10s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.100f,
        .position = {-53.009f, 28.779f, -326.136f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 35: 1.13s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.133f,
        .position = {-53.007f, 28.775f, -326.045f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 36: 1.17s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.167f,
        .position = {-53.005f, 28.771f, -325.955f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 37: 1.20s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.200f,
        .position = {-53.003f, 28.767f, -325.864f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 38: 1.23s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.233f,
        .position = {-53.001f, 28.764f, -325.774f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 39: 1.27s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.267f,
        .position = {-52.999f, 28.760f, -325.683f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 40: 1.30s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.300f,
        .position = {-52.997f, 28.756f, -325.593f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 41: 1.33s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.333f,
        .position = {-52.995f, 28.752f, -325.502f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 42: 1.37s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.367f,
        .position = {-52.993f, 28.748f, -325.412f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 43: 1.40s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.400f,
        .position = {-52.991f, 28.745f, -325.321f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 44: 1.43s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.433f,
        .position = {-52.989f, 28.741f, -325.231f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 45: 1.47s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.467f,
        .position = {-52.988f, 28.737f, -325.140f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 46: 1.50s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.500f,
        .position = {-52.986f, 28.733f, -325.050f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 47: 1.53s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.533f,
        .position = {-52.984f, 28.729f, -324.959f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 48: 1.57s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.567f,
        .position = {-52.982f, 28.726f, -324.869f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 49: 1.60s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.600f,
        .position = {-52.980f, 28.722f, -324.778f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 50: 1.63s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.633f,
        .position = {-52.978f, 28.718f, -324.688f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 51: 1.67s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.667f,
        .position = {-52.976f, 28.714f, -324.597f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 52: 1.70s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.700f,
        .position = {-52.974f, 28.710f, -324.507f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 53: 1.73s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.733f,
        .position = {-52.972f, 28.707f, -324.416f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 54: 1.77s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.767f,
        .position = {-52.970f, 28.703f, -324.326f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 55: 1.80s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.800f,
        .position = {-52.968f, 28.699f, -324.235f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 56: 1.83s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.833f,
        .position = {-52.966f, 28.695f, -324.145f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 57: 1.87s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.867f,
        .position = {-52.964f, 28.691f, -324.054f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 58: 1.90s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.900f,
        .position = {-52.962f, 28.688f, -323.964f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 59: 1.93s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.933f,
        .position = {-52.960f, 28.684f, -323.873f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 60: 1.97s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 1.967f,
        .position = {-52.958f, 28.680f, -323.783f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 61: 2.00s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.000f,
        .position = {-52.957f, 28.676f, -323.692f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 62: 2.03s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.033f,
        .position = {-52.955f, 28.673f, -323.602f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 63: 2.07s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.067f,
        .position = {-52.953f, 28.669f, -323.511f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 64: 2.10s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.100f,
        .position = {-52.951f, 28.665f, -323.421f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 65: 2.13s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.133f,
        .position = {-52.949f, 28.661f, -323.330f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 66: 2.17s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.167f,
        .position = {-52.947f, 28.657f, -323.240f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 67: 2.20s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.200f,
        .position = {-52.945f, 28.654f, -323.149f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 68: 2.23s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.233f,
        .position = {-52.943f, 28.650f, -323.059f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 69: 2.27s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.267f,
        .position = {-52.941f, 28.646f, -322.968f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 70: 2.30s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.300f,
        .position = {-52.939f, 28.642f, -322.878f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 71: 2.33s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.333f,
        .position = {-52.937f, 28.638f, -322.787f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 72: 2.37s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.367f,
        .position = {-52.935f, 28.635f, -322.697f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 73: 2.40s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.400f,
        .position = {-52.933f, 28.631f, -322.606f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 74: 2.43s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.433f,
        .position = {-52.931f, 28.627f, -322.516f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 75: 2.47s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.467f,
        .position = {-52.929f, 28.623f, -322.425f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 76: 2.50s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.500f,
        .position = {-52.928f, 28.619f, -322.335f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 77: 2.53s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.533f,
        .position = {-52.926f, 28.616f, -322.244f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 78: 2.57s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.567f,
        .position = {-52.924f, 28.612f, -322.154f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 79: 2.60s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.600f,
        .position = {-52.922f, 28.608f, -322.063f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 80: 2.63s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.633f,
        .position = {-52.920f, 28.604f, -321.973f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 81: 2.67s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.667f,
        .position = {-52.918f, 28.600f, -321.882f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 82: 2.70s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.700f,
        .position = {-52.916f, 28.597f, -321.792f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 83: 2.73s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.733f,
        .position = {-52.914f, 28.593f, -321.701f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 84: 2.77s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.767f,
        .position = {-52.912f, 28.589f, -321.611f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 85: 2.80s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.800f,
        .position = {-52.910f, 28.585f, -321.520f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 86: 2.83s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.833f,
        .position = {-52.908f, 28.581f, -321.430f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 87: 2.87s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.867f,
        .position = {-52.906f, 28.578f, -321.339f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 88: 2.90s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.900f,
        .position = {-52.904f, 28.574f, -321.249f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 89: 2.93s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.933f,
        .position = {-52.902f, 28.570f, -321.158f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 90: 2.97s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 2.967f,
        .position = {-52.900f, 28.566f, -321.068f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 91: 3.00s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 3.000f,
        .position = {-52.899f, 28.563f, -320.977f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 92: 3.03s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 3.033f,
        .position = {-52.897f, 28.559f, -320.887f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 93: 3.07s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 3.067f,
        .position = {-52.895f, 28.555f, -320.796f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 94: 3.10s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 3.100f,
        .position = {-52.893f, 28.551f, -320.706f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 95: 3.13s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 3.133f,
        .position = {-52.891f, 28.547f, -320.615f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 96: 3.17s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 3.167f,
        .position = {-52.889f, 28.544f, -320.525f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 97: 3.20s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 3.200f,
        .position = {-52.887f, 28.540f, -320.434f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 98: 3.23s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 3.233f,
        .position = {-52.885f, 28.536f, -320.344f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 99: 3.27s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 3.267f,
        .position = {-52.883f, 28.532f, -320.253f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 100: 3.30s (raw rot: 1.529, 0.000, -3.120)
    {
        .time = 3.300f,
        .position = {-52.881f, 28.528f, -320.163f},
        .rotation = {3.082f, -3.120f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = false
    },
    // Frame 101: 3.33s (raw rot: 1.480, 0.000, 0.014)
    {
        .time = 3.333f,
        .position = {-52.226f, 38.143f, -211.432f},
        .rotation = {3.033f, 0.014f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 102: 3.37s (raw rot: 1.481, 0.000, 0.013)
    {
        .time = 3.367f,
        .position = {-52.117f, 38.127f, -211.384f},
        .rotation = {3.034f, 0.013f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 103: 3.40s (raw rot: 1.484, 0.000, 0.011)
    {
        .time = 3.400f,
        .position = {-52.022f, 38.082f, -211.343f},
        .rotation = {3.037f, 0.011f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 104: 3.43s (raw rot: 1.488, 0.000, 0.010)
    {
        .time = 3.433f,
        .position = {-51.941f, 38.014f, -211.308f},
        .rotation = {3.041f, 0.010f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 105: 3.47s (raw rot: 1.493, 0.000, 0.010)
    {
        .time = 3.467f,
        .position = {-51.872f, 37.925f, -211.278f},
        .rotation = {3.046f, 0.010f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 106: 3.50s (raw rot: 1.499, 0.000, 0.009)
    {
        .time = 3.500f,
        .position = {-51.815f, 37.821f, -211.254f},
        .rotation = {3.052f, 0.009f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 107: 3.53s (raw rot: 1.505, 0.000, 0.008)
    {
        .time = 3.533f,
        .position = {-51.768f, 37.706f, -211.233f},
        .rotation = {3.059f, 0.008f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 108: 3.57s (raw rot: 1.512, 0.000, 0.008)
    {
        .time = 3.567f,
        .position = {-51.730f, 37.584f, -211.217f},
        .rotation = {3.066f, 0.008f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 109: 3.60s (raw rot: 1.520, 0.000, 0.007)
    {
        .time = 3.600f,
        .position = {-51.701f, 37.460f, -211.204f},
        .rotation = {3.073f, 0.007f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 110: 3.63s (raw rot: 1.527, 0.000, 0.007)
    {
        .time = 3.633f,
        .position = {-51.679f, 37.338f, -211.195f},
        .rotation = {3.080f, 0.007f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 111: 3.67s (raw rot: 1.533, 0.000, 0.007)
    {
        .time = 3.667f,
        .position = {-51.663f, 37.223f, -211.188f},
        .rotation = {3.087f, 0.007f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 112: 3.70s (raw rot: 1.539, 0.000, 0.007)
    {
        .time = 3.700f,
        .position = {-51.653f, 37.119f, -211.183f},
        .rotation = {3.093f, 0.007f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 113: 3.73s (raw rot: 1.544, 0.000, 0.007)
    {
        .time = 3.733f,
        .position = {-51.647f, 37.031f, -211.181f},
        .rotation = {3.098f, 0.007f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 114: 3.77s (raw rot: 1.548, 0.000, 0.007)
    {
        .time = 3.767f,
        .position = {-51.643f, 36.962f, -211.179f},
        .rotation = {3.102f, 0.007f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 115: 3.80s (raw rot: 1.551, 0.000, 0.007)
    {
        .time = 3.800f,
        .position = {-51.642f, 36.917f, -211.179f},
        .rotation = {3.104f, 0.007f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 116: 3.83s (raw rot: 1.552, 0.000, 0.007)
    {
        .time = 3.833f,
        .position = {-51.642f, 36.902f, -211.179f},
        .rotation = {3.105f, 0.007f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 117: 3.87s (raw rot: 1.551, 0.000, 0.007)
    {
        .time = 3.867f,
        .position = {-51.644f, 36.916f, -211.182f},
        .rotation = {3.104f, 0.007f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 118: 3.90s (raw rot: 1.548, 0.000, 0.008)
    {
        .time = 3.900f,
        .position = {-51.649f, 36.959f, -211.190f},
        .rotation = {3.101f, 0.008f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 119: 3.93s (raw rot: 1.544, 0.000, 0.010)
    {
        .time = 3.933f,
        .position = {-51.658f, 37.024f, -211.203f},
        .rotation = {3.097f, 0.010f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 120: 3.97s (raw rot: 1.538, 0.000, 0.012)
    {
        .time = 3.967f,
        .position = {-51.671f, 37.109f, -211.220f},
        .rotation = {3.092f, 0.012f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 121: 4.00s (raw rot: 1.532, 0.000, 0.015)
    {
        .time = 4.000f,
        .position = {-51.687f, 37.209f, -211.240f},
        .rotation = {3.085f, 0.015f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 122: 4.03s (raw rot: 1.525, 0.000, 0.018)
    {
        .time = 4.033f,
        .position = {-51.706f, 37.322f, -211.263f},
        .rotation = {3.078f, 0.018f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 123: 4.07s (raw rot: 1.517, 0.000, 0.021)
    {
        .time = 4.067f,
        .position = {-51.729f, 37.441f, -211.287f},
        .rotation = {3.070f, 0.021f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 124: 4.10s (raw rot: 1.509, 0.000, 0.024)
    {
        .time = 4.100f,
        .position = {-51.754f, 37.565f, -211.312f},
        .rotation = {3.062f, 0.024f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 125: 4.13s (raw rot: 1.501, 0.000, 0.028)
    {
        .time = 4.133f,
        .position = {-51.782f, 37.689f, -211.337f},
        .rotation = {3.054f, 0.028f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 126: 4.17s (raw rot: 1.493, 0.000, 0.031)
    {
        .time = 4.167f,
        .position = {-51.813f, 37.809f, -211.361f},
        .rotation = {3.047f, 0.031f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 127: 4.20s (raw rot: 1.486, 0.000, 0.034)
    {
        .time = 4.200f,
        .position = {-51.847f, 37.921f, -211.383f},
        .rotation = {3.039f, 0.034f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 128: 4.23s (raw rot: 1.479, 0.000, 0.037)
    {
        .time = 4.233f,
        .position = {-51.884f, 38.022f, -211.403f},
        .rotation = {3.033f, 0.037f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 129: 4.27s (raw rot: 1.474, 0.000, 0.039)
    {
        .time = 4.267f,
        .position = {-51.923f, 38.106f, -211.420f},
        .rotation = {3.027f, 0.039f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 130: 4.30s (raw rot: 1.470, 0.000, 0.041)
    {
        .time = 4.300f,
        .position = {-51.964f, 38.172f, -211.433f},
        .rotation = {3.023f, 0.041f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 131: 4.33s (raw rot: 1.467, 0.000, 0.042)
    {
        .time = 4.333f,
        .position = {-52.008f, 38.214f, -211.442f},
        .rotation = {3.020f, 0.042f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 132: 4.37s (raw rot: 1.466, 0.000, 0.042)
    {
        .time = 4.367f,
        .position = {-52.054f, 38.229f, -211.445f},
        .rotation = {3.019f, 0.042f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 133: 4.40s (raw rot: 1.467, 0.000, 0.041)
    {
        .time = 4.400f,
        .position = {-52.101f, 38.226f, -211.445f},
        .rotation = {3.020f, 0.041f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 134: 4.43s (raw rot: 1.468, 0.000, 0.039)
    {
        .time = 4.433f,
        .position = {-52.151f, 38.217f, -211.444f},
        .rotation = {3.021f, 0.039f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 135: 4.47s (raw rot: 1.470, 0.000, 0.034)
    {
        .time = 4.467f,
        .position = {-52.201f, 38.203f, -211.444f},
        .rotation = {3.024f, 0.034f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 136: 4.50s (raw rot: 1.473, 0.000, 0.029)
    {
        .time = 4.500f,
        .position = {-52.253f, 38.184f, -211.443f},
        .rotation = {3.027f, 0.029f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 137: 4.53s (raw rot: 1.477, 0.000, 0.022)
    {
        .time = 4.533f,
        .position = {-52.304f, 38.162f, -211.442f},
        .rotation = {3.030f, 0.022f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 138: 4.57s (raw rot: 1.481, 0.000, 0.015)
    {
        .time = 4.567f,
        .position = {-52.355f, 38.136f, -211.441f},
        .rotation = {3.034f, 0.015f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 139: 4.60s (raw rot: 1.486, 0.000, 0.006)
    {
        .time = 4.600f,
        .position = {-52.406f, 38.109f, -211.440f},
        .rotation = {3.039f, 0.006f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 140: 4.63s (raw rot: 1.490, 0.000, -0.002)
    {
        .time = 4.633f,
        .position = {-52.455f, 38.079f, -211.439f},
        .rotation = {3.044f, -0.002f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 141: 4.67s (raw rot: 1.495, 0.000, -0.012)
    {
        .time = 4.667f,
        .position = {-52.503f, 38.049f, -211.438f},
        .rotation = {3.049f, -0.012f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 142: 4.70s (raw rot: 1.500, 0.000, -0.021)
    {
        .time = 4.700f,
        .position = {-52.549f, 38.018f, -211.437f},
        .rotation = {3.054f, -0.021f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 143: 4.73s (raw rot: 1.505, 0.000, -0.030)
    {
        .time = 4.733f,
        .position = {-52.592f, 37.987f, -211.435f},
        .rotation = {3.058f, -0.030f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 144: 4.77s (raw rot: 1.510, 0.000, -0.039)
    {
        .time = 4.767f,
        .position = {-52.633f, 37.958f, -211.434f},
        .rotation = {3.063f, -0.039f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 145: 4.80s (raw rot: 1.514, 0.000, -0.047)
    {
        .time = 4.800f,
        .position = {-52.670f, 37.930f, -211.433f},
        .rotation = {3.068f, -0.047f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 146: 4.83s (raw rot: 1.518, 0.000, -0.055)
    {
        .time = 4.833f,
        .position = {-52.703f, 37.904f, -211.432f},
        .rotation = {3.072f, -0.055f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 147: 4.87s (raw rot: 1.522, 0.000, -0.062)
    {
        .time = 4.867f,
        .position = {-52.731f, 37.882f, -211.431f},
        .rotation = {3.075f, -0.062f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 148: 4.90s (raw rot: 1.525, 0.000, -0.068)
    {
        .time = 4.900f,
        .position = {-52.755f, 37.864f, -211.430f},
        .rotation = {3.078f, -0.068f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 149: 4.93s (raw rot: 1.527, 0.000, -0.073)
    {
        .time = 4.933f,
        .position = {-52.774f, 37.850f, -211.430f},
        .rotation = {3.081f, -0.073f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 150: 4.97s (raw rot: 1.529, 0.000, -0.076)
    {
        .time = 4.967f,
        .position = {-52.786f, 37.841f, -211.430f},
        .rotation = {3.082f, -0.076f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 151: 5.00s (raw rot: 1.529, 0.000, -0.077)
    {
        .time = 5.000f,
        .position = {-52.793f, 37.837f, -211.429f},
        .rotation = {3.083f, -0.077f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 152: 5.03s (raw rot: 1.529, 0.000, -0.078)
    {
        .time = 5.033f,
        .position = {-52.796f, 37.842f, -211.430f},
        .rotation = {3.082f, -0.078f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 153: 5.07s (raw rot: 1.527, 0.000, -0.078)
    {
        .time = 5.067f,
        .position = {-52.799f, 37.855f, -211.431f},
        .rotation = {3.080f, -0.078f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 154: 5.10s (raw rot: 1.523, 0.000, -0.079)
    {
        .time = 5.100f,
        .position = {-52.802f, 37.874f, -211.433f},
        .rotation = {3.077f, -0.079f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 155: 5.13s (raw rot: 1.519, 0.000, -0.079)
    {
        .time = 5.133f,
        .position = {-52.804f, 37.900f, -211.435f},
        .rotation = {3.073f, -0.079f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 156: 5.17s (raw rot: 1.514, 0.000, -0.080)
    {
        .time = 5.167f,
        .position = {-52.806f, 37.931f, -211.438f},
        .rotation = {3.068f, -0.080f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 157: 5.20s (raw rot: 1.509, 0.000, -0.080)
    {
        .time = 5.200f,
        .position = {-52.807f, 37.965f, -211.441f},
        .rotation = {3.062f, -0.080f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 158: 5.23s (raw rot: 1.503, 0.000, -0.080)
    {
        .time = 5.233f,
        .position = {-52.808f, 38.002f, -211.445f},
        .rotation = {3.056f, -0.080f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 159: 5.27s (raw rot: 1.496, 0.000, -0.080)
    {
        .time = 5.267f,
        .position = {-52.809f, 38.041f, -211.448f},
        .rotation = {3.050f, -0.080f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 160: 5.30s (raw rot: 1.490, 0.000, -0.081)
    {
        .time = 5.300f,
        .position = {-52.810f, 38.080f, -211.452f},
        .rotation = {3.043f, -0.081f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 161: 5.33s (raw rot: 1.484, 0.000, -0.081)
    {
        .time = 5.333f,
        .position = {-52.811f, 38.119f, -211.456f},
        .rotation = {3.037f, -0.081f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 162: 5.37s (raw rot: 1.478, 0.000, -0.081)
    {
        .time = 5.367f,
        .position = {-52.811f, 38.156f, -211.459f},
        .rotation = {3.031f, -0.081f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 163: 5.40s (raw rot: 1.472, 0.000, -0.081)
    {
        .time = 5.400f,
        .position = {-52.812f, 38.191f, -211.462f},
        .rotation = {3.026f, -0.081f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 164: 5.43s (raw rot: 1.467, 0.000, -0.081)
    {
        .time = 5.433f,
        .position = {-52.812f, 38.221f, -211.465f},
        .rotation = {3.021f, -0.081f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 165: 5.47s (raw rot: 1.463, 0.000, -0.081)
    {
        .time = 5.467f,
        .position = {-52.812f, 38.247f, -211.467f},
        .rotation = {3.016f, -0.081f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 166: 5.50s (raw rot: 1.460, 0.000, -0.081)
    {
        .time = 5.500f,
        .position = {-52.812f, 38.267f, -211.469f},
        .rotation = {3.013f, -0.081f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 167: 5.53s (raw rot: 1.458, 0.000, -0.081)
    {
        .time = 5.533f,
        .position = {-52.812f, 38.279f, -211.470f},
        .rotation = {3.011f, -0.081f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 168: 5.57s (raw rot: 1.457, 0.000, -0.081)
    {
        .time = 5.567f,
        .position = {-52.812f, 38.284f, -211.471f},
        .rotation = {3.010f, -0.081f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 169: 5.60s (raw rot: 1.458, 0.000, -0.080)
    {
        .time = 5.600f,
        .position = {-52.808f, 38.281f, -211.471f},
        .rotation = {3.011f, -0.080f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 170: 5.63s (raw rot: 1.459, 0.000, -0.078)
    {
        .time = 5.633f,
        .position = {-52.797f, 38.275f, -211.470f},
        .rotation = {3.012f, -0.078f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 171: 5.67s (raw rot: 1.460, 0.000, -0.075)
    {
        .time = 5.667f,
        .position = {-52.779f, 38.264f, -211.468f},
        .rotation = {3.014f, -0.075f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 172: 5.70s (raw rot: 1.463, 0.000, -0.072)
    {
        .time = 5.700f,
        .position = {-52.755f, 38.251f, -211.466f},
        .rotation = {3.016f, -0.072f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 173: 5.73s (raw rot: 1.465, 0.000, -0.067)
    {
        .time = 5.733f,
        .position = {-52.726f, 38.234f, -211.464f},
        .rotation = {3.019f, -0.067f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 174: 5.77s (raw rot: 1.468, 0.000, -0.062)
    {
        .time = 5.767f,
        .position = {-52.692f, 38.215f, -211.462f},
        .rotation = {3.022f, -0.062f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 175: 5.80s (raw rot: 1.472, 0.000, -0.056)
    {
        .time = 5.800f,
        .position = {-52.656f, 38.194f, -211.459f},
        .rotation = {3.025f, -0.056f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 176: 5.83s (raw rot: 1.475, 0.000, -0.049)
    {
        .time = 5.833f,
        .position = {-52.617f, 38.171f, -211.456f},
        .rotation = {3.029f, -0.049f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 177: 5.87s (raw rot: 1.479, 0.000, -0.043)
    {
        .time = 5.867f,
        .position = {-52.576f, 38.148f, -211.453f},
        .rotation = {3.033f, -0.043f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 178: 5.90s (raw rot: 1.483, 0.000, -0.036)
    {
        .time = 5.900f,
        .position = {-52.535f, 38.124f, -211.449f},
        .rotation = {3.036f, -0.036f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 179: 5.93s (raw rot: 1.487, 0.000, -0.029)
    {
        .time = 5.933f,
        .position = {-52.493f, 38.100f, -211.446f},
        .rotation = {3.040f, -0.029f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 180: 5.97s (raw rot: 1.491, 0.000, -0.023)
    {
        .time = 5.967f,
        .position = {-52.453f, 38.077f, -211.443f},
        .rotation = {3.044f, -0.023f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 181: 6.00s (raw rot: 1.494, 0.000, -0.016)
    {
        .time = 6.000f,
        .position = {-52.414f, 38.054f, -211.440f},
        .rotation = {3.048f, -0.016f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 182: 6.03s (raw rot: 1.498, 0.000, -0.010)
    {
        .time = 6.033f,
        .position = {-52.377f, 38.033f, -211.437f},
        .rotation = {3.051f, -0.010f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 183: 6.07s (raw rot: 1.501, 0.000, -0.005)
    {
        .time = 6.067f,
        .position = {-52.344f, 38.014f, -211.435f},
        .rotation = {3.054f, -0.005f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 184: 6.10s (raw rot: 1.504, 0.000, -0.000)
    {
        .time = 6.100f,
        .position = {-52.315f, 37.997f, -211.432f},
        .rotation = {3.057f, -0.000f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 185: 6.13s (raw rot: 1.506, 0.000, 0.004)
    {
        .time = 6.133f,
        .position = {-52.291f, 37.983f, -211.430f},
        .rotation = {3.059f, 0.004f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 186: 6.17s (raw rot: 1.507, 0.000, 0.006)
    {
        .time = 6.167f,
        .position = {-52.273f, 37.973f, -211.429f},
        .rotation = {3.061f, 0.006f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 187: 6.20s (raw rot: 1.509, 0.000, 0.008)
    {
        .time = 6.200f,
        .position = {-52.261f, 37.966f, -211.428f},
        .rotation = {3.062f, 0.008f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 188: 6.23s (raw rot: 1.509, 0.000, 0.009)
    {
        .time = 6.233f,
        .position = {-52.257f, 37.964f, -211.428f},
        .rotation = {3.062f, 0.009f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 189: 6.27s (raw rot: 1.508, 0.000, 0.009)
    {
        .time = 6.267f,
        .position = {-52.258f, 37.967f, -211.428f},
        .rotation = {3.062f, 0.009f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 190: 6.30s (raw rot: 1.507, 0.000, 0.009)
    {
        .time = 6.300f,
        .position = {-52.258f, 37.977f, -211.429f},
        .rotation = {3.060f, 0.009f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 191: 6.33s (raw rot: 1.504, 0.000, 0.009)
    {
        .time = 6.333f,
        .position = {-52.259f, 37.992f, -211.430f},
        .rotation = {3.058f, 0.009f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 192: 6.37s (raw rot: 1.501, 0.000, 0.009)
    {
        .time = 6.367f,
        .position = {-52.259f, 38.011f, -211.431f},
        .rotation = {3.055f, 0.009f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 193: 6.40s (raw rot: 1.498, 0.000, 0.009)
    {
        .time = 6.400f,
        .position = {-52.260f, 38.034f, -211.433f},
        .rotation = {3.051f, 0.009f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 194: 6.43s (raw rot: 1.493, 0.000, 0.008)
    {
        .time = 6.433f,
        .position = {-52.261f, 38.060f, -211.435f},
        .rotation = {3.047f, 0.008f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 195: 6.47s (raw rot: 1.489, 0.000, 0.008)
    {
        .time = 6.467f,
        .position = {-52.262f, 38.088f, -211.436f},
        .rotation = {3.042f, 0.008f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 196: 6.50s (raw rot: 1.484, 0.000, 0.008)
    {
        .time = 6.500f,
        .position = {-52.264f, 38.117f, -211.439f},
        .rotation = {3.037f, 0.008f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 197: 6.53s (raw rot: 1.479, 0.000, 0.008)
    {
        .time = 6.533f,
        .position = {-52.265f, 38.147f, -211.441f},
        .rotation = {3.033f, 0.008f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 198: 6.57s (raw rot: 1.475, 0.000, 0.008)
    {
        .time = 6.567f,
        .position = {-52.266f, 38.176f, -211.443f},
        .rotation = {3.028f, 0.008f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 199: 6.60s (raw rot: 1.470, 0.000, 0.008)
    {
        .time = 6.600f,
        .position = {-52.267f, 38.204f, -211.445f},
        .rotation = {3.023f, 0.008f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 200: 6.63s (raw rot: 1.466, 0.000, 0.007)
    {
        .time = 6.633f,
        .position = {-52.268f, 38.230f, -211.446f},
        .rotation = {3.019f, 0.007f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 201: 6.67s (raw rot: 1.462, 0.000, 0.007)
    {
        .time = 6.667f,
        .position = {-52.269f, 38.253f, -211.448f},
        .rotation = {3.015f, 0.007f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 202: 6.70s (raw rot: 1.459, 0.000, 0.007)
    {
        .time = 6.700f,
        .position = {-52.270f, 38.273f, -211.449f},
        .rotation = {3.012f, 0.007f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 203: 6.73s (raw rot: 1.457, 0.000, 0.007)
    {
        .time = 6.733f,
        .position = {-52.270f, 38.288f, -211.450f},
        .rotation = {3.010f, 0.007f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 204: 6.77s (raw rot: 1.455, 0.000, 0.007)
    {
        .time = 6.767f,
        .position = {-52.271f, 38.297f, -211.451f},
        .rotation = {3.008f, 0.007f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 205: 6.80s (raw rot: 1.454, 0.000, 0.007)
    {
        .time = 6.800f,
        .position = {-52.271f, 38.301f, -211.451f},
        .rotation = {3.008f, 0.007f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 206: 6.83s (raw rot: 1.454, 0.000, 0.007)
    {
        .time = 6.833f,
        .position = {-52.270f, 38.297f, -211.451f},
        .rotation = {3.008f, 0.007f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 207: 6.87s (raw rot: 1.455, 0.000, 0.007)
    {
        .time = 6.867f,
        .position = {-52.268f, 38.289f, -211.450f},
        .rotation = {3.008f, 0.007f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 208: 6.90s (raw rot: 1.455, 0.000, 0.008)
    {
        .time = 6.900f,
        .position = {-52.264f, 38.276f, -211.448f},
        .rotation = {3.008f, 0.008f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 209: 6.93s (raw rot: 1.455, 0.000, 0.009)
    {
        .time = 6.933f,
        .position = {-52.259f, 38.260f, -211.446f},
        .rotation = {3.009f, 0.009f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 210: 6.97s (raw rot: 1.456, 0.000, 0.010)
    {
        .time = 6.967f,
        .position = {-52.254f, 38.241f, -211.444f},
        .rotation = {3.010f, 0.010f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 211: 7.00s (raw rot: 1.458, 0.000, 0.011)
    {
        .time = 7.000f,
        .position = {-52.248f, 38.222f, -211.441f},
        .rotation = {3.011f, 0.011f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 212: 7.03s (raw rot: 1.460, 0.000, 0.011)
    {
        .time = 7.033f,
        .position = {-52.243f, 38.202f, -211.439f},
        .rotation = {3.013f, 0.011f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 213: 7.07s (raw rot: 1.462, 0.000, 0.012)
    {
        .time = 7.067f,
        .position = {-52.238f, 38.184f, -211.437f},
        .rotation = {3.015f, 0.012f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 214: 7.10s (raw rot: 1.465, 0.000, 0.013)
    {
        .time = 7.100f,
        .position = {-52.233f, 38.167f, -211.435f},
        .rotation = {3.019f, 0.013f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 215: 7.13s (raw rot: 1.469, 0.000, 0.014)
    {
        .time = 7.133f,
        .position = {-52.229f, 38.154f, -211.433f},
        .rotation = {3.023f, 0.014f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 216: 7.17s (raw rot: 1.474, 0.000, 0.014)
    {
        .time = 7.167f,
        .position = {-52.227f, 38.146f, -211.432f},
        .rotation = {3.028f, 0.014f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 217: 7.20s (raw rot: 1.480, 0.000, 0.014)
    {
        .time = 7.200f,
        .position = {-52.226f, 38.143f, -211.432f},
        .rotation = {3.033f, 0.014f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = false
    },
    // Frame 218: 7.23s (raw rot: 1.592, 0.000, -1.738)
    {
        .time = 7.233f,
        .position = {-146.755f, 65.795f, -219.150f},
        .rotation = {3.145f, -1.738f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 219: 7.27s (raw rot: 1.592, 0.000, -1.738)
    {
        .time = 7.267f,
        .position = {-146.755f, 65.795f, -219.150f},
        .rotation = {3.145f, -1.738f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 220: 7.30s (raw rot: 1.592, 0.000, -1.738)
    {
        .time = 7.300f,
        .position = {-146.755f, 65.795f, -219.150f},
        .rotation = {3.145f, -1.738f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 221: 7.33s (raw rot: 1.592, 0.000, -1.738)
    {
        .time = 7.333f,
        .position = {-146.755f, 65.795f, -219.150f},
        .rotation = {3.145f, -1.738f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 222: 7.37s (raw rot: 1.592, 0.000, -1.738)
    {
        .time = 7.367f,
        .position = {-146.755f, 65.795f, -219.150f},
        .rotation = {3.145f, -1.738f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 223: 7.40s (raw rot: 1.592, 0.000, -1.738)
    {
        .time = 7.400f,
        .position = {-146.755f, 65.795f, -219.150f},
        .rotation = {3.145f, -1.738f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 224: 7.43s (raw rot: 1.592, 0.000, -1.738)
    {
        .time = 7.433f,
        .position = {-146.755f, 65.795f, -219.150f},
        .rotation = {3.145f, -1.738f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 225: 7.47s (raw rot: 1.592, 0.000, -1.738)
    {
        .time = 7.467f,
        .position = {-146.755f, 65.795f, -219.150f},
        .rotation = {3.145f, -1.738f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 226: 7.50s (raw rot: 1.592, 0.000, -1.738)
    {
        .time = 7.500f,
        .position = {-146.755f, 65.795f, -219.150f},
        .rotation = {3.145f, -1.738f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 227: 7.53s (raw rot: 1.592, 0.000, -1.738)
    {
        .time = 7.533f,
        .position = {-146.755f, 65.795f, -219.150f},
        .rotation = {3.145f, -1.738f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 228: 7.57s (raw rot: 1.592, 0.000, -1.738)
    {
        .time = 7.567f,
        .position = {-146.755f, 65.795f, -219.150f},
        .rotation = {3.145f, -1.738f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 229: 7.60s (raw rot: 1.592, 0.000, -1.738)
    {
        .time = 7.600f,
        .position = {-146.755f, 65.795f, -219.150f},
        .rotation = {3.145f, -1.738f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 230: 7.63s (raw rot: 1.592, 0.000, -1.738)
    {
        .time = 7.633f,
        .position = {-146.654f, 65.845f, -219.131f},
        .rotation = {3.145f, -1.738f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 231: 7.67s (raw rot: 1.592, 0.000, -1.739)
    {
        .time = 7.667f,
        .position = {-146.355f, 65.993f, -219.075f},
        .rotation = {3.145f, -1.739f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 232: 7.70s (raw rot: 1.591, 0.000, -1.739)
    {
        .time = 7.700f,
        .position = {-145.863f, 66.236f, -218.982f},
        .rotation = {3.145f, -1.739f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 233: 7.73s (raw rot: 1.591, 0.000, -1.740)
    {
        .time = 7.733f,
        .position = {-145.184f, 66.571f, -218.855f},
        .rotation = {3.144f, -1.740f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 234: 7.77s (raw rot: 1.590, 0.000, -1.741)
    {
        .time = 7.767f,
        .position = {-144.325f, 66.996f, -218.693f},
        .rotation = {3.144f, -1.741f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 235: 7.80s (raw rot: 1.590, 0.000, -1.742)
    {
        .time = 7.800f,
        .position = {-143.289f, 67.508f, -218.498f},
        .rotation = {3.143f, -1.742f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 236: 7.83s (raw rot: 1.589, 0.000, -1.743)
    {
        .time = 7.833f,
        .position = {-142.085f, 68.104f, -218.272f},
        .rotation = {3.142f, -1.743f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 237: 7.87s (raw rot: 1.588, 0.000, -1.745)
    {
        .time = 7.867f,
        .position = {-140.716f, 68.780f, -218.015f},
        .rotation = {3.141f, -1.745f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 238: 7.90s (raw rot: 1.587, 0.000, -1.746)
    {
        .time = 7.900f,
        .position = {-139.190f, 69.535f, -217.728f},
        .rotation = {3.140f, -1.746f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 239: 7.93s (raw rot: 1.586, 0.000, -1.748)
    {
        .time = 7.933f,
        .position = {-137.511f, 70.365f, -217.412f},
        .rotation = {3.139f, -1.748f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 240: 7.97s (raw rot: 1.585, 0.000, -1.750)
    {
        .time = 7.967f,
        .position = {-135.685f, 71.267f, -217.069f},
        .rotation = {3.138f, -1.750f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 241: 8.00s (raw rot: 1.584, 0.000, -1.752)
    {
        .time = 8.000f,
        .position = {-133.719f, 72.239f, -216.699f},
        .rotation = {3.137f, -1.752f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 242: 8.03s (raw rot: 1.583, 0.000, -1.755)
    {
        .time = 8.033f,
        .position = {-131.618f, 73.278f, -216.304f},
        .rotation = {3.136f, -1.755f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 243: 8.07s (raw rot: 1.581, 0.000, -1.757)
    {
        .time = 8.067f,
        .position = {-129.387f, 74.381f, -215.885f},
        .rotation = {3.134f, -1.757f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 244: 8.10s (raw rot: 1.580, 0.000, -1.760)
    {
        .time = 8.100f,
        .position = {-127.033f, 75.544f, -215.442f},
        .rotation = {3.133f, -1.760f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 245: 8.13s (raw rot: 1.578, 0.000, -1.763)
    {
        .time = 8.133f,
        .position = {-124.561f, 76.767f, -214.977f},
        .rotation = {3.132f, -1.763f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 246: 8.17s (raw rot: 1.577, 0.000, -1.765)
    {
        .time = 8.167f,
        .position = {-121.976f, 78.044f, -214.491f},
        .rotation = {3.130f, -1.765f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 247: 8.20s (raw rot: 1.575, 0.000, -1.768)
    {
        .time = 8.200f,
        .position = {-119.286f, 79.374f, -213.986f},
        .rotation = {3.128f, -1.768f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 248: 8.23s (raw rot: 1.573, 0.000, -1.771)
    {
        .time = 8.233f,
        .position = {-116.495f, 80.754f, -213.461f},
        .rotation = {3.127f, -1.771f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 249: 8.27s (raw rot: 1.572, 0.000, -1.775)
    {
        .time = 8.267f,
        .position = {-113.609f, 82.181f, -212.918f},
        .rotation = {3.125f, -1.775f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 250: 8.30s (raw rot: 1.570, 0.000, -1.778)
    {
        .time = 8.300f,
        .position = {-110.634f, 83.651f, -212.359f},
        .rotation = {3.123f, -1.778f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 251: 8.33s (raw rot: 1.568, 0.000, -1.781)
    {
        .time = 8.333f,
        .position = {-107.575f, 85.163f, -211.784f},
        .rotation = {3.121f, -1.781f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 252: 8.37s (raw rot: 1.566, 0.000, -1.785)
    {
        .time = 8.367f,
        .position = {-104.439f, 86.713f, -211.194f},
        .rotation = {3.119f, -1.785f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 253: 8.40s (raw rot: 1.564, 0.000, -1.788)
    {
        .time = 8.400f,
        .position = {-101.231f, 88.299f, -210.591f},
        .rotation = {3.117f, -1.788f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 254: 8.43s (raw rot: 1.562, 0.000, -1.792)
    {
        .time = 8.433f,
        .position = {-97.957f, 89.918f, -209.976f},
        .rotation = {3.115f, -1.792f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 255: 8.47s (raw rot: 1.560, 0.000, -1.795)
    {
        .time = 8.467f,
        .position = {-94.623f, 91.566f, -209.349f},
        .rotation = {3.113f, -1.795f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 256: 8.50s (raw rot: 1.558, 0.000, -1.799)
    {
        .time = 8.500f,
        .position = {-91.234f, 93.241f, -208.712f},
        .rotation = {3.111f, -1.799f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 257: 8.53s (raw rot: 1.556, 0.000, -1.803)
    {
        .time = 8.533f,
        .position = {-87.796f, 94.941f, -208.065f},
        .rotation = {3.109f, -1.803f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 258: 8.57s (raw rot: 1.554, 0.000, -1.807)
    {
        .time = 8.567f,
        .position = {-84.315f, 96.662f, -207.411f},
        .rotation = {3.107f, -1.807f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 259: 8.60s (raw rot: 1.551, 0.000, -1.811)
    {
        .time = 8.600f,
        .position = {-80.797f, 98.401f, -206.749f},
        .rotation = {3.105f, -1.811f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 260: 8.63s (raw rot: 1.549, 0.000, -1.815)
    {
        .time = 8.633f,
        .position = {-77.247f, 100.156f, -206.082f},
        .rotation = {3.103f, -1.815f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 261: 8.67s (raw rot: 1.547, 0.000, -1.819)
    {
        .time = 8.667f,
        .position = {-73.672f, 101.923f, -205.410f},
        .rotation = {3.100f, -1.819f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 262: 8.70s (raw rot: 1.545, 0.000, -1.822)
    {
        .time = 8.700f,
        .position = {-70.076f, 103.701f, -204.734f},
        .rotation = {3.098f, -1.822f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 263: 8.73s (raw rot: 1.543, 0.000, -1.826)
    {
        .time = 8.733f,
        .position = {-66.466f, 105.486f, -204.055f},
        .rotation = {3.096f, -1.826f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 264: 8.77s (raw rot: 1.541, 0.000, -1.830)
    {
        .time = 8.767f,
        .position = {-62.847f, 107.275f, -203.375f},
        .rotation = {3.094f, -1.830f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 265: 8.80s (raw rot: 1.538, 0.000, -1.834)
    {
        .time = 8.800f,
        .position = {-59.225f, 109.065f, -202.694f},
        .rotation = {3.092f, -1.834f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 266: 8.83s (raw rot: 1.536, 0.000, -1.838)
    {
        .time = 8.833f,
        .position = {-55.606f, 110.854f, -202.013f},
        .rotation = {3.089f, -1.838f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 267: 8.87s (raw rot: 1.534, 0.000, -1.842)
    {
        .time = 8.867f,
        .position = {-51.996f, 112.639f, -201.335f},
        .rotation = {3.087f, -1.842f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 268: 8.90s (raw rot: 1.532, 0.000, -1.846)
    {
        .time = 8.900f,
        .position = {-48.400f, 114.416f, -200.659f},
        .rotation = {3.085f, -1.846f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 269: 8.93s (raw rot: 1.530, 0.000, -1.850)
    {
        .time = 8.933f,
        .position = {-44.824f, 116.184f, -199.986f},
        .rotation = {3.083f, -1.850f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 270: 8.97s (raw rot: 1.527, 0.000, -1.854)
    {
        .time = 8.967f,
        .position = {-41.274f, 117.939f, -199.319f},
        .rotation = {3.081f, -1.854f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 271: 9.00s (raw rot: 1.525, 0.000, -1.858)
    {
        .time = 9.000f,
        .position = {-37.756f, 119.678f, -198.658f},
        .rotation = {3.079f, -1.858f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 272: 9.03s (raw rot: 1.523, 0.000, -1.862)
    {
        .time = 9.033f,
        .position = {-34.275f, 121.399f, -198.003f},
        .rotation = {3.076f, -1.862f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 273: 9.07s (raw rot: 1.521, 0.000, -1.866)
    {
        .time = 9.067f,
        .position = {-30.837f, 123.098f, -197.357f},
        .rotation = {3.074f, -1.866f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 274: 9.10s (raw rot: 1.519, 0.000, -1.869)
    {
        .time = 9.100f,
        .position = {-27.449f, 124.773f, -196.720f},
        .rotation = {3.072f, -1.869f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 275: 9.13s (raw rot: 1.517, 0.000, -1.873)
    {
        .time = 9.133f,
        .position = {-24.114f, 126.422f, -196.093f},
        .rotation = {3.070f, -1.873f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 276: 9.17s (raw rot: 1.515, 0.000, -1.877)
    {
        .time = 9.167f,
        .position = {-20.840f, 128.040f, -195.477f},
        .rotation = {3.068f, -1.877f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 277: 9.20s (raw rot: 1.513, 0.000, -1.880)
    {
        .time = 9.200f,
        .position = {-17.632f, 129.626f, -194.874f},
        .rotation = {3.066f, -1.880f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 278: 9.23s (raw rot: 1.511, 0.000, -1.884)
    {
        .time = 9.233f,
        .position = {-14.496f, 131.176f, -194.285f},
        .rotation = {3.064f, -1.884f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 279: 9.27s (raw rot: 1.509, 0.000, -1.887)
    {
        .time = 9.267f,
        .position = {-11.438f, 132.688f, -193.710f},
        .rotation = {3.062f, -1.887f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 280: 9.30s (raw rot: 1.507, 0.000, -1.890)
    {
        .time = 9.300f,
        .position = {-8.463f, 134.159f, -193.150f},
        .rotation = {3.061f, -1.890f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 281: 9.33s (raw rot: 1.506, 0.000, -1.893)
    {
        .time = 9.333f,
        .position = {-5.577f, 135.586f, -192.608f},
        .rotation = {3.059f, -1.893f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 282: 9.37s (raw rot: 1.504, 0.000, -1.896)
    {
        .time = 9.367f,
        .position = {-2.786f, 136.965f, -192.083f},
        .rotation = {3.057f, -1.896f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 283: 9.40s (raw rot: 1.502, 0.000, -1.899)
    {
        .time = 9.400f,
        .position = {-0.095f, 138.295f, -191.577f},
        .rotation = {3.056f, -1.899f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 284: 9.43s (raw rot: 1.501, 0.000, -1.902)
    {
        .time = 9.433f,
        .position = {2.489f, 139.573f, -191.091f},
        .rotation = {3.054f, -1.902f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 285: 9.47s (raw rot: 1.499, 0.000, -1.905)
    {
        .time = 9.467f,
        .position = {4.961f, 140.795f, -190.626f},
        .rotation = {3.052f, -1.905f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 286: 9.50s (raw rot: 1.498, 0.000, -1.908)
    {
        .time = 9.500f,
        .position = {7.315f, 141.959f, -190.184f},
        .rotation = {3.051f, -1.908f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 287: 9.53s (raw rot: 1.496, 0.000, -1.910)
    {
        .time = 9.533f,
        .position = {9.546f, 143.062f, -189.764f},
        .rotation = {3.050f, -1.910f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 288: 9.57s (raw rot: 1.495, 0.000, -1.912)
    {
        .time = 9.567f,
        .position = {11.647f, 144.100f, -189.369f},
        .rotation = {3.048f, -1.912f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 289: 9.60s (raw rot: 1.494, 0.000, -1.914)
    {
        .time = 9.600f,
        .position = {13.614f, 145.072f, -189.000f},
        .rotation = {3.047f, -1.914f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 290: 9.63s (raw rot: 1.493, 0.000, -1.916)
    {
        .time = 9.633f,
        .position = {15.439f, 145.975f, -188.656f},
        .rotation = {3.046f, -1.916f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 291: 9.67s (raw rot: 1.492, 0.000, -1.918)
    {
        .time = 9.667f,
        .position = {17.118f, 146.805f, -188.341f},
        .rotation = {3.045f, -1.918f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 292: 9.70s (raw rot: 1.491, 0.000, -1.920)
    {
        .time = 9.700f,
        .position = {18.645f, 147.559f, -188.054f},
        .rotation = {3.044f, -1.920f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 293: 9.73s (raw rot: 1.490, 0.000, -1.922)
    {
        .time = 9.733f,
        .position = {20.013f, 148.236f, -187.797f},
        .rotation = {3.043f, -1.922f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 294: 9.77s (raw rot: 1.489, 0.000, -1.923)
    {
        .time = 9.767f,
        .position = {21.218f, 148.831f, -187.570f},
        .rotation = {3.043f, -1.923f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 295: 9.80s (raw rot: 1.489, 0.000, -1.924)
    {
        .time = 9.800f,
        .position = {22.253f, 149.343f, -187.375f},
        .rotation = {3.042f, -1.924f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 296: 9.83s (raw rot: 1.488, 0.000, -1.925)
    {
        .time = 9.833f,
        .position = {23.113f, 149.768f, -187.214f},
        .rotation = {3.041f, -1.925f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 297: 9.87s (raw rot: 1.488, 0.000, -1.926)
    {
        .time = 9.867f,
        .position = {23.792f, 150.104f, -187.086f},
        .rotation = {3.041f, -1.926f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 298: 9.90s (raw rot: 1.487, 0.000, -1.926)
    {
        .time = 9.900f,
        .position = {24.283f, 150.347f, -186.994f},
        .rotation = {3.041f, -1.926f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 299: 9.93s (raw rot: 1.487, 0.000, -1.927)
    {
        .time = 9.933f,
        .position = {24.583f, 150.495f, -186.937f},
        .rotation = {3.040f, -1.927f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 300: 9.97s (raw rot: 1.487, 0.000, -1.927)
    {
        .time = 9.967f,
        .position = {24.684f, 150.545f, -186.918f},
        .rotation = {3.040f, -1.927f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 301: 10.00s (raw rot: 1.487, 0.000, -1.927)
    {
        .time = 10.000f,
        .position = {24.684f, 150.545f, -186.918f},
        .rotation = {3.040f, -1.927f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 302: 10.03s (raw rot: 1.487, 0.000, -1.927)
    {
        .time = 10.033f,
        .position = {24.684f, 150.545f, -186.918f},
        .rotation = {3.040f, -1.927f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 303: 10.07s (raw rot: 1.487, 0.000, -1.927)
    {
        .time = 10.067f,
        .position = {24.684f, 150.545f, -186.918f},
        .rotation = {3.040f, -1.927f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 304: 10.10s (raw rot: 1.487, 0.000, -1.927)
    {
        .time = 10.100f,
        .position = {24.684f, 150.545f, -186.918f},
        .rotation = {3.040f, -1.927f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 305: 10.13s (raw rot: 1.487, 0.000, -1.927)
    {
        .time = 10.133f,
        .position = {24.684f, 150.545f, -186.918f},
        .rotation = {3.040f, -1.927f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 306: 10.17s (raw rot: 1.487, 0.000, -1.927)
    {
        .time = 10.167f,
        .position = {24.684f, 150.545f, -186.918f},
        .rotation = {3.040f, -1.927f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 307: 10.20s (raw rot: 1.487, 0.000, -1.927)
    {
        .time = 10.200f,
        .position = {24.684f, 150.545f, -186.918f},
        .rotation = {3.040f, -1.927f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 308: 10.23s (raw rot: 1.487, 0.000, -1.927)
    {
        .time = 10.233f,
        .position = {24.684f, 150.545f, -186.918f},
        .rotation = {3.040f, -1.927f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 309: 10.27s (raw rot: 1.487, 0.000, -1.927)
    {
        .time = 10.267f,
        .position = {24.684f, 150.545f, -186.918f},
        .rotation = {3.040f, -1.927f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 310: 10.30s (raw rot: 1.487, 0.000, -1.927)
    {
        .time = 10.300f,
        .position = {24.684f, 150.545f, -186.918f},
        .rotation = {3.040f, -1.927f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 311: 10.33s (raw rot: 1.487, 0.000, -1.927)
    {
        .time = 10.333f,
        .position = {24.684f, 150.545f, -186.918f},
        .rotation = {3.040f, -1.927f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 312: 10.37s (raw rot: 1.487, 0.000, -1.927)
    {
        .time = 10.367f,
        .position = {24.684f, 150.545f, -186.918f},
        .rotation = {3.040f, -1.927f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 313: 10.40s (raw rot: 1.487, 0.000, -1.927)
    {
        .time = 10.400f,
        .position = {24.684f, 150.545f, -186.918f},
        .rotation = {3.040f, -1.927f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 314: 10.43s (raw rot: 1.487, 0.000, -1.927)
    {
        .time = 10.433f,
        .position = {24.684f, 150.545f, -186.918f},
        .rotation = {3.040f, -1.927f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 315: 10.47s (raw rot: 1.487, 0.000, -1.927)
    {
        .time = 10.467f,
        .position = {24.684f, 150.545f, -186.918f},
        .rotation = {3.040f, -1.927f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 316: 10.50s (raw rot: 1.487, 0.000, -1.927)
    {
        .time = 10.500f,
        .position = {24.684f, 150.545f, -186.918f},
        .rotation = {3.040f, -1.927f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 317: 10.53s (raw rot: 1.487, 0.000, -1.927)
    {
        .time = 10.533f,
        .position = {24.684f, 150.545f, -186.918f},
        .rotation = {3.040f, -1.927f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 318: 10.57s (raw rot: 1.487, 0.000, -1.927)
    {
        .time = 10.567f,
        .position = {24.684f, 150.545f, -186.918f},
        .rotation = {3.040f, -1.927f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 319: 10.60s (raw rot: 1.487, 0.000, -1.927)
    {
        .time = 10.600f,
        .position = {24.684f, 150.545f, -186.918f},
        .rotation = {3.040f, -1.927f, 0.000f},
        .actionId = ECutsceneAction::None,
        .actionData = nullptr,
        .lerp = true
    },
    // Frame 320: 10.63s (raw rot: 1.487, 0.000, -1.927)
    {
        .time = 10.633f,
        .position = {24.684f, 150.545f, -186.918f},
        .rotation = {3.040f, -1.927f, 0.000f},
        .actionId = ECutsceneAction::TransitionToScene,
        .actionData = &sVillageSceneDef,
        .lerp = true
    }
};

static const SCutsceneObjectDef sExampleCutsceneObjects[] = {
    {
        .modelPath = "rom:/village.t3dm",
        .animationName = nullptr,
        .position = {0.0f, 0.0f, 0.0f},
        .rotation = {0.0f, 0.0f, 0.0f},
        .scale = {1.0f, 1.0f, 1.0f}
    },
    {
        .modelPath = "rom:/cutscn-snep.t3dm",
        .animationName = "intro",
        .position = {0.0f, 0.0f, 0.0f},
        .rotation = {0.0f, 0.0f, 0.0f},
        .scale = {1.0f, 1.0f, 1.0f}
    },
};

static const SCutsceneDef sExampleCutsceneDef = {
    .name = "intro cutscene",
    .objects = sExampleCutsceneObjects,
    .objectCount = sizeof(sExampleCutsceneObjects) / sizeof(sExampleCutsceneObjects[0]),
    .cameraFrames = sExampleCutsceneFrames,
    .frameCount = sizeof(sExampleCutsceneFrames) / sizeof(sExampleCutsceneFrames[0]),
    .onInit = exampleCutsceneInit,
    .onEnd = exampleCutsceneEnd
};
