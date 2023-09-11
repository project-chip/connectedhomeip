/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>

#include <lib/core/CHIPError.h>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {
namespace Internal {
namespace AppPreference {

CHIP_ERROR CheckData(const char * key);
CHIP_ERROR GetData(const char * key, void * data, size_t dataSize, size_t * getDataSize, size_t offset);
CHIP_ERROR SaveData(const char * key, const void * data, size_t size);
CHIP_ERROR RemoveData(const char * key);

} // namespace AppPreference
} // namespace Internal
} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
