/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <cstdint>

#include <lib/core/CHIPError.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Pure range predicate extracted from WrapCBErrorCodeAsKOS so it can be unit-tested without
 * CoreBluetooth / Objective-C. The kOS range of ChipError only has 24 bits of value space
 * (ChipError::kValueLength == 24, so the representable interval is [0, 0xFFFFFF]). A CoreBluetooth
 * NSError.code outside that interval would be silently masked by ChipError's value field
 * (e.g. 0x1000000 masks to 0, -1 masks to 0xFFFFFF), corrupting triage data and possibly
 * aliasing onto unrelated kOS codes. This predicate is the guard that decides whether the cast
 * is safe; the caller falls back to a caller-provided CHIP_ERROR (typically a BLE_ERROR_GATT_*
 * sentinel for GATT-operation metrics, but may be a generic code like CHIP_ERROR_INCORRECT_STATE
 * for non-GATT call sites) when it returns false.
 *
 * Takes int64_t so the full NSInteger domain (including negatives and values above the 24-bit
 * max) round-trips losslessly into this check on both 32- and 64-bit targets.
 */
constexpr bool CBErrorCodeFitsInKOSValueRange(int64_t code)
{
    return code >= 0 && code <= 0xFFFFFF;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
