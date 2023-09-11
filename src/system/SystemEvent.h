/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file enumerates and defines the different types of events
 *      generated at the CHIP System Layer.
 */

#pragma once

// Include headers
#include <system/SystemConfig.h>

namespace chip {
namespace DeviceLayer {
struct ChipDeviceEvent;
} // namespace DeviceLayer
} // namespace chip

namespace chip {
namespace System {

/**
 *  @typedef EventType
 *  The basic type for all InetLayer events.
 *
 *  This is defined to a platform- or system-specific type.
 *
 */
typedef CHIP_SYSTEM_CONFIG_EVENT_TYPE EventType;

/**
 *  @typedef Event
 *  The basic object for all InetLayer events.
 *
 *  This is defined to a platform- or system-specific type.
 *
 */
typedef CHIP_SYSTEM_CONFIG_EVENT_OBJECT_TYPE Event;

} // namespace System
} // namespace chip
