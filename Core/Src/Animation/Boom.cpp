#include "Boom.hpp"

#include "DigiLed.h"

namespace Animation {

void Boom::tick(float tickTime,
        Eigen::Vector3f const& accelMg,
        Eigen::Vector3f const& angularRateMdps) {

    float elapsedS = (float)_timer.elapsedTime().count() / 1000.0f;

    float size;
    if (elapsedS < 0.1f)
        size = _power * (elapsedS / 0.1f);
    else
        size = _power * ((0.9f - (elapsedS - 0.1f)) / 0.9f);

    for (size_t frameIdx = 0; frameIdx < LED_FRAME_SIZE; ++frameIdx) {
        float distance = std::abs((float)frameIdx - _position);
        float level = (size - distance);
        for (int i = 8; i >= 2; --i) {
            if (level > i)
                animationRgbFrame[frameIdx] += RgbColor{0x20, 0x20, 0x20};
        }
        if (level > 0) {
            animationRgbFrame[frameIdx] += _color * level;
        }
    }
}

} // namespace Animation

