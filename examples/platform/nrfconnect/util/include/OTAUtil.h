/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#pragma once

#include <platform/nrfconnect/ExternalFlashManager.h>

#if CONFIG_CHIP_OTA_REQUESTOR
#include <platform/nrfconnect/OTAImageProcessorImpl.h>

namespace chip {
namespace DeviceLayer {
class OTAImageProcessorImpl;
} // namespace DeviceLayer
} // namespace chip

/**
 * Select recommended OTA image processor implementation.
 *
 * If the application uses QSPI external flash and enables API for controlling
 * power states of peripherals, select the implementation that automatically
 * powers off the external flash when no longer needed. Otherwise, select the
 * most basic implementation.
 */
chip::DeviceLayer::OTAImageProcessorImpl & GetOTAImageProcessor();

/** Initialize basic OTA requestor.
 *
 * Initialize all necessary components and start the OTA requestor state machine.
 * Assume that the device is not able to ask a user for consent before applying
 * an update so the confirmation must be done on the OTA provider side.
 */
void InitBasicOTARequestor();

/**
 * Check if the current image is the first boot the after OTA update and if so
 * confirm it in MCUBoot.
 *
 * @return CHIP_NO_ERROR if the image has been confirmed, or it is not the first
 * boot after the OTA update.
 * Other CHIP_ERROR codes if the image could not be confirmed.
 */
void OtaConfirmNewImage();

#endif // CONFIG_CHIP_OTA_REQUESTOR

/**
 * Get ExternalFlashManager static instance.
 *
 * Returned object can be used to control the QSPI external flash,
 * which can be introduced into sleep mode and woken up on demand.
 */
chip::DeviceLayer::ExternalFlashManager & GetFlashHandler();
