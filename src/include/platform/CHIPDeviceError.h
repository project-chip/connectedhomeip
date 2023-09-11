/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lib/core/CHIPError.h>

#define CHIP_DEVICE_ERROR(e) CHIP_SDK_ERROR(::chip::ChipError::SdkPart::kDevice, (e))

/**
 *  @def CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND
 *
 *  @brief
 *    The requested configuration value was not found.
 *
 */
#define CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND CHIP_DEVICE_ERROR(0x01)

/**
 *  @def CHIP_DEVICE_ERROR_NOT_SERVICE_PROVISIONED
 *
 *  @brief
 *    The device has not been service provisioned.
 *
 */
#define CHIP_DEVICE_ERROR_NOT_SERVICE_PROVISIONED CHIP_DEVICE_ERROR(0x02)

/**
 *  @def CHIP_DEVICE_ERROR_SOFTWARE_UPDATE_ABORTED
 *
 *  @brief
 *    The software update was aborted by application
 *
 */
#define CHIP_DEVICE_ERROR_SOFTWARE_UPDATE_ABORTED CHIP_DEVICE_ERROR(0x03)

/**
 *  @def CHIP_DEVICE_ERROR_SOFTWARE_UPDATE_IGNORED
 *
 *  @brief
 *    The software update was ignored by application.
 *
 */
#define CHIP_DEVICE_ERROR_SOFTWARE_UPDATE_IGNORED CHIP_DEVICE_ERROR(0x04)
