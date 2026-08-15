#ifndef __IMU_HPP__
#define __IMU_HPP__

#include "Eigen/Core"

void imu_init();

void imu_tick();

bool imu_accel_fetch(Eigen::Vector3f* result_accel_mg);
bool imu_angular_rate_fetch(Eigen::Vector3f* result_angular_rate_mdps);
bool imu_temperature_fetch(float* result_temperature_degC);

#endif
