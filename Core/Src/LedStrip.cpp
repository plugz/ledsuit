#include "LedStrip.hpp"

#include "Chrono.hpp"
#include "DigiLed.h"
#include "RgbColor.hpp"

#include "Animation/Ball.hpp"

#include "spi.h"

#include <cmath>

static bool blinkin = false;

union AnimationUnion {
    Animation::Animation animation;
    Animation::Ball ball;
};

static constexpr size_t ANIMATION_COUNT = 10;

static uint8_t animationsMemory[sizeof(AnimationUnion) * ANIMATION_COUNT];
static AnimationUnion* animations = (AnimationUnion*)animationsMemory;
static RgbColor frame[LED_FRAME_SIZE];

void ledstrip_init() {
    DigiLed_init(&hspi2);
    DigiLed_setAllRGB(0x00);
    DigiLed_setAllIllumination(0x1f); // 0 - 31    0x00 - 0x1f
    DigiLed_update(0);

    // fill up with dummy animations
    for (size_t i = 0; i < ANIMATION_COUNT; ++i) {
        new (animations + i) Animation::Animation(frame);
    }

    // ball anim
    new (animations) Animation::Ball(frame, {0xff, 0xff, 0});
}

void ledstrip_tick(Eigen::Vector3f const& accelMg, Eigen::Vector3f const& angularRateMdps) {
    static Chrono::MsTimer timer(Chrono::Milliseconds(2));
    if (!timer.done())
        return;
    auto tickTime = timer.elapsedTime();
    timer.advance();
    float tickTimeS = (float)tickTime.count() / 1000.0f;

    // reinit frame
    for (size_t i = 0; i < LED_FRAME_SIZE; ++i) {
        new (frame + i) RgbColor();
    }

    for (size_t i = 0; i < ANIMATION_COUNT; ++i) {
        Animation::Animation* animation = (Animation::Animation*)(animations + i);
        if (!animation->done())
            animation->tick(tickTimeS, accelMg, angularRateMdps);
    }

    for (int i = 0; i < LED_FRAME_SIZE; ++i) {
        DigiLed_setColor(i, frame[i].r(), frame[i].g(), frame[i].b());
    }

    DigiLed_update(0);
}

void ledstrip_setblinkin(bool b) {
    blinkin = b;
}
