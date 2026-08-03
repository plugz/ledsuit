#include "LedStrip.hpp"

#include "DigiLed.h"
#include "spi.h"

#include <cmath>

static uint8_t myLedz[LED_FRAME_SIZE] = {0,};
static bool blinkin = false;

void ledstrip_init() {
    DigiLed_init(&hspi2);
    DigiLed_setAllRGB(0x00);
    DigiLed_setAllIllumination(0x09); // 0 - 31    0x00 - 0x1f
    DigiLed_update(0);
}

void ledstrip_tick(float yaw) {
    static int constexpr virtualLedCount = LED_FRAME_SIZE + 8;
    int const yawLedIdx = (yaw / (2 * M_PI)) * virtualLedCount;
    //int const invYawLedIdx = (yawLedIdx + (virtualLedCount / 2)) % virtualLedCount;

    for (int i = 0; i < LED_FRAME_SIZE; ++i) {
        if ((std::abs(i - yawLedIdx) < 4) || (std::abs(i + virtualLedCount - yawLedIdx) < 4) || (std::abs(i - virtualLedCount - yawLedIdx) < 4)) {
            myLedz[i] = 0xff;
        }
    }

    static uint32_t prevTime = 0;
    uint32_t curTime = HAL_GetTick();
    if (curTime - prevTime > 2) {
        for (int i = 0; i < LED_FRAME_SIZE; ++i) {
            if (blinkin) {
                if (((curTime / 30) % 4) == (((i + curTime / 120) / 8) % 4))
                    DigiLed_setColor(i, 0xff, 0x15, 0xff);
                else
                    DigiLed_setColor(i, 0x00, 0x00, 0x00);
            } else {
                DigiLed_setColor(i, myLedz[i], 0, myLedz[i]);
            }
            for (unsigned int j = 0; j < 5 && myLedz[i]; ++j) {
                --myLedz[i];
            }
        }
        do { prevTime += 2; } while (prevTime < curTime);
    }
    DigiLed_update(0);
}

void ledstrip_setblinkin(bool b) {
    blinkin = b;
}
