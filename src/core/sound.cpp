#include "sound.hpp"
#include "wav64.h"

std::unordered_map<std::string, TSoundRes> CSoundMgr::sSoundResStrList{};
std::unordered_map<int32_t, TSoundRes> CSoundMgr::sSoundResKeyList{};

int32_t CSoundMgr::sMixerChannelCount = 0;
int32_t CSoundMgr::sSfxChannelBase = 0;
int32_t CSoundMgr::sSfxChannelCount = 0;
int32_t CSoundMgr::sSfxChannelNext = 0;

int32_t CSoundMgr::allocSfxChannel(bool stereo)
{
    if (sSfxChannelCount <= 0)
        return 0;

    if (stereo)
    {
        int32_t stereoCount = sSfxChannelCount & ~1; 
        if (stereoCount < 2)
        {
            stereo = false;
        }
        else
        {
            if (sSfxChannelNext >= stereoCount)
                sSfxChannelNext = 0;
            sSfxChannelNext &= ~1; // align to even start index
            const int32_t idx = sSfxChannelNext;
            sSfxChannelNext = (sSfxChannelNext + 2) % stereoCount;
            return sSfxChannelBase + idx;
        }
    }

    const int32_t idx = sSfxChannelNext % sSfxChannelCount;
    sSfxChannelNext = (sSfxChannelNext + 1) % sSfxChannelCount;
    return sSfxChannelBase + idx;
}

void CSoundMgr::init(int32_t freq, int32_t numOfBuffers, int32_t numOfChannels)
{
    audio_init(freq, numOfBuffers);
	mixer_init(numOfChannels);

    // 6..15 channels
    sMixerChannelCount = numOfChannels;
    sSfxChannelCount = (numOfChannels < 10) ? numOfChannels : 10;
    sSfxChannelBase = (numOfChannels > 10) ? (numOfChannels - 10) : 0;
    sSfxChannelNext = 0;

	//mixer_ch_set_limits(2, 0, 128000, 0);
}

void CSoundMgr::open(std::string const & res)
{
    TSoundRes soundRes{};
    std::string resFullPath = "rom:/" + res + ".wav64";

    wav64_open(&soundRes.waveRes, resFullPath.c_str());
	wav64_set_loop(&soundRes.waveRes, soundRes.isLoop);

    sSoundResStrList[res] = soundRes;
}

void CSoundMgr::play(const std::string & res)
{
    play(res, false);
}

void CSoundMgr::play(const std::string & res, bool loop)
{
    play(res, loop, -1);
}

void CSoundMgr::play(const std::string & res, bool loop, int32_t channel)
{
    auto it = sSoundResStrList.find(res);
    if (it == sSoundResStrList.end())
    {
        open(res);
        it = sSoundResStrList.find(res);
        if (it == sSoundResStrList.end())
            return;
    }

    it->second.isLoop = loop;
    wav64_set_loop(&it->second.waveRes, loop);

	const bool stereo = it->second.waveRes.wave.channels == 2;

    int32_t mixerChannel = channel;
    const bool channelValid = (mixerChannel >= 0) && (mixerChannel < sMixerChannelCount);
    if (!channelValid)
    {
        mixerChannel = allocSfxChannel(stereo);
    }
    else if (stereo)
    {
        const bool stereoValid = ((mixerChannel & 1) == 0) && ((mixerChannel + 1) < sMixerChannelCount);
        if (!stereoValid)
            mixerChannel = allocSfxChannel(true);
    }

    wav64_play(&it->second.waveRes, mixerChannel);
}

void CSoundMgr::stop(int32_t channel)
{
    if (channel >= 0 && channel < sMixerChannelCount) {
        mixer_ch_stop(channel);
    }
}

void CSoundMgr::update()
{
    mixer_try_play();
}