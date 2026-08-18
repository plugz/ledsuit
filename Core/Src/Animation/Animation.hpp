#ifndef __ANIMATION_ANIMATION_HPP__
#define __ANIMATION_ANIMATION_HPP__

#include "Chrono.hpp"
#include "Eigen/Core"

struct RgbColor;

namespace Animation {

class Animation {
public:
    Animation(RgbColor* frame, bool infinite = false) : _frame(frame), _infinite(infinite) {
        _timer.reset();
    }

    virtual void tick(float tickTime,
            Eigen::Vector3f const& accelMg,
            Eigen::Vector3f const& angularRateMdps) {}

    virtual bool done() {
        return (!_infinite) && _timer.done();
    }

protected:
    RgbColor* _frame;
    bool _infinite;
    Chrono::MsTimer _timer;
};

} // namespace Animation

#endif
