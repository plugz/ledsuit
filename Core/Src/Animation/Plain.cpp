#include "Plain.hpp"

#include "DigiLed.h"

namespace Animation {

void Plain::tick(float tickTime,
        Eigen::Vector3f const& accelMg,
        Eigen::Vector3f const& angularRateMdps) {

    (void)tickTime;
    (void)accelMg;
    (void)angularRateMdps;

    for (size_t frameIdx = 0; frameIdx < LED_FRAME_SIZE; ++frameIdx) {
        animationRgbFrame[frameIdx] += _color;
    }
}

} // namespace Animation
