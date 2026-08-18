#ifndef __ANIMATION_BOOM_HPP__
#define __ANIMATION_BOOM_HPP__

#include "Animation.hpp"
#include "RgbColor.hpp"

namespace Animation {

class Boom : public Animation {
public:
    Boom(RgbColor color, float position, float power) :
        Animation(false),
        _color(color),
        _position(position),
        _power(power) {
            if (_power > 10.0f)
                _power = 10.0f;
            _timer.reset(Chrono::Milliseconds(1000));
        }

    virtual void tick(float tickTime,
            Eigen::Vector3f const& accelMg,
            Eigen::Vector3f const& angularRateMdps) override;

protected:
    RgbColor _color;
    float _position;
    float _power;
};

} // namespace Animation

#endif


