#ifndef __ANIMATION_BALL_HPP__
#define __ANIMATION_BALL_HPP__

#include "Animation.hpp"
#include "RgbColor.hpp"

namespace Animation {

class Ball : public Animation {
public:
    Ball(RgbColor color) :
        Animation(true),
        _color(color) {
        }

    virtual void tick(float tickTime,
            Eigen::Vector3f const& accelMg,
            Eigen::Vector3f const& angularRateMdps) override;

protected:
    void _createBoom(float power);

protected:
    size_t _accelIdx = 0;
    RgbColor _color;
    float _position = 0.0f;
    float _speed = 0.0f;
    float _size = 1.0f;
};

} // namespace Animation

#endif

