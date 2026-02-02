#include "music.hpp"

#include <libdragon.h>

std::string CMusicMgr::sPath{};
xm64player_t CMusicMgr::sPlayer{};
bool CMusicMgr::sLoaded = false;
bool CMusicMgr::sLoop = true;
int CMusicMgr::sFirstMixerChannel = 0;

static std::string normalizeXm64Path(const std::string& res) {
	if (res.rfind("rom:/", 0) == 0) return res;
	if (res.size() >= 5 && (res.substr(res.size() - 5) == ".xm64" || res.substr(res.size() - 5) == ".XM64")) {
		return std::string("rom:/") + res;
	}
	return std::string("rom:/") + res + ".xm64";
}

void CMusicMgr::init(int firstMixerChannel, const char* extSampleDir)
{
	sFirstMixerChannel = firstMixerChannel;
	if (extSampleDir && extSampleDir[0] != '\0') {
		xm64_set_extsampledir(extSampleDir);
	}
}

void CMusicMgr::open(const std::string& res)
{
	close();
	sPath = normalizeXm64Path(res);
	xm64player_open(&sPlayer, sPath.c_str());
	xm64player_set_loop(&sPlayer, sLoop);
	sLoaded = true;
}

void CMusicMgr::play()
{
	if (!sLoaded) return;
	xm64player_play(&sPlayer, sFirstMixerChannel);
}

void CMusicMgr::stop()
{
	if (!sLoaded) return;
	xm64player_stop(&sPlayer);
}

void CMusicMgr::close()
{
	if (!sLoaded) return;
	
	xm64player_stop(&sPlayer);
	xm64player_close(&sPlayer);
	sLoaded = false;
	sPath.clear();
}

void CMusicMgr::setLoop(bool loop)
{
	sLoop = loop;
	if (sLoaded) xm64player_set_loop(&sPlayer, sLoop);
}

void CMusicMgr::setVolume(float volume)
{
	if (!sLoaded) return;
	xm64player_set_vol(&sPlayer, volume);
}

bool CMusicMgr::isLoaded()
{
	return sLoaded;
}
