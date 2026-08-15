#include "ledsuit.h"

#include "Imu.hpp"
#include "LedStrip.hpp"
#include "OnBoardLeds.hpp"

#include "gpio.h"
#include "main.h"

#include "Eigen"

#include <cmath>
#include <cstdio>
#include <cstring>

#define ANGLEHISTSIZE 32

// for debug printf
int __io_putchar(int ch)
{
    // Write character to ITM ch.0
    ITM_SendChar(ch);
    return(ch);
}

void ledsuit_init_beforeloop() {
    pxm_leds_off();

    ledstrip_init();

    imu_init();
}

void ledsuit_tick() {
    static int counter = 0;
    static int ascend = 1;



    static float yaw = 0.0f;
    static float yawLenMg = 1.0f;

    static Eigen::Quaternionf accelQuaternion;


    static float imu_acceleration_mg[3];
    static float imu_angular_rate_mdps[3];
    static float imu_temperature_degC;

    imu_tick();

    if (imu_accel_fetch(imu_acceleration_mg)) {
        for (unsigned int i = 0; i < 3; ++i) {
            if (imu_acceleration_mg[i] >= 0) {
                pxm_led_set(i * 2 + 0, (int)(imu_acceleration_mg[i] / 4));
                pxm_led_set(i * 2 + 1, 0);
            }
            else {
                pxm_led_set(i * 2 + 0, 0);
                pxm_led_set(i * 2 + 1, (int)(-imu_acceleration_mg[i] / 4));
            }
        }
        // calc acc vector Yaw/Pitch
        yaw = atan2(imu_acceleration_mg[0], imu_acceleration_mg[2]);
        yaw += M_PI;

        yawLenMg = sqrtf(imu_acceleration_mg[0] * imu_acceleration_mg[0] + imu_acceleration_mg[1] * imu_acceleration_mg[1]);

    }

    if (imu_angular_rate_fetch(imu_angular_rate_mdps)) {
        static float prevAngles[ANGLEHISTSIZE] = {0.0f,};
        static int prevAnglesIdx = 0;

        prevAngles[prevAnglesIdx] = imu_angular_rate_mdps[1];// + imu_angular_rate_mdps[1] + imu_angular_rate_mdps[2];
        if (prevAngles[prevAnglesIdx] < 0.0f)
            prevAngles[prevAnglesIdx] = -prevAngles[prevAnglesIdx]; // abs
        if (prevAngles[prevAnglesIdx] > 10000)
            prevAngles[prevAnglesIdx] = 10000; // limit
        prevAnglesIdx = (prevAnglesIdx + 1) % ANGLEHISTSIZE;

        float avg = 0;
        for (unsigned int i = 0; i < ANGLEHISTSIZE; ++i) {
            avg += prevAngles[i];
        }
        avg /= ANGLEHISTSIZE;

        static uint32_t blinkinTime = 0;
        uint32_t curTime = HAL_GetTick();

        if (avg > 9500) {
            if (blinkinTime) {
                ledstrip_setblinkin(curTime - blinkinTime > 600);
            }
            else {
                ledstrip_setblinkin(false);
                blinkinTime = curTime;
            }
        }
        else {
            ledstrip_setblinkin(false);
            blinkinTime = 0;
        }

        /*		  static uint32_t firstBigAngleTime = 0;
                  uint32_t curTime = HAL_GetTick();

                  if (imu_angular_rate_mdps[0] + imu_angular_rate_mdps[1] + imu_angular_rate_mdps[2] > 30000) {
                  if (firstBigAngleTime) {
                  blinkin = (curTime - firstBigAngleTime > 600);
                  }
                  else {
                  blinkin = false;
                  firstBigAngleTime = curTime;
                  }
                  }
                  else {
                  firstBigAngleTime = 0;
                  blinkin = false;
                  }*/

        /*for (unsigned int i = 0; i < 3; ++i) {
          if (imu_angular_rate_mdps[i] >= 0) {
          int rate = imu_angular_rate_mdps[i] / 16000;
          rate = rate < 0xff ? rate : 0xff;
          DigiLed_setRGB(i * 2 + 0, (rate << 16) & 0xff0000);
          DigiLed_setRGB(i * 2 + 1, 0);
          }
          else {
          int rate = -imu_angular_rate_mdps[i] / 16000;
          rate = rate < 0xff ? rate : 0xff;
          DigiLed_setRGB(i * 2 + 0, 0);
          DigiLed_setRGB(i * 2 + 1, (rate << 16) & 0xff0000);
          }
          }
          DigiLed_update(0);*/
        //printf("Angular rate [mdps]:%4.2f\t%4.2f\t%4.2f\r\n",
        //	  imu_angular_rate_mdps[0], imu_angular_rate_mdps[1], imu_angular_rate_mdps[2]);
    }

    ledstrip_tick(yaw);

    if (imu_temperature_fetch(&imu_temperature_degC)) {
        // Read temperature data
        if (imu_temperature_degC > 20) {
            pxm_led_set(6, (int)((imu_temperature_degC - 20) * 20));
            pxm_led_set(7, 0);
        }
        else {
            pxm_led_set(6, 0);
            pxm_led_set(7, (int)((20 - imu_temperature_degC) * 20));
        }
        //printf("Temperature [degC]:%6.2f\r\n",
        //	  imu_temperature_degC);
    }





    //int counter2 = 0xff - (counter & 0xff);

    //	for (unsigned int i = 0; i < 8; ++i) {
    //		pxm_led_set(i, counter);
    //	}

    //    for (unsigned int i = 0; i < DigiLed_getFrameSize(); ++i) {
    //      DigiLed_setRGB(i,
    //        ((((i % 2) ? counter2 : 0) << 16) & 0xff0000) |
    //        ((((i % 3) ? (counter * i) : counter) << 8) & 0x00ff00) |
    //        (((i % 2) ? counter : counter2) & 0x0000ff)
    //        );
    //    }
    //    DigiLed_update(0);

    //   HAL_Delay(20);
    if (counter == 0) {
        //    HAL_Delay(980);
        ascend = 1;
    }
    else if (counter == 0xff)
        ascend = 0;
    if (ascend)
        counter = (counter + 1) % 0x100;
    else
        counter = (counter - 1) % 0x100;
    //printf("COUNTER:%i\n", counter);
}
