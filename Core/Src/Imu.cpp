#include "Imu.hpp"

#include "lsm6ds3tr-c_reg.h"

#include "spi.h"

#include <cstdio>
#include <cstring>

// LSM6DS3TR-C IMU

#define IMU_SENSOR_BUS hspi1
#define IMU_BOOT_TIME 15 // ms
#define IMU_TX_BUF_DIM 1000

static int16_t imu_data_raw_acceleration[3];
static int16_t imu_data_raw_angular_rate[3];
static int16_t imu_data_raw_temperature;
static uint8_t imu_whoamI;
static uint8_t imu_rst;

static stmdev_ctx_t imu_dev_ctx = {0};

static bool accel_updated;
static bool angular_rate_updated;
static bool temperature_updated;

static int32_t imu_platform_write(void *nHandle, uint8_t reg, const uint8_t *bufp, uint16_t len) {
    SPI_HandleTypeDef* handle = (SPI_HandleTypeDef*)nHandle;
    HAL_GPIO_WritePin(SPI1_NCS_GYR_GPIO_Port, SPI1_NCS_GYR_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(handle, &reg, 1, 1000);
    HAL_SPI_Transmit(handle, (uint8_t*) bufp, len, 1000);
    HAL_GPIO_WritePin(SPI1_NCS_GYR_GPIO_Port, SPI1_NCS_GYR_Pin, GPIO_PIN_SET);
    return 0;
}

static int32_t imu_platform_read(void *nHandle, uint8_t reg, uint8_t *bufp, uint16_t len) {
    SPI_HandleTypeDef* handle = (SPI_HandleTypeDef*)nHandle;
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

void imu_init() {
    // IMU

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
}

void imu_tick() {
    // Read output only if new value is available
    lsm6ds3tr_c_reg_t reg;
    lsm6ds3tr_c_status_reg_get(&imu_dev_ctx, &reg.status_reg);

    accel_updated = false;
    angular_rate_updated = false;
    temperature_updated = false;

    if (reg.status_reg.xlda) {
        accel_updated = true;

        memset(imu_data_raw_acceleration, 0x00, sizeof(imu_data_raw_acceleration));
        lsm6ds3tr_c_acceleration_raw_get(&imu_dev_ctx, imu_data_raw_acceleration);
    }

    if (reg.status_reg.gda) {
        angular_rate_updated = true;

        memset(imu_data_raw_angular_rate, 0x00, sizeof(imu_data_raw_angular_rate));
        lsm6ds3tr_c_angular_rate_raw_get(&imu_dev_ctx, imu_data_raw_angular_rate);
    }

    if (reg.status_reg.tda) {
        temperature_updated = true;

        memset(&imu_data_raw_temperature, 0x00, sizeof(imu_data_raw_temperature));
        lsm6ds3tr_c_temperature_raw_get(&imu_dev_ctx, &imu_data_raw_temperature);
    }
}

bool imu_accel_fetch(Eigen::Vector3f* result_accel_mg) {
    if (!accel_updated)
        return false;

    *result_accel_mg = {lsm6ds3tr_c_from_fs16g_to_mg(imu_data_raw_acceleration[0]),
        lsm6ds3tr_c_from_fs16g_to_mg(imu_data_raw_acceleration[1]),
        lsm6ds3tr_c_from_fs16g_to_mg(imu_data_raw_acceleration[2])};
    return true;
}

bool imu_angular_rate_fetch(Eigen::Vector3f* result_angular_rate_mdps) {
    if (!angular_rate_updated)
        return false;

    *result_angular_rate_mdps = {lsm6ds3tr_c_from_fs2000dps_to_mdps(imu_data_raw_angular_rate[0]),
        lsm6ds3tr_c_from_fs2000dps_to_mdps(imu_data_raw_angular_rate[1]),
        lsm6ds3tr_c_from_fs2000dps_to_mdps(imu_data_raw_angular_rate[2])};
    return true;
}

bool imu_temperature_fetch(float* result_temperature_degC) {
    if (!temperature_updated)
        return false;

    *result_temperature_degC = lsm6ds3tr_c_from_lsb_to_celsius(imu_data_raw_temperature);
    return true;
}
