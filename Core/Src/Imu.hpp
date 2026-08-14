#ifndef __IMU_HPP__
#define __IMU_HPP__

void imu_init();

void imu_tick();

bool imu_accel_fetch(float* result_accel_mg);
bool imu_angular_rate_fetch(float* result_angular_rate_mdps);
bool imu_temperature_fetch(float* result_temperature_degC);

#endif
