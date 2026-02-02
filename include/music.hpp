#pragma once

#include <string>
#include "xm64.h"

class CMusicMgr
{
public:
	static void init(int firstMixerChannel = 0, const char* extSampleDir = nullptr);
	static void open(const std::string& res);
	static void play();
	static void stop();
	static void close();

	static void setLoop(bool loop);
	static void setVolume(float volume);

	static bool isLoaded();

private:
	static std::string sPath;
	static xm64player_t sPlayer;
	static bool sLoaded;
	static bool sLoop;
	static int sFirstMixerChannel;
};
