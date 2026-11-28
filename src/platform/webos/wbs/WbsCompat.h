/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

/**
 *    @file
 *          Compatibility shims that let the wbs layer (wbs/, written against Matter v1.2 and
 *          kept unmodified) build against this SDK version. Force-included for the webOS
 *          platform sources by BUILD.gn.
 */

#pragma once

#ifdef __cplusplus

#include <cstddef>

// wbs sources include internal BLE headers (e.g. <ble/CHIPBleServiceData.h>) directly, which
// now require <ble/Ble.h> to be included first.
#include <ble/Ble.h>
#include <lib/support/CodeUtils.h>

namespace chip {

// Removed in favor of MATTER_ARRAY_SIZE(); still used by wbs/ChipDeviceScanner.cpp.
template <typename T, size_t N>
constexpr size_t ArraySize(T (&)[N])
{
    return N;
}

} // namespace chip

#endif // __cplusplus
