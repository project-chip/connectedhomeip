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

#include <stdint.h>

#include <credentials/FabricTable.h>
#include <credentials/GroupDataProvider.h>
#include <lib/core/CHIPError.h>
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
