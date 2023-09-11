/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

#include <credentials/FabricTable.h>
#include <credentials/GroupDataProvider.h>
#include <lib/core/CHIPError.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/Span.h>

#include "CASEDestinationId.h"

namespace chip {

using namespace chip::Crypto;

CHIP_ERROR GenerateCaseDestinationId(const ByteSpan & ipk, const ByteSpan & initiatorRandom, const ByteSpan & rootPubKey,
                                     FabricId fabricId, NodeId nodeId, MutableByteSpan & outDestinationId)
{
    VerifyOrReturnError(ipk.size() == kIPKSize, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(initiatorRandom.size() == kSigmaParamRandomNumberSize, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(rootPubKey.size() == kP256_PublicKey_Length, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(outDestinationId.size() >= kSHA256_Hash_Length, CHIP_ERROR_INVALID_ARGUMENT);

    constexpr size_t kDestinationMessageLen =
        kSigmaParamRandomNumberSize + kP256_PublicKey_Length + sizeof(FabricId) + sizeof(NodeId);
    uint8_t destinationMessage[kDestinationMessageLen];

    Encoding::LittleEndian::BufferWriter bbuf(destinationMessage, sizeof(destinationMessage));
    bbuf.Put(initiatorRandom.data(), initiatorRandom.size());
    bbuf.Put(rootPubKey.data(), rootPubKey.size());
    bbuf.Put64(fabricId);
    bbuf.Put64(nodeId);

    size_t written = 0;
    VerifyOrReturnError(bbuf.Fit(written), CHIP_ERROR_BUFFER_TOO_SMALL);

    HMAC_sha hmac;
    CHIP_ERROR err =
        hmac.HMAC_SHA256(ipk.data(), ipk.size(), bbuf.Buffer(), written, outDestinationId.data(), outDestinationId.size());

    if (err == CHIP_NO_ERROR)
    {
        outDestinationId.reduce_size(kSHA256_Hash_Length);
    }

    return err;
}

} // namespace chip
