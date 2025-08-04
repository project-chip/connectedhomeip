/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/support/Fold.h>

#include <algorithm>
#include <initializer_list>

namespace chip {
namespace Dnssd {

/*
 * Matter DNS host settings
 */

// Matter spec expects hostname to be MAC or 802.15.4 Extended Address in hex.
// But in latest android nsdManager, it would set hostname with 40 bytes with prefix as android_,
// and there is no existing API to update the hostname, therefore we put temporary workaround with 40 bytes.
// Follow-up with ticket issue https://github.com/project-chip/connectedhomeip/issues/33474
inline constexpr size_t kHostNameMaxLength = 40;
//

/*
 * Matter DNS service subtypes
 */

inline constexpr size_t kSubTypeShortDiscriminatorMaxLength = 4;  // _S<dd>
inline constexpr size_t kSubTypeLongDiscriminatorMaxLength  = 6;  // _L<dddd>
inline constexpr size_t kSubTypeVendorIdMaxLength           = 7;  // _V<ddddd>
inline constexpr size_t kSubTypeDeviceTypeMaxLength         = 12; // _T<dddddddddd>
inline constexpr size_t kSubTypeCommissioningModeMaxLength  = 3;  // _CM
inline constexpr size_t kSubTypeCompressedFabricIdMaxLength = 18; // _I<16-hex-digits>

/*
 * Matter operational node service settings
 */

namespace Operational {

#define SUBTYPES (std::initializer_list<size_t>{ kSubTypeCompressedFabricIdMaxLength })

inline constexpr size_t kInstanceNameMaxLength = 33; // <NodeId>-<FabricId> in hex (16 + 1 + 16)
inline constexpr size_t kSubTypeMaxNumber      = SUBTYPES.size();
inline constexpr size_t kSubTypeMaxLength      = std::max(SUBTYPES);
inline constexpr size_t kSubTypeTotalLength    = chip::Sum(SUBTYPES);

#undef SUBTYPES

} // namespace Operational

/*
 * Matter commissionable/commissioner node service constants.
 */

namespace Commission {

#define SUBTYPES                                                                                                                   \
    (std::initializer_list<size_t>{ kSubTypeShortDiscriminatorMaxLength, kSubTypeLongDiscriminatorMaxLength,                       \
                                    kSubTypeVendorIdMaxLength, kSubTypeDeviceTypeMaxLength, kSubTypeCommissioningModeMaxLength })

inline constexpr size_t kInstanceNameMaxLength = 16; // 64-bit random number in hex
inline constexpr size_t kSubTypeMaxNumber      = SUBTYPES.size();
inline constexpr size_t kSubTypeMaxLength      = std::max(SUBTYPES);
inline constexpr size_t kSubTypeTotalLength    = chip::Sum(SUBTYPES);

#undef SUBTYPES

} // namespace Commission

/*
 * Constants for any Matter service.
 */

namespace Common {

inline constexpr size_t kInstanceNameMaxLength = std::max(Operational::kInstanceNameMaxLength, Commission::kInstanceNameMaxLength);
inline constexpr size_t kSubTypeMaxNumber      = std::max(Operational::kSubTypeMaxNumber, Commission::kSubTypeMaxNumber);
inline constexpr size_t kSubTypeMaxLength      = std::max(Operational::kSubTypeMaxLength, Commission::kSubTypeMaxLength);
inline constexpr size_t kSubTypeTotalLength    = std::max(Operational::kSubTypeTotalLength, Commission::kSubTypeTotalLength);

} // namespace Common

} // namespace Dnssd
} // namespace chip
