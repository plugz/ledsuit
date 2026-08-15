#include "LedStrip.hpp"

#include "DigiLed.h"
#include "spi.h"

#include <cmath>

static bool blinkin = false;

void ledstrip_init() {
    DigiLed_init(&hspi2);
    DigiLed_setAllRGB(0x00);
    DigiLed_setAllIllumination(0x1f); // 0 - 31    0x00 - 0x1f
    DigiLed_update(0);
}

void ledstrip_tick(Eigen::Vector3f const& accelMg, Eigen::Vector3f const& angularRateMdps) {
    static uint32_t prevTime = 0;
    uint32_t curTime = HAL_GetTick();
    if (curTime - prevTime <= 2)
        return;
    prevTime = curTime;

    uint8_t rgb[3];
    for (size_t i = 0; i < 3; ++i) {
        float tmp = (50) + (accelMg[i] / 10.0f);
        if (tmp < 0)
            tmp = 0;
        rgb[i] = tmp < 254 ? tmp : 255;
    }

    for (int i = 0; i < LED_FRAME_SIZE; ++i) {
        DigiLed_setColor(i, rgb[0], rgb[1], rgb[2]);
    }
    DigiLed_update(0);
}

void ledstrip_setblinkin(bool b) {
    blinkin = b;
}
