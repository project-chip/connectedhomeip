/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          General utility functions available on all platforms.
 */

#pragma once

#include <inet/Inet.h>

namespace chip {
namespace DeviceLayer {

extern const char * CharacterizeIPv6Address(const chip::Inet::IPAddress & ipAddr);
extern const char * CharacterizeIPv6Prefix(const chip::Inet::IPPrefix & inPrefix);
extern void RegisterDeviceLayerErrorFormatter();
extern bool FormatDeviceLayerError(char * buf, uint16_t bufSize, CHIP_ERROR err);

} // namespace DeviceLayer
} // namespace chip
