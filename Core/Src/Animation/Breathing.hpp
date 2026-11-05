#ifndef __ANIMATION_BREATHING_HPP__
#define __ANIMATION_BREATHING_HPP__

#include "Animation.hpp"
#include "RgbColor.hpp"

namespace Animation {

class Breathing : public Animation {
public:
    Breathing(StripPart* stripPart, RgbColor color, Chrono::Milliseconds period) :
        Animation(stripPart, true),
        _color(color),
        _period(period) {
        }

    virtual void tick(float tickTime,
            Eigen::Vector3f const& accelMg,
            Eigen::Vector3f const& angularRateMdps) override;

protected:
    RgbColor _color;
    Chrono::Milliseconds _period;
};

} // namespace Animation

#endif
