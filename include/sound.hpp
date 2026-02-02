#pragma once

#include <libdragon.h>
#include "wav64.h"
#include <string>
#include <unordered_map>

struct TSoundRes
{
    wav64_t waveRes;
    int32_t id;
    int32_t loopStart;
    int32_t loopEnd;
    bool isLoop;
};

class CSoundMgr
{
    public:
    
    static void init(int32_t freq, int32_t numOfBuffers, int32_t numOfChannels);
    static void open(std::string const & res);
    static void play(std::string const & res);
    static void play(std::string const & res, bool loop);
    static void play(std::string const & res, bool loop, int32_t channel);
    static void stop(int32_t channel);
    static void update();
    
    private:
    static int32_t allocSfxChannel(bool stereo);

    static std::unordered_map<std::string, TSoundRes> sSoundResStrList;
    static std::unordered_map<int32_t, TSoundRes> sSoundResKeyList;

    static int32_t sMixerChannelCount;
    static int32_t sSfxChannelBase;
    static int32_t sSfxChannelCount;
    static int32_t sSfxChannelNext;
};