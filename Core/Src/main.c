/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "DigiLed.h"
#include "lsm6ds3tr-c_reg.h"
#include <math.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

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

static void* const PXM_LEDS_TIMER[8] = {
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


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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

static void pxm_leds_off() {
	for (unsigned int i = 0; i < 8; ++i) {
		pxm_led_set(i, 0);
	}
}

// for debug printf
int __io_putchar(int ch)
{
 // Write character to ITM ch.0
 ITM_SendChar(ch);
 return(ch);
}


// LSM6DS3TR-C IMU

#define IMU_SENSOR_BUS hspi1
#define IMU_BOOT_TIME 15 // ms
#define IMU_TX_BUF_DIM 1000

static int16_t imu_data_raw_acceleration[3];
static int16_t imu_data_raw_angular_rate[3];
static int16_t imu_data_raw_temperature;
static float imu_acceleration_mg[3];
static float imu_angular_rate_mdps[3];
static float imu_temperature_degC;
static uint8_t imu_whoamI;
static uint8_t imu_rst;

static int32_t imu_platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len) {
    HAL_GPIO_WritePin(SPI1_NCS_GYR_GPIO_Port, SPI1_NCS_GYR_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(handle, &reg, 1, 1000);
    HAL_SPI_Transmit(handle, (uint8_t*) bufp, len, 1000);
    HAL_GPIO_WritePin(SPI1_NCS_GYR_GPIO_Port, SPI1_NCS_GYR_Pin, GPIO_PIN_SET);
    return 0;
}
static int32_t imu_platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len) {
    reg |= 0x80;
    HAL_GPIO_WritePin(SPI1_NCS_GYR_GPIO_Port, SPI1_NCS_GYR_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(handle, &reg, 1, 1000);
    HAL_SPI_Receive(handle, bufp, len, 1000);
    HAL_GPIO_WritePin(SPI1_NCS_GYR_GPIO_Port, SPI1_NCS_GYR_Pin, GPIO_PIN_SET);
    return 0;
}
static void imu_platform_delay(uint32_t ms) {
	HAL_Delay(ms);
}
static void imu_platform_init(void) {
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_USB_DEVICE_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */


  int counter = 0;
  int ascend = 1;
  pxm_leds_off();

  DigiLed_init(&hspi2);
  DigiLed_setAllRGB(0x00);
  DigiLed_setAllIllumination(0x09); // 0 - 31    0x00 - 0x1f
  DigiLed_update(0);

  uint8_t myLedz[LED_FRAME_SIZE] = {0,};

  // IMU

  stmdev_ctx_t imu_dev_ctx = {0};
  imu_dev_ctx.write_reg = imu_platform_write;
  imu_dev_ctx.read_reg = imu_platform_read;
  imu_dev_ctx.mdelay = imu_platform_delay;
  imu_dev_ctx.handle = &IMU_SENSOR_BUS;

  imu_platform_init();
  imu_platform_delay(IMU_BOOT_TIME);
  imu_whoamI = 0;
  lsm6ds3tr_c_device_id_get(&imu_dev_ctx, &imu_whoamI);
  if (imu_whoamI != LSM6DS3TR_C_ID) {
	  printf("NOT THE CORRECT IMU DEVICE ! FUCK\n");
	  while (1) { HAL_Delay(20); }
  }

  lsm6ds3tr_c_reset_set(&imu_dev_ctx, PROPERTY_ENABLE);

  // wait for reset
  do {
	printf("waitin' for imu reset\n");
    lsm6ds3tr_c_reset_get(&imu_dev_ctx, &imu_rst);
  } while (imu_rst);

  // Enable Block Data Update
  lsm6ds3tr_c_block_data_update_set(&imu_dev_ctx, PROPERTY_ENABLE);
  // Set Output Data Rate
  lsm6ds3tr_c_xl_data_rate_set(&imu_dev_ctx, LSM6DS3TR_C_XL_ODR_104Hz);
  lsm6ds3tr_c_gy_data_rate_set(&imu_dev_ctx, LSM6DS3TR_C_GY_ODR_104Hz);
  // Set full scale
  lsm6ds3tr_c_xl_full_scale_set(&imu_dev_ctx, LSM6DS3TR_C_16g);
  lsm6ds3tr_c_gy_full_scale_set(&imu_dev_ctx, LSM6DS3TR_C_2000dps);
  // Configure filtering chain(No aux interface)
  // Accelerometer - analog filter
  lsm6ds3tr_c_xl_filter_analog_set(&imu_dev_ctx, LSM6DS3TR_C_XL_ANA_BW_400Hz);
  // Accelerometer - LPF1 path ( LPF2 not used )
  // lsm6ds3tr_c_xl_lp1_bandwidth_set(&dev_ctx, LSM6DS3TR_C_XL_LP1_ODR_DIV_4);
  // Accelerometer - LPF1 + LPF2 path
  lsm6ds3tr_c_xl_lp2_bandwidth_set(&imu_dev_ctx, LSM6DS3TR_C_XL_LOW_NOISE_LP_ODR_DIV_100);
  // Accelerometer - High Pass / Slope path
  // lsm6ds3tr_c_xl_reference_mode_set(&dev_ctx, PROPERTY_DISABLE);
  // lsm6ds3tr_c_xl_hp_bandwidth_set(&dev_ctx, LSM6DS3TR_C_XL_HP_ODR_DIV_100);
  // Gyroscope - filtering chain
  lsm6ds3tr_c_gy_band_pass_set(&imu_dev_ctx, LSM6DS3TR_C_HP_260mHz_LP1_STRONG);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	    static bool blinkin = true;
	    static float yaw = 0.0f;
	    static float yawLenMg = 1.0f;

	    // Read output only if new value is available
	    lsm6ds3tr_c_reg_t reg;
	    lsm6ds3tr_c_status_reg_get(&imu_dev_ctx, &reg.status_reg);

	    if (reg.status_reg.xlda) {
	      // Read magnetic field data
	      memset(imu_data_raw_acceleration, 0x00, sizeof(imu_data_raw_acceleration));
	      lsm6ds3tr_c_acceleration_raw_get(&imu_dev_ctx, imu_data_raw_acceleration);
	      imu_acceleration_mg[0] = lsm6ds3tr_c_from_fs16g_to_mg(imu_data_raw_acceleration[0]);
	      imu_acceleration_mg[1] = lsm6ds3tr_c_from_fs16g_to_mg(imu_data_raw_acceleration[1]);
	      imu_acceleration_mg[2] = lsm6ds3tr_c_from_fs16g_to_mg(imu_data_raw_acceleration[2]);
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
	      //printf("Acceleration [mg]:%4.2f\t%4.2f\t%4.2f\r\n",
	      //       imu_acceleration_mg[0], imu_acceleration_mg[1], imu_acceleration_mg[2]);
	    }

	    if (reg.status_reg.gda) {
	      // Read magnetic field data
	      memset(imu_data_raw_angular_rate, 0x00, sizeof(imu_data_raw_angular_rate));
	      lsm6ds3tr_c_angular_rate_raw_get(&imu_dev_ctx, imu_data_raw_angular_rate);
	      imu_angular_rate_mdps[0] = lsm6ds3tr_c_from_fs2000dps_to_mdps(imu_data_raw_angular_rate[0]);
	      imu_angular_rate_mdps[1] = lsm6ds3tr_c_from_fs2000dps_to_mdps(imu_data_raw_angular_rate[1]);
	      imu_angular_rate_mdps[2] = lsm6ds3tr_c_from_fs2000dps_to_mdps(imu_data_raw_angular_rate[2]);


#define ANGLEHISTSIZE 32
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

	      //blinkin = (avg > 9500);

	      		  static uint32_t blinkinTime = 0;
	      		  uint32_t curTime = HAL_GetTick();

	      	      if (avg > 9500) {
	      	    	  if (blinkinTime) {
	      	    	      blinkin = (curTime - blinkinTime > 600);
	      	    	  }
	      	    	  else {
	      	    		  blinkin = false;
	      	    		blinkinTime = curTime;
	      	    	  }
	      	      }
	      	      else {
	      	    	blinkinTime = 0;
	      	    	  blinkin = false;
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

  	  int const virtualLedCount = LED_FRAME_SIZE + 8;
  	  int const yawLedIdx = (yaw / (2 * M_PI)) * virtualLedCount;
  	  //int const invYawLedIdx = (yawLedIdx + (virtualLedCount / 2)) % virtualLedCount;

  	  for (int i = 0; i < LED_FRAME_SIZE; ++i) {
  		  if ((abs(i - yawLedIdx) < 4) || (abs(i + virtualLedCount - yawLedIdx) < 4) || (abs(i - virtualLedCount - yawLedIdx) < 4)) {
  			  myLedz[i] = 0xff;
  		  }
  	  }


	    static uint32_t prevTime = 0;
	    uint32_t curTime = HAL_GetTick();
	    if (curTime - prevTime > 2) {
	        for (int i = 0; i < LED_FRAME_SIZE; ++i) {
	        	if (blinkin) {
	        		if (((curTime / 30) % 4) == (((i + curTime / 120) / 8) % 4))
	        		    DigiLed_setColor(i, 0xff, 0x15, 0xff);
	        		else
	        			DigiLed_setColor(i, 0x00, 0x00, 0x00);
	        	} else {
	        		DigiLed_setColor(i, myLedz[i], 0, myLedz[i]);
	        	}
	        	for (unsigned int j = 0; j < 5 && myLedz[i]; ++j) {
	        		--myLedz[i];
	        	}
	        }
		    do { prevTime += 2; } while (prevTime < curTime);
	    }
  	    DigiLed_update(0);

	    if (reg.status_reg.tda) {
	      // Read temperature data
	      memset(&imu_data_raw_temperature, 0x00, sizeof(imu_data_raw_temperature));
	      lsm6ds3tr_c_temperature_raw_get(&imu_dev_ctx, &imu_data_raw_temperature);
	      imu_temperature_degC = lsm6ds3tr_c_from_lsb_to_celsius(imu_data_raw_temperature);
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





    int counter2 = 0xff - (counter & 0xff);

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
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex : printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
