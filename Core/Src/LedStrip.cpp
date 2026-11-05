#include "LedStrip.hpp"

#include "Chrono.hpp"
#include "DigiLed.h"
#include "RgbColor.hpp"

#include "Animation/Ball.hpp"
#include "Animation/Breathing.hpp"
#include "Animation/HatBall.hpp"
#include "Animation/Plain.hpp"
#include "Animation/Strobe.hpp"

#include "spi.h"

#include <cmath>

// Animation stuff

union AnimationUnion {
    Animation::Animation animation;
    Animation::Ball ball;
    Animation::Breathing breathing;
    Animation::HatBall hatball;
    Animation::Plain plain;
    Animation::Strobe strobe;
};

static constexpr size_t ANIMATION_COUNT = 10;

static uint8_t animationsMemory[sizeof(AnimationUnion) * ANIMATION_COUNT];
static AnimationUnion* animations = (AnimationUnion*)animationsMemory;

Animation::Animation* newAnimationMemory() {
    for (size_t i = 0; i < ANIMATION_COUNT; ++i) {
        auto animation = (Animation::Animation*)(animations + i);
        if (animation->done())
            return animation;
    }
    return nullptr;
}

// Strip Parts

static RgbColor frame[LED_FRAME_SIZE];

Eigen::Vector3f stripBottomCirclePositions[15] = {
    {-2.0f, -5.0f, 0.0f},
    {-8.0f, -4.0f, 0.0f},
    {-4.0f, -3.0f, 0.0f},
    {-5.0f, -2.5f, 0.0f},
    {-4.0f, 0.5f, 0.0f},
    {-3.0f, 3.0f, 0.0f},
    {-2.0f, 4.0f, 0.0f},
    {0.0f, 4.9f, 0.0f},
    {2.0f, 4.0f, 0.0f},
    {3.0f, 3.0f, 0.0f},
    {4.0f, 0.5f, 0.0f},
    {4.9f, -2.5f, 0.0f},
    {4.0f, -3.0f, 0.0f},
    {3.0f, -4.0f, 0.0f},
    {2.0f, -5.5f, 0.0f},
};

static Animation::StripPart stripBottomCircle{frame, 15, stripBottomCirclePositions};
static Animation::StripPart stripMiddleLine{frame + 15, 19, nullptr};
static Animation::StripPart stripTop{frame + 15 + 19, 1, nullptr};
static Animation::StripPart stripAll{frame, 35, nullptr};

// init

void ledstrip_init() {
    DigiLed_init(&hspi2);
    DigiLed_setAllRGB(0x00);
    DigiLed_setAllIllumination(0x1f); // 0 - 31    0x00 - 0x1f
    DigiLed_update(0);

    // fill up with dummy animations
    for (size_t i = 0; i < ANIMATION_COUNT; ++i) {
        new (animations + i) Animation::Animation();
    }

    // anims
    new (newAnimationMemory()) Animation::HatBall(&stripBottomCircle, {0xff, 0x99, 0});
    new (newAnimationMemory()) Animation::Ball(&stripMiddleLine, {0xff, 0x99, 0});
    new (newAnimationMemory()) Animation::Breathing(&stripTop, {0x50, 0x20, 0}, Chrono::Milliseconds(2000));
    new (newAnimationMemory()) Animation::Strobe(&stripTop, {0xff, 0xff, 0xff}, {0x00, 0x00, 0x00}, Chrono::Milliseconds{25}, Chrono::Milliseconds{475});
}

// tick

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
