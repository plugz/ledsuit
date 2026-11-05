#include "Strobe.hpp"

#include "DigiLed.h"

namespace Animation {

void Strobe::tick(float tickTime,
        Eigen::Vector3f const& accelMg,
        Eigen::Vector3f const& angularRateMdps) {

    (void)accelMg;
    (void)angularRateMdps;

    auto const period = _period0 + _period1;

    _timer.advance(period);

    RgbColor color;
    if (_timer.elapsedTime() < _period0)
        color = _color0;
    else
        color = _color1;

    for (size_t frameIdx = 0; frameIdx < LED_FRAME_SIZE; ++frameIdx) {
        animationRgbFrame[frameIdx] += color;
    }
}

} // namespace Animation
