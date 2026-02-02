#include <libdragon.h>

#include "util.hpp"

float TUtil::sLastTime;
float TUtil::sCurrTime;
float TUtil::sDeltaTime;

void TUtil::updateTime()
{
    sCurrTime = getTimeSeconds();
    sDeltaTime = sCurrTime - sLastTime;
    sLastTime = sCurrTime;
}