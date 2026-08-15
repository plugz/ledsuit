#ifndef __LEDSTRIP_HPP__
#define __LEDSTRIP_HPP__

#include "Eigen/Core"

void ledstrip_init();
void ledstrip_tick(Eigen::Vector3f const& accelMg, Eigen::Vector3f const& angularRateMdps);

void ledstrip_setblinkin(bool blinkin);

#endif
