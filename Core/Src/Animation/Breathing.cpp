#include "Breathing.hpp"

#include "DigiLed.h"

namespace Animation {

void Breathing::tick(float tickTime,
        Eigen::Vector3f const& accelMg,
        Eigen::Vector3f const& angularRateMdps) {

    (void)accelMg;
    (void)angularRateMdps;

    _timer.advance();

    float elapsedS = (float)_timer.elapsedTime().count() / 1000.0f;
    float periodS = (float)_period.count() / 1000.0f;

    float level = (sin((elapsedS / periodS) * 2 * M_PI) + 1.0f) / 2.0f;

    for (size_t frameIdx = 0; frameIdx < LED_FRAME_SIZE; ++frameIdx) {
        animationRgbFrame[frameIdx] += _color * level;
    }
}

} // namespace Animation

