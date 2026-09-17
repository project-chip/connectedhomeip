/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "AXP192.h"

#if CONFIG_DEVICE_TYPE_M5STACK_CORE2

#include <esp_log.h>
#include <freertos/FreeRTOS.h>

namespace AXP192 {

namespace {

constexpr const char TAG[] = "AXP192";

// 7-bit I2C slave address.
constexpr uint8_t kAddress = 0x34;

i2c_port_t gPort = I2C_NUM_0;

// Registers used here. Addresses and bit meanings match M5Stack's own Core2
// AXP192 driver (https://github.com/m5stack/M5Core2/blob/master/src/AXP192.cpp).
constexpr uint8_t kRegPowerOutputCtrl = 0x12; // bit1: DCDC3 enable, bit2: LDO2 enable, bit3: LDO3 enable
constexpr uint8_t kRegDCDC3Voltage    = 0x27; // 700mV + N*25mV, 7 bits
constexpr uint8_t kRegLDO23Voltage    = 0x28; // upper nibble: LDO2, lower nibble: LDO3; 1800mV + N*100mV each
constexpr uint8_t kRegGPIO34Ctrl      = 0x95; // GPIO4 function select (bits [2:0] of the GPIO4 nibble)
constexpr uint8_t kRegGPIO34Signal    = 0x96; // bit1: GPIO4 output level

esp_err_t ReadReg(uint8_t reg, uint8_t & value)
{
    return i2c_master_write_read_device(gPort, kAddress, &reg, 1, &value, 1, pdMS_TO_TICKS(100));
}

esp_err_t WriteReg(uint8_t reg, uint8_t value)
{
    uint8_t payload[2] = { reg, value };
    return i2c_master_write_to_device(gPort, kAddress, payload, sizeof(payload), pdMS_TO_TICKS(100));
}

// Read-modify-write helper: sets or clears `mask`'s bits, leaving the rest of the register untouched.
esp_err_t UpdateBits(uint8_t reg, uint8_t mask, bool set)
{
    uint8_t value;
    esp_err_t err = ReadReg(reg, value);
    if (err != ESP_OK)
    {
        return err;
    }
    value = set ? static_cast<uint8_t>(value | mask) : static_cast<uint8_t>(value & ~mask);
    return WriteReg(reg, value);
}

// Converts a millivolt target into this chip's N-steps-from-a-floor encoding,
// clamped to the representable range.
uint8_t VoltageToSteps(uint16_t millivolts, uint16_t floorMillivolts, uint16_t stepMillivolts, uint8_t maxSteps)
{
    if (millivolts <= floorMillivolts)
    {
        return 0;
    }
    uint32_t steps = (millivolts - floorMillivolts) / stepMillivolts;
    return static_cast<uint8_t>(steps > maxSteps ? maxSteps : steps);
}

} // namespace

esp_err_t Init(i2c_port_t port, gpio_num_t sda, gpio_num_t scl)
{
    gPort = port;

    i2c_config_t conf     = {};
    conf.mode             = I2C_MODE_MASTER;
    conf.sda_io_num       = sda;
    conf.scl_io_num       = scl;
    conf.sda_pullup_en    = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en    = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 400000;

    esp_err_t err = i2c_param_config(gPort, &conf);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "i2c_param_config failed: %s", esp_err_to_name(err));
        return err;
    }

    err = i2c_driver_install(gPort, conf.mode, 0, 0, 0);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "i2c_driver_install failed: %s", esp_err_to_name(err));
        return err;
    }
    return ESP_OK;
}

esp_err_t SetDCDC3Voltage(uint16_t millivolts)
{
    // (millivolts - 700) / 25, kept in the register's low 7 bits.
    uint8_t vdata = VoltageToSteps(millivolts, 700, 25, 0x7F);
    uint8_t reg;
    esp_err_t err = ReadReg(kRegDCDC3Voltage, reg);
    if (err != ESP_OK)
    {
        return err;
    }
    return WriteReg(kRegDCDC3Voltage, static_cast<uint8_t>((reg & 0x80) | vdata));
}

esp_err_t SetDCDC3Enable(bool enable)
{
    return UpdateBits(kRegPowerOutputCtrl, 1u << 1, enable);
}

esp_err_t SetLDOVoltage(uint8_t ldoNumber, uint16_t millivolts)
{
    // (millivolts - 1800) / 100, kept in LDO2's upper nibble or LDO3's lower nibble.
    uint8_t vdata = VoltageToSteps(millivolts, 1800, 100, 0x0F);
    uint8_t reg;
    esp_err_t err = ReadReg(kRegLDO23Voltage, reg);
    if (err != ESP_OK)
    {
        return err;
    }
    if (ldoNumber == 2)
    {
        return WriteReg(kRegLDO23Voltage, static_cast<uint8_t>((reg & 0x0F) | (vdata << 4)));
    }
    if (ldoNumber == 3)
    {
        return WriteReg(kRegLDO23Voltage, static_cast<uint8_t>((reg & 0xF0) | vdata));
    }
    return ESP_ERR_INVALID_ARG;
}

esp_err_t SetLDOEnable(uint8_t ldoNumber, bool enable)
{
    if (ldoNumber != 2 && ldoNumber != 3)
    {
        return ESP_ERR_INVALID_ARG;
    }
    return UpdateBits(kRegPowerOutputCtrl, static_cast<uint8_t>(1u << ldoNumber), enable);
}

esp_err_t SetGPIO4PushPullOutput()
{
    esp_err_t err = UpdateBits(kRegGPIO34Ctrl, 0x72, false);
    if (err != ESP_OK)
    {
        return err;
    }
    return UpdateBits(kRegGPIO34Ctrl, 0x84, true);
}

esp_err_t SetGPIO4Level(bool high)
{
    return UpdateBits(kRegGPIO34Signal, 1u << 1, high);
}

} // namespace AXP192

#endif // CONFIG_DEVICE_TYPE_M5STACK_CORE2
