/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
