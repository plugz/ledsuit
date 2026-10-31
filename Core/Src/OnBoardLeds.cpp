#include "OnBoardLeds.hpp"

#include "tim.h"

/*static uint32_t const PXM_LEDS_GREEN_PWM_MIN = 0x006a;
static uint32_t const PXM_LEDS_GREEN_PWM_MAX = 0x004d;
static uint32_t const PXM_LEDS_ORANGE_PWM_MIN = 0x006a;
static uint32_t const PXM_LEDS_ORANGE_PWM_MAX = 0x0000;
static uint32_t const PXM_LEDS_WHITE_PWM_MIN = 0x00cc;
static uint32_t const PXM_LEDS_WHITE_PWM_MAX = 0x0000;*/

static uint32_t const PXM_LEDS_GREEN_PWM_MIN = 100;
static uint32_t const PXM_LEDS_GREEN_PWM_MAX = 77;
static uint32_t const PXM_LEDS_ORANGE_PWM_MIN = 100;
static uint32_t const PXM_LEDS_ORANGE_PWM_MAX = 0;
static uint32_t const PXM_LEDS_WHITE_PWM_MIN = 100;
static uint32_t const PXM_LEDS_WHITE_PWM_MAX = 50;

static uint32_t const PXM_LEDS_PWM_MIN[8] = {
    PXM_LEDS_ORANGE_PWM_MIN,
    PXM_LEDS_GREEN_PWM_MIN,
    PXM_LEDS_GREEN_PWM_MIN,
    PXM_LEDS_ORANGE_PWM_MIN,
    PXM_LEDS_ORANGE_PWM_MIN,
    PXM_LEDS_GREEN_PWM_MIN,
    PXM_LEDS_WHITE_PWM_MIN,
    PXM_LEDS_WHITE_PWM_MIN,
};

static uint32_t const PXM_LEDS_PWM_MAX[8] = {
    PXM_LEDS_ORANGE_PWM_MAX,
    PXM_LEDS_GREEN_PWM_MAX,
    PXM_LEDS_GREEN_PWM_MAX,
    PXM_LEDS_ORANGE_PWM_MAX,
    PXM_LEDS_ORANGE_PWM_MAX,
    PXM_LEDS_GREEN_PWM_MAX,
    PXM_LEDS_WHITE_PWM_MAX,
    PXM_LEDS_WHITE_PWM_MAX,
};

static TIM_HandleTypeDef* const PXM_LEDS_TIMER[8] = {
    &htim4,
    &htim4,
    &htim4,
    &htim4,
    &htim2,
    &htim2,
    &htim2,
    &htim2
};

static uint32_t const PXM_LEDS_CHAN[8] = {
    TIM_CHANNEL_4,
    TIM_CHANNEL_3,
    TIM_CHANNEL_1,
    TIM_CHANNEL_2,
    TIM_CHANNEL_2,
    TIM_CHANNEL_1,
    TIM_CHANNEL_4,
    TIM_CHANNEL_3
};

// leds orders :
// x0, x1, y0, y1, z0, z1, white0, white1


// val 0-255
void pxm_led_set(unsigned int idx, unsigned int val) {

    {
        static uint32_t prevTime[8] = {0};
        uint32_t curTime = HAL_GetTick();
        if (curTime - prevTime[idx] < 20) {
            return;
        }
        prevTime[idx] += ((curTime - prevTime[idx]) / 20) * 20;
    }

    static unsigned int pulses[8] = {0};

    //unsigned int vaval = val;

    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    // max is 0xff
    val = val < 300 ? val : 300;
    // revert because led is activated by a sink
    val = 300 - val;
    unsigned int newPulse = (val * (PXM_LEDS_PWM_MIN[idx] - PXM_LEDS_PWM_MAX[idx])) / 300 + PXM_LEDS_PWM_MAX[idx];
    if (newPulse != pulses[idx]) {
        pulses[idx] = newPulse;
        sConfigOC.Pulse = newPulse;
        HAL_TIM_PWM_ConfigChannel(PXM_LEDS_TIMER[idx], &sConfigOC, PXM_LEDS_CHAN[idx]);
        HAL_TIM_PWM_Start(PXM_LEDS_TIMER[idx], PXM_LEDS_CHAN[idx]);
    }


    //printf("led%u vaval %u val%u pulse%u\n", idx, vaval, val, (unsigned int)sConfigOC.Pulse);
}

void pxm_leds_off() {
    for (unsigned int i = 0; i < 8; ++i) {
        pxm_led_set(i, 0);
    }
}


