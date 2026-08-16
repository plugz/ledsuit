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
    DigiLed_setAllIllumination(0x01); // 0 - 31    0x00 - 0x1f
    DigiLed_update(0);
}

struct Ball {
    float position = 0.0f;
    float speed = 0.0f;
    float size = 1.0f;

    void tick(float accel, Chrono::Milliseconds tickTime) {
        float tickTimeS = (float)tickTime.count() / 1000.0f;
        speed += accel * 10 * tickTimeS;
        position += speed * tickTimeS;
        if (position > ((LED_FRAME_SIZE * 1) - 1)) {
            position = (LED_FRAME_SIZE * 1) - 1;
            speed = 0;
        }
        if (position < 0) {
            position = 0;
            speed = 0;
        }
    }
};

void ledstrip_tick(Eigen::Vector3f const& accelMg, Eigen::Vector3f const& angularRateMdps) {
    static Chrono::MsTimer timer(Chrono::Milliseconds(2));
    if (!timer.done())
        return;
    auto tickTime = timer.elapsedTime();
    timer.advance();

    RgbColor frame[LED_FRAME_SIZE];

    static Ball balls[3];

    static const RgbColor ballColors[3] = {
        {0x40, 0x00, 0x40},
        {0x00, 0x40, 0x40},
        {0x20, 0x20, 0x20}
    };

    {
        for (size_t i = 0; i < 3; ++i) {
            float accelG = -accelMg[i] / 1000.0f;
            balls[i].tick(accelG, tickTime);
        }

        for (size_t frameIdx = 0; frameIdx < LED_FRAME_SIZE; ++frameIdx) {
            //for (size_t i = 0; i < 3; ++i) {
            size_t i = 0; {
                float distance = std::abs((float)frameIdx - balls[i].position);
                float level = (balls[i].size - distance);
                if (level > 0) {
                    frame[frameIdx] += ballColors[i] * level;
                }
            }
        }
    }

    for (int i = 0; i < LED_FRAME_SIZE; ++i) {
        DigiLed_setColor(i, frame[i].r(), frame[i].g(), frame[i].b());
    }

    DigiLed_update(0);
}

void ledstrip_setblinkin(bool b) {
    blinkin = b;
}
