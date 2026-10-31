#include "HatBall.hpp"
#include "Boom.hpp"

#include "DigiLed.h"

namespace Animation {

HatBall::HatBall(RgbColor color) :
    Animation(true),
    _color(color) {
    _position = LED_FRAME_SIZE / 2;
}


void HatBall::tick(float tickTime,
        Eigen::Vector3f const& accelMg,
        Eigen::Vector3f const& angularRateMdps) {

    (void)accelMg;

    float accel = (-angularRateMdps[_accelIdx] / 1000.0f);

    _speed += accel * 2.6 * tickTime;
    _position += _speed * tickTime;

    while (_position < 0)
        _position += LED_FRAME_SIZE;
    while (_position > LED_FRAME_SIZE)
        _position -= LED_FRAME_SIZE;


    for (size_t frameIdx = 0; frameIdx < LED_FRAME_SIZE; ++frameIdx) {
        float distance = std::abs((float)frameIdx - _position);
        float level = (_size - distance);
        if (level > 0) {
            animationRgbFrame[frameIdx] += _color * level;
        }
    }

    _speed *= 0.95;
}

} // namespace Animation
