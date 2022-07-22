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

constexpr size_t kHostNameMaxLength = 16; // MAC or 802.15.4 Extended Address in hex

/*
 * Matter DNS service subtypes
 */

constexpr size_t kSubTypeShortDiscriminatorMaxLength      = 4;  // _S<dd>
constexpr size_t kSubTypeLongDiscriminatorMaxLength       = 6;  // _L<dddd>
constexpr size_t kSubTypeVendorIdMaxLength                = 7;  // _V<ddddd>
constexpr size_t kSubTypeDeviceTypeMaxLength              = 12; // _T<dddddddddd>
constexpr size_t kSubTypeCommissioningModeMaxLength       = 3;  // _C<d>
constexpr size_t kSubTypeAdditionalCommissioningMaxLength = 3;  // _A<d>
constexpr size_t kSubTypeCompressedFabricIdMaxLength      = 18; // _I<16-hex-digits>

/*
 * Matter operational node service settings
 */

namespace Operational {

#define SUBTYPES (std::initializer_list<size_t>{ kSubTypeCompressedFabricIdMaxLength })

constexpr size_t kInstanceNameMaxLength = 33; // <NodeId>-<FabricId> in hex (16 + 1 + 16)
constexpr size_t kSubTypeMaxNumber      = SUBTYPES.size();
constexpr size_t kSubTypeMaxLength      = std::max(SUBTYPES);
constexpr size_t kSubTypeTotalLength    = chip::Sum(SUBTYPES);

#undef SUBTYPES

} // namespace Operational

/*
 * Matter commissionable/commissioner node service constants.
 */

namespace Commission {

#define SUBTYPES                                                                                                                   \
    (std::initializer_list<size_t>{ kSubTypeShortDiscriminatorMaxLength, kSubTypeLongDiscriminatorMaxLength,                       \
                                    kSubTypeVendorIdMaxLength, kSubTypeDeviceTypeMaxLength, kSubTypeCommissioningModeMaxLength,    \
                                    kSubTypeAdditionalCommissioningMaxLength })

constexpr size_t kInstanceNameMaxLength = 16; // 64-bit random number in hex
constexpr size_t kSubTypeMaxNumber      = SUBTYPES.size();
constexpr size_t kSubTypeMaxLength      = std::max(SUBTYPES);
constexpr size_t kSubTypeTotalLength    = chip::Sum(SUBTYPES);

#undef SUBTYPES

} // namespace Commission

/*
 * Constants for any Matter service.
 */

namespace Common {

constexpr size_t kInstanceNameMaxLength = std::max(Operational::kInstanceNameMaxLength, Commission::kInstanceNameMaxLength);
constexpr size_t kSubTypeMaxNumber      = std::max(Operational::kSubTypeMaxNumber, Commission::kSubTypeMaxNumber);
constexpr size_t kSubTypeMaxLength      = std::max(Operational::kSubTypeMaxLength, Commission::kSubTypeMaxLength);
constexpr size_t kSubTypeTotalLength    = std::max(Operational::kSubTypeTotalLength, Commission::kSubTypeTotalLength);

} // namespace Common

} // namespace Dnssd
} // namespace chip
