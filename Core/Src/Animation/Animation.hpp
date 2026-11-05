#ifndef __ANIMATION_ANIMATION_HPP__
#define __ANIMATION_ANIMATION_HPP__

#include "Chrono.hpp"
#include "Eigen/Core"

struct RgbColor;

namespace Animation {

struct StripPart {
    RgbColor*              begin = nullptr;
    size_t                 size = 0;
    Eigen::Vector3f const* positions = nullptr;
};

class Animation {
public:
    Animation(StripPart* stripPart = nullptr, bool infinite = false) :
        _stripPart(*stripPart),
        _infinite(infinite) {
        _timer.reset();
    }

    virtual void tick(float tickTime,
            Eigen::Vector3f const& accelMg,
            Eigen::Vector3f const& angularRateMdps) {}

    virtual bool done() {
        return (!_infinite) && _timer.done();
    }

protected:
    StripPart& _stripPart;
    bool _infinite;
    Chrono::MsTimer _timer;
};

} // namespace Animation

Animation::Animation* newAnimationMemory();

#endif
