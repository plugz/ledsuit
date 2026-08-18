#include "Ball.hpp"
#include "Boom.hpp"

#include "DigiLed.h"

namespace Animation {

void Ball::tick(float tickTime,
        Eigen::Vector3f const& accelMg,
        Eigen::Vector3f const& angularRateMdps) {
    float accel = -accelMg[_accelIdx] / 1000.0f;

    _speed += accel * 100 * tickTime;
    _position += _speed * tickTime;

    if (_position > ((LED_FRAME_SIZE * 1) - 1)) {
        if (_speed != accel * 100 * tickTime)
            _createBoom(_speed);
        _position = (LED_FRAME_SIZE * 1) - 1;
        _speed = 0;
    }
    else if (_position < 0) {
        if (_speed != accel * 100 * tickTime)
            _createBoom(_speed);
        _position = 0;
        _speed = 0;
    }

    for (size_t frameIdx = 0; frameIdx < LED_FRAME_SIZE; ++frameIdx) {
        float distance = std::abs((float)frameIdx - _position);
        float level = (_size - distance);
        if (level > 0) {
            animationRgbFrame[frameIdx] += _color * level;
        }
    }
}

void Ball::_createBoom(float power) {
    auto newAnim = newAnimationMemory();
    if (!newAnim)
        return;

    new (newAnim) Boom(_color, _position, power / 10.0f);
}

} // namespace Animation
