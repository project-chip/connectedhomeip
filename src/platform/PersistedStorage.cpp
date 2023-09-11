/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Provides implementations for the CHIP persistent storage
 *          APIs.  This implementation is common across all platforms.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/ConfigurationManager.h>

namespace chip {
namespace Platform {
namespace PersistedStorage {

using namespace ::chip::DeviceLayer;

CHIP_ERROR Read(Key key, uint32_t & value)
{
    return ConfigurationMgr().ReadPersistedStorageValue(key, value);
}

CHIP_ERROR Write(Key key, uint32_t value)
{
    return ConfigurationMgr().WritePersistedStorageValue(key, value);
}

} // namespace PersistedStorage
} // namespace Platform
} // namespace chip
