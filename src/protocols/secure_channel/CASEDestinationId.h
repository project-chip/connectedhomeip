/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <stdint.h>

#include <credentials/FabricTable.h>
#include <credentials/GroupDataProvider.h>
#include <crypto/CHIPCryptoPAL.h>

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/Span.h>

namespace chip {

inline constexpr uint16_t kSigmaParamRandomNumberSize = 32;
inline constexpr uint16_t kIPKSize                    = Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES;

CHIP_ERROR GenerateCaseDestinationId(const ByteSpan & ipk, const ByteSpan & initiatorRandom, const ByteSpan & rootPubKey,
                                     FabricId fabricId, NodeId nodeId, MutableByteSpan & outDestinationId);

} // namespace chip
