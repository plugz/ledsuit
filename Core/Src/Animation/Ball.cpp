#include "Ball.hpp"

#include "DigiLed.h"

namespace Animation {

void Ball::tick(float tickTime,
        Eigen::Vector3f const& accelMg,
        Eigen::Vector3f const& angularRateMdps) {
    float accel = -accelMg[_accelIdx] / 1000.0f;

    _speed += accel * 10 * tickTime;
    _position += _speed * tickTime;
    if (_position > ((LED_FRAME_SIZE * 1) - 1)) {
        _position = (LED_FRAME_SIZE * 1) - 1;
        _speed = 0;
    }
    if (_position < 0) {
        _position = 0;
        _speed = 0;
    }

    for (size_t frameIdx = 0; frameIdx < LED_FRAME_SIZE; ++frameIdx) {
        float distance = std::abs((float)frameIdx - _position);
        float level = (_size - distance);
        if (level > 0) {
            _frame[frameIdx] += _color * level;
        }
    }
}

} // namespace Animation
