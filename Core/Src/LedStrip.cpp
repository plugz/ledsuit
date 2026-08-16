#include "LedStrip.hpp"

#include "Chrono.hpp"
#include "DigiLed.h"
#include "RgbColor.hpp"

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

    RgbColor frame[LED_FRAME_SIZE];

    {
        size_t ballPositions[3];
        size_t ballSizes[3];
        RgbColor ballColors[3] = {
            {0x20, 0x00, 0x20},
            {0x00, 0x20, 0x20},
            {0x10, 0x10, 0x10}
        };

        for (size_t i = 0; i < 3; ++i) {
            int accel2dG = accelMg[i] / 50.0f;
            if (accel2dG >= 0) {
                ballSizes[i] = std::max(1, accel2dG / LED_FRAME_SIZE);
                ballPositions[i] = ((LED_FRAME_SIZE / 2) + accel2dG) % LED_FRAME_SIZE;
            }
            else {
                ballSizes[i] = std::max(1, -accel2dG / LED_FRAME_SIZE);
                ballPositions[i] = (LED_FRAME_SIZE - 1) - (((LED_FRAME_SIZE / 2) - accel2dG) % LED_FRAME_SIZE);
            }
        }

        for (size_t frameIdx = 0; frameIdx < LED_FRAME_SIZE; ++frameIdx) {
            for (size_t i = 0; i < 3; ++i) {
            //size_t i = 0; {
                if (((LED_FRAME_SIZE + (int)frameIdx - (int)ballPositions[i]) % LED_FRAME_SIZE) <= (int)ballSizes[i]) {
                    frame[frameIdx] += ballColors[i];
                }
            }
        }
    }

    for (int i = 0; i < LED_FRAME_SIZE; ++i) {
        DigiLed_setColor(i, frame[i].r(), frame[i].g(), frame[i].b());
    }

    DigiLed_setAllIllumination(0x01);
    DigiLed_update(0);
}

void ledstrip_setblinkin(bool b) {
    blinkin = b;
}
