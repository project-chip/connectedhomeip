/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/telink/OTAImageProcessorImpl.h>

namespace chip {
namespace DeviceLayer {
class OTAImageProcessorImpl;
} // namespace DeviceLayer
} // namespace chip

/** Initialize basic OTA requestor.
 *
 * Initialize all necessary components and start the OTA requestor state machine.
 * Assume that the device is not able to ask a user for consent before applying
 * an update so the confirmation must be done on the OTA provider side.
 */
void InitBasicOTARequestor();
