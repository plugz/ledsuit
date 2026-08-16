#include "ledsuit.h"

#include "Imu.hpp"
#include "LedStrip.hpp"
#include "OnBoardLeds.hpp"

#include "Eigen/Core"
#include "gpio.h"
#include "main.h"
#include "usbd_cdc_if.h"

#include <cmath>
#include <cstdio>
#include <cstring>

#define HISTSIZE 16

// for debug printf
// XXX test itm out
//int __io_putchar(int ch)
//{
//    // Write character to ITM ch.0
//    ITM_SendChar(ch);
//    return(ch);
//}

//#ifdef USB_DEBUG_OUTPUT
//int _write(int file, char *ptr, int len) {
//    (void)file;
//    CDC_Transmit_FS((uint8_t*) ptr, len);
//    return len;
//}
//#endif

void ledsuit_init_beforeloop() {
    HAL_Delay(200); // let low-level stuff initialize

    pxm_leds_off();

    ledstrip_init();

    imu_init();
}

void ledsuit_tick() {
    static int accelIdx = 0;
    static Eigen::Vector3f accelMg[HISTSIZE];
    static Eigen::Vector3f accelMgAvg;
    static int angularRateIdx = 0;
    static Eigen::Vector3f angularRateMdps[HISTSIZE];
    static Eigen::Vector3f angularRateMdpsAvg;

    imu_tick();

    imu_accel_fetch(&accelMgAvg);

    //if (imu_accel_fetch(accelMg + accelIdx)) {
    //    accelIdx = (accelIdx + 1) % std::size(accelMg);

    //    accelMgAvg << 0, 0, 0;
    //    for (size_t i = 0; i < std::size(accelMg); ++i) {
    //        accelMgAvg += accelMg[i];
    //    }
    //    accelMgAvg /= std::size(accelMg);
    //}

    if (imu_angular_rate_fetch(angularRateMdps + angularRateIdx)) {
        angularRateIdx = (angularRateIdx + 1) % std::size(angularRateMdps);

        angularRateMdpsAvg << 0, 0, 0;
        for (size_t i = 0; i < std::size(angularRateMdps); ++i) {
            angularRateMdpsAvg += angularRateMdps[i];
        }
        angularRateMdpsAvg /= std::size(angularRateMdps);
    }

    ledstrip_tick(accelMgAvg, angularRateMdpsAvg);
}
