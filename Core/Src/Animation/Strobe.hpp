#ifndef __ANIMATION_STROBE_HPP__
#define __ANIMATION_STROBE_HPP__

#include "Animation.hpp"
#include "RgbColor.hpp"

namespace Animation {

class Strobe : public Animation {
public:
    Strobe(StripPart* stripPart, RgbColor color0, RgbColor color1,
           Chrono::Milliseconds period0, Chrono::Milliseconds period1) :
        Animation(stripPart, true),
        _color0(color0),
        _color1(color1),
        _period0(period0),
        _period1(period1) {
        }

    virtual void tick(float tickTime,
            Eigen::Vector3f const& accelMg,
            Eigen::Vector3f const& angularRateMdps) override;

protected:
    RgbColor _color0;
    RgbColor _color1;
    Chrono::Milliseconds _period0;
    Chrono::Milliseconds _period1;
};

} // namespace Animation

#endif
