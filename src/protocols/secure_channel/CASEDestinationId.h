/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

constexpr uint16_t kSigmaParamRandomNumberSize = 32;
constexpr uint16_t kIPKSize                    = Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES;

CHIP_ERROR GenerateCaseDestinationId(const ByteSpan & ipk, const ByteSpan & initiatorRandom, const ByteSpan & rootPubKey,
                                     FabricId fabricId, NodeId nodeId, MutableByteSpan & outDestinationId);

} // namespace chip
