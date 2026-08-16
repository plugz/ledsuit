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
        float ballPositions[3];
        float ballSizes[3];
        RgbColor ballColors[3] = {
            {0x40, 0x00, 0x40},
            {0x00, 0x40, 0x40},
            {0x20, 0x20, 0x20}
        };

        for (size_t i = 0; i < 3; ++i) {
            float accel2dG = accelMg[i] / 100.0f;
            if (accel2dG >= 0.0f) {
                ballSizes[i] = std::max(0.5f, accel2dG / LED_FRAME_SIZE);
                ballPositions[i] = ((LED_FRAME_SIZE / 2) + accel2dG);
                ballPositions[i] -= LED_FRAME_SIZE * ((int)ballPositions[i] / LED_FRAME_SIZE); // ballPositions[i] %= LED_FRAME_SIZE
            }
            else {
                ballSizes[i] = std::max(0.5f, -accel2dG / LED_FRAME_SIZE);
                ballPositions[i] = (LED_FRAME_SIZE / 2) - accel2dG;
                ballPositions[i] -= LED_FRAME_SIZE * ((int)ballPositions[i] / LED_FRAME_SIZE); // ballPositions[i] %= LED_FRAME_SIZE
                ballPositions[i] = (LED_FRAME_SIZE - 1) - ballPositions[i];
            }
        }

        for (size_t frameIdx = 0; frameIdx < LED_FRAME_SIZE; ++frameIdx) {
            for (size_t i = 0; i < 3; ++i) {
            //size_t i = 0; {
                float distance = (float)LED_FRAME_SIZE + (float)frameIdx - ballPositions[i];
                distance -= LED_FRAME_SIZE * ((int)distance / LED_FRAME_SIZE); // distance %= LED_FRAME_SIZE
                float level = (ballSizes[i] - distance) / 3.0f;
                if (level > 0) {
                    frame[frameIdx] += ballColors[i] * level;
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
