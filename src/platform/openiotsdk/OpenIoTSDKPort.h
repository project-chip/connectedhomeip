/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Utilities for interacting with the the Open IoT SDK platform port
 */

#pragma once

#include <iotsdk/BlockDevice.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

iotsdk::storage::BlockDevice * GetBlockDevice(void);

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
