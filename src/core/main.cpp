#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>

#include "display.h"
#include "math.hpp"
#include "rdpq_text.h"
#include "scene.hpp"
#include "sound.hpp"
#include "music.hpp"
#include "viewport.hpp"
#include "model.hpp"
#include "skinned_model.hpp"
#include "light.hpp"
#include "player.hpp"
#include "particle.hpp"
#include "wipe.hpp"
#include "collision.hpp"
#include "textbox.hpp"
#include "camera.hpp"
#include "scene_defs.hpp"
#include "menu.hpp"
#include "save_manager.hpp"

float get_time_s() {
  return (float)((double)get_ticks_us() / 1000000.0);
}

void modelTestInit()
{
	CViewport viewport{};
	viewport.init();
	viewport.setProjection(55.0f, 10.0f, 1000.0f);

	CLight light{};
	light.setAmbient(0xAA, 0xAA, 0xAA);
	light.setDirectional(0, 0xFF, 0xAA, 0xAA, {1.0f, 1.0f, 1.0f});
	light.setCount(1);

	
	CPlayer player{};
	player.init({0.0f, 0.15f, 0.0f});
	player.setBounds(140.0f);
	
	player.getStateMachine().getReelState().setFishPool(sFishPool, sFishPoolCount);

	CCamera camera{};
	camera.init();
	camera.setDistance(65.0f);
	camera.setHeight(45.0f);
	camera.setTargetOffset(10.0f);
	camera.setDistanceLimits(30.0f, 120.0f);
	
	player.setCamera(&camera);

	CSceneManager::instance().init(player, viewport, light, camera);
	CSceneManager::instance().startLogoScene(&sExampleCutsceneDef);
	//CSceneManager::instance().loadScene(sVillageSceneDef);

	CContinuousEmitter snowEmitter{};
	snowEmitter.init(600);
	snowEmitter.setGravity({0.0f, -15.0f, 0.0f});
	snowEmitter.setFadeOverLife(false);
	snowEmitter.setEmissionRate(600.0f);
	snowEmitter.setSpawnVelocity({-5.0f, -10.0f, -5.0f}, {5.0f, -3.0f, 5.0f});
	snowEmitter.setSpawnSize(1.0f, 3.0f);
	snowEmitter.setSpawnLife(3.0f, 5.0f);
	snowEmitter.setSpawnColor(255, 255, 255, 220);
	snowEmitter.setSpawnSpread(175.0f);
	snowEmitter.setWorldScale(1.0f);
	snowEmitter.setPosition({0.0f, 80.0f, 0.0f});
	snowEmitter.start();

	CParticleEmitter burstEmitter{};
	burstEmitter.init(50);
	burstEmitter.setGravity({0.0f, -15.0f, 0.0f});
	burstEmitter.setFadeOverLife(true);

	CMenu pauseMenu{};
	pauseMenu.init(FONT_BUILTIN_DEBUG_MONO);
	
	player.setMenu(&pauseMenu);
	player.setMenu(&pauseMenu);
	pauseMenu.setPlayer(&player);
	pauseMenu.setTotalFishSpecies(sFishPoolCount);
	
	CShop* shop = CSceneManager::instance().getShop();
	if (shop) {
		shop->setMenu(&pauseMenu);
		shop->setPlayer(&player);
		shop->setFishPool(sFishPool, sFishPoolCount);
	}
	
	pauseMenu.setCurrency(100);
	
	if (gSaveManager.init()) {
		if (gSaveManager.hasSaveData()) {
			gSaveManager.load(pauseMenu);
		}
	}
	
	//pauseMenu.addItem(EMenuTab::FishingRods, "Basic Rod", 1, 0);
	//pauseMenu.addItem(EMenuTab::FishingRods, "Oak Rod", 1, 1);
	//pauseMenu.addItem(EMenuTab::FishingRods, "Steel Rod", 1, 2);
	//pauseMenu.equipFishingRod(0);
	
	//pauseMenu.addItem(EMenuTab::Bait, "Worm", 10, 3);
	//pauseMenu.addItem(EMenuTab::Bait, "Minnow", 5, 4);
	//pauseMenu.addItem(EMenuTab::Bait, "Bread Crumbs", 20, 5);
	//pauseMenu.equipBait(0);
	
	//pauseMenu.addItem(EMenuTab::MiscItems, "Map", 1, 7);
	//pauseMenu.addItem(EMenuTab::MiscItems, "Protein Bar", 3, 6);
	
	float totalPlayTime = 0.0f;
	float dayNightCycleDuration = 120.0f;
	float dayNightTime = 0.0f;

	float lastTime = get_time_s() - (1.0f / 60.0f);
	rspq_syncpoint_t syncPoint = 0;
	
	uint32_t frameIndex = 0;

	float saveTimer = 0.0f;
	constexpr float SAVE_INTERVAL = 60.0f;

	float villageMusicTimer = 0.0f;
	float villageMusicDelay = 60.0f;
	bool hasPlayedVillageMusic = false;

	dayNightTime = 40.0f;
	for(;;)
	{
		joypad_poll();

		float newTime = get_time_s();
		float deltaTime = display_get_delta_time();
		lastTime = newTime;
		
		dayNightTime += deltaTime;
		if (dayNightTime > dayNightCycleDuration) {
			dayNightTime -= dayNightCycleDuration;
		}
		
		float cycleFactor = dayNightTime / dayNightCycleDuration;
		
		float dayNightBlend = (1.0f - cosf(cycleFactor * 2.0f * 3.14159f)) * 0.5f;
		
		uint8_t dayAmbR = 170, dayAmbG = 170, dayAmbB = 170;
		uint8_t dayDirR = 255, dayDirG = 200, dayDirB = 180;
		uint8_t daySkyR = 136, daySkyG = 161, daySkyB = 252;
		uint8_t dayFogR = 255, dayFogG = 255, dayFogB = 255;
		
		uint8_t nightAmbR = 40, nightAmbG = 50, nightAmbB = 80;
		uint8_t nightDirR = 80, nightDirG = 90, nightDirB = 130;
		uint8_t nightSkyR = 20, nightSkyG = 25, nightSkyB = 50;
		uint8_t nightFogR = 60, nightFogG = 70, nightFogB = 100;
		
		light.lerpColors(dayAmbR, dayAmbG, dayAmbB,
		                 nightAmbR, nightAmbG, nightAmbB,
		                 dayDirR, dayDirG, dayDirB,
		                 nightDirR, nightDirG, nightDirB,
		                 dayNightBlend);
		
		uint8_t skyR = (uint8_t)(daySkyR + (nightSkyR - daySkyR) * dayNightBlend);
		uint8_t skyG = (uint8_t)(daySkyG + (nightSkyG - daySkyG) * dayNightBlend);
		uint8_t skyB = (uint8_t)(daySkyB + (nightSkyB - daySkyB) * dayNightBlend);
		
		uint8_t fogR = (uint8_t)(dayFogR + (nightFogR - dayFogR) * dayNightBlend);
		uint8_t fogG = (uint8_t)(dayFogG + (nightFogG - dayFogG) * dayNightBlend);
		uint8_t fogB = (uint8_t)(dayFogB + (nightFogB - dayFogB) * dayNightBlend);

		CSceneManager::instance().setFrameIndex(frameIndex);
		
		if (!CSceneManager::instance().isInCutscene() && !CSceneManager::instance().isInLogoScene()) {
			player.setFrameIndex(frameIndex);
			player.update(deltaTime);
			player.updateBufferedMatrix(frameIndex);

		CScene* currentScene = CSceneManager::instance().getCurrentScene();
		if (currentScene != nullptr) {
			CCollisionMesh* collision = currentScene->getCollision();
			if (collision->isLoaded()) {
				player.applyCollision(*collision);
			}
		}
	}
	
	CSceneManager::instance().updateBufferedMatrix(frameIndex);

		
		TVec3F focusPos = CSceneManager::instance().getFocusPosition();
		snowEmitter.setPosition({focusPos.x(), focusPos.y() + 40.0f, focusPos.z()});
		snowEmitter.update(deltaTime);

		joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);
		joypad_buttons_t held = joypad_get_buttons_held(JOYPAD_PORT_1);
		joypad_inputs_t joypad = joypad_get_inputs(JOYPAD_PORT_1);
		
		CShop* shop = CSceneManager::instance().getShop();
		bool shopActive = shop && shop->isOpen();
		
		bool inItemGet = player.getStateMachine().isInState("item_get");
		bool inCutsceneOrLogo = CSceneManager::instance().isInCutscene() || CSceneManager::instance().isInLogoScene();
		
		if (btn.start && !inItemGet && !shopActive && !inCutsceneOrLogo) {
			pauseMenu.toggle();
		}
		
		totalPlayTime += deltaTime;
		pauseMenu.addPlayTime(deltaTime);
		
		saveTimer += deltaTime;
		if (saveTimer >= SAVE_INTERVAL && gSaveManager.isAvailable()) {
			gSaveManager.save(pauseMenu);
			saveTimer = 0.0f;
		}
		
		bool shopConsumedInput = shopActive && shop->update(deltaTime, btn, held);
		bool menuActive = !inItemGet && !shopConsumedInput && pauseMenu.update(deltaTime, btn, held);
		
		if (!menuActive && !inItemGet && !shopConsumedInput) {
			
		}
		
		burstEmitter.update(deltaTime);

		CSceneManager::instance().update(deltaTime);
		std::string currentSceneName{};

		if (CSceneManager::instance().getCurrentScene() != nullptr) {
			currentSceneName = CSceneManager::instance().getCurrentScene()->getName();
		}
		
		TVec3F playerPos = player.getPosition();
		if (!CSceneManager::instance().isInCutscene() && !CSceneManager::instance().isInLogoScene()) {
			camera.update(deltaTime, playerPos, joypad);
			
			CScene* currentScene = CSceneManager::instance().getCurrentScene();
			if (currentScene != nullptr) {
				CCollisionMesh* collision = currentScene->getCollision();
				if (collision != nullptr && collision->isLoaded()) {
					camera.applyCollision(*collision);
				}
			}

			player.setCameraAngle(camera.getOrbitAngle());

			viewport.setProjection(85.0f, 10.0f, 1000.0f);
			camera.apply(viewport);
		}

		if (currentSceneName == "village") {
			mixer_ch_set_vol(0, 0.3f, 0.3f);
			mixer_ch_set_vol(1, 0.3f, 0.3f);
			
			villageMusicTimer += deltaTime;
			if (villageMusicTimer >= villageMusicDelay) {
				if (!hasPlayedVillageMusic) {
					CSoundMgr::play("p2mp", false, 2);
					hasPlayedVillageMusic = true;
				}
				if (!mixer_ch_playing(2)) {
					villageMusicTimer = 0.0f;
					hasPlayedVillageMusic = false;
				}
			}
		} else {
			if (CSceneManager::instance().isInCutscene() || CSceneManager::instance().isInLogoScene()) {
				mixer_ch_set_vol(0, 0.3f, 0.3f);
				mixer_ch_set_vol(1, 0.3f, 0.3f);
			} else {
				mixer_ch_set_vol(0, 0.1f, 0.1f);
				mixer_ch_set_vol(1, 0.1f, 0.1f);
			}
			
			villageMusicTimer = 0.0f;
			hasPlayedVillageMusic = false;
		}

		CSoundMgr::update();

		if(syncPoint) rspq_syncpoint_wait(syncPoint);

		surface_t* disp = display_get();
		rdpq_attach(disp, display_get_zbuf());
		
		if (shop && shop->shouldDraw()) {
			rdpq_set_mode_fill(RGBA32(0, 0, 0, 0xFF));
			rdpq_fill_rectangle(0, 0, 640, 480);
			
			shop->draw();
			shop->drawFade();
			
			rdpq_detach_show();
		} else {
			
			
			rdpq_set_mode_fill(RGBA32(skyR, skyG, skyB, 0xFF));
			rdpq_fill_rectangle(0, 0, 256, 240);
		
		//skybox.setColorTint(skyR, skyG, skyB, 255);
		//skybox.draw();
		
		t3d_frame_start();
		t3d_screen_clear_depth();
		rdpq_mode_antialias(AA_NONE);
		rdpq_mode_filter(FILTER_POINT);
		viewport.attach();

		light.apply();
		
		if (CSceneManager::instance().isInCutscene()) {
			t3d_fog_set_range(0.4f, 550.0f);
		} else {
			t3d_fog_set_range(0.4f, 150.0f);
		}
		

		if (currentSceneName == "village") {
			rdpq_mode_fog(RDPQ_FOG_STANDARD);
			rdpq_set_fog_color((color_t){fogR, fogG, fogB, 255});
			t3d_fog_set_enabled(true);
		} else {
			t3d_fog_set_enabled(false);
		}

		CSceneManager::instance().draw();

		if (!CSceneManager::instance().isInCutscene() && !CSceneManager::instance().isInLogoScene()) {
			player.draw();

			if (player.getStateMachine().isInState("throw") || player.getStateMachine().isInState("hold") || player.getStateMachine().isInState("reel")) {
				player.drawFishingLine(viewport, camera.getPosition());
			}
		}

		if (currentSceneName == "village" || CSceneManager::instance().isInCutscene()) {
			snowEmitter.draw();
		}
		
		CSceneManager::instance().drawUI(); 

		//wipe.draw();
		//shatter.draw();
		//drownWipe.draw();

		
		syncPoint = rspq_syncpoint_new();

		CSceneManager::instance().drawTransitionOverlays();

		float posX = 16;
		float posY = 24;

		rdpq_sync_pipe();
		
		//rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY, "FPS: %.1f", display_get_fps());
		//rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY + 8, "%.1f, %.1f, %.1f", 
		//                 playerPos.x(), playerPos.y(), playerPos.z());
		//rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, posX, posY + 16, "%.1f", player.getSpeed());
		
		player.drawItemGetOverlay(FONT_BUILTIN_DEBUG_MONO);
		player.drawExpGainAnimation(FONT_BUILTIN_DEBUG_MONO);
		player.drawLevelUpAnimation(FONT_BUILTIN_DEBUG_MONO);
		
		player.drawReelMeter(FONT_BUILTIN_DEBUG_MONO);
		
		pauseMenu.draw();
		
		if (shop && shop->isFading()) {
			shop->drawFade();
		}
		
		rdpq_detach_show();
		}
		
		frameIndex++;

	}
}

int main()
{
	debug_init_isviewer();
	debug_init_usblog();
	asset_init_compression(2);
	wav64_init_compression(1);
	
	display_init(RESOLUTION_256x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
	rdpq_init();
	//rdpq_debug_start();
	joypad_init();

	t3d_init((T3DInitParams){});
	rdpq_text_register_font(FONT_BUILTIN_DEBUG_MONO, rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO));
	

	int ret = dfs_init(DFS_DEFAULT_LOCATION);
	assert(ret == DFS_ESUCCESS);

	rdpq_font_t *fnt1 = rdpq_font_load("rom:/divinity.font64");
	rdpq_text_register_font(2, fnt1);

	CSoundMgr::init(48000, 4,  16);
	//CSoundMgr::open("menu_open");
	CSoundMgr::play("wind", true, 0);
	mixer_ch_set_vol(0, 0.3f, 0.3f);
	mixer_ch_set_vol(1, 0.3f, 0.3f);

	//CMusicMgr::init(0, "nullptr");
	//CMusicMgr::open("rom:/wind1.xm64");
	//CMusicMgr::setVolume(1.0f);
	//CMusicMgr::play();

	modelTestInit();
	
	return 0;
}