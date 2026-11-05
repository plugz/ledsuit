#include "HatBall.hpp"
#include "Boom.hpp"

namespace Animation {

HatBall::HatBall(StripPart* stripPart, RgbColor color) :
    Animation(stripPart, true),
    _color(color) {
    _position = _stripPart.size / 2;
}


void HatBall::tick(float tickTime,
        Eigen::Vector3f const& accelMg,
        Eigen::Vector3f const& angularRateMdps) {

    (void)accelMg;

    float accel = (-angularRateMdps[_accelIdx] / 1000.0f);

    _speed += accel * 2.6 * tickTime;
    _position += _speed * tickTime;

    while (_position < 0)
        _position += _stripPart.size;
    while (_position > _stripPart.size)
        _position -= _stripPart.size;


    for (size_t frameIdx = 0; frameIdx < _stripPart.size; ++frameIdx) {
        float distance = std::abs((float)frameIdx - _position);
        float level = (_size - distance);
        if (level > 0) {
            _stripPart.begin[frameIdx] += _color * level;
        }
    }

    _speed *= 0.95;
}

} // namespace Animation
