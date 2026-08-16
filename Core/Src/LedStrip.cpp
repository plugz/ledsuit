#include "LedStrip.hpp"

#include "Chrono.hpp"
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
    static Chrono::MsTimer timer(Chrono::Milliseconds(2));
    if (!timer.done())
        return;
    timer.advance();

    uint8_t rgb[3];
    for (size_t i = 0; i < 3; ++i) {
        float tmp = (50) + (accelMg[i] / 10.0f);
        if (tmp < 0)
            tmp = 0;
        rgb[i] = tmp < 254 ? tmp : 255;
    }

    // tmp
    {
        static Chrono::MsTimer timer2{};
        int colorIdx = (timer2.elapsedTime().count() / 2000) % 7;
        static constexpr uint8_t colors[7][3] = {
            {0xff, 0x00, 0x00},
            {0x00, 0xff, 0x00},
            {0x00, 0x00, 0xff},
            {0xff, 0xff, 0x00},
            {0x00, 0xff, 0xff},
            {0xff, 0x00, 0xff},
            {0xff, 0xff, 0xff}
        };
        for (size_t i = 0; i < 3; ++i) {
            rgb[i] = colors[colorIdx][i];
        }
    }

    for (int i = 0; i < LED_FRAME_SIZE; ++i) {
        DigiLed_setColor(i, rgb[0], rgb[1], rgb[2]);
    }

    DigiLed_update(0);
}

void ledstrip_setblinkin(bool b) {
    blinkin = b;
}
