#include "Plain.hpp"

namespace Animation {

void Plain::tick(float tickTime,
        Eigen::Vector3f const& accelMg,
        Eigen::Vector3f const& angularRateMdps) {

    (void)tickTime;
    (void)accelMg;
    (void)angularRateMdps;

    for (size_t frameIdx = 0; frameIdx < _stripPart.size; ++frameIdx) {
        _stripPart.begin[frameIdx] += _color;
    }
}

} // namespace Animation
