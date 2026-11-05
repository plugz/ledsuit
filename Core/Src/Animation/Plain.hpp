#ifndef __ANIMATION_PLAIN_HPP__
#define __ANIMATION_PLAIN_HPP__

#include "Animation.hpp"
#include "RgbColor.hpp"

namespace Animation {

class Plain : public Animation {
public:
    Plain(StripPart* stripPart, RgbColor color) :
        Animation(stripPart, true),
        _color(color) {
        }

    virtual void tick(float tickTime,
            Eigen::Vector3f const& accelMg,
            Eigen::Vector3f const& angularRateMdps) override;

protected:
    RgbColor _color;
};

} // namespace Animation

#endif
