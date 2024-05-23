/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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

#include "BdxUri.h"

#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CodeUtils.h>

#include <cstring>

namespace chip {
namespace bdx {

constexpr size_t kSchemeLen    = sizeof(kScheme) - 1;
constexpr size_t kHostPos      = kSchemeLen;
constexpr size_t kHostLen      = sizeof(NodeId) * 2;
constexpr size_t kHostEnd      = kHostPos + kHostLen;
constexpr size_t kSeparatorLen = 1;
constexpr size_t kMinUriLen    = kHostEnd + kSeparatorLen + 1 /* file-designator */;

static_assert(sizeof(NodeId) == sizeof(uint64_t), "The code below assumes NodeId is uint64_t");

CHIP_ERROR ParseURI(CharSpan uri, NodeId & nodeId, CharSpan & fileDesignator)
{
    VerifyOrReturnError(uri.size() >= kMinUriLen, CHIP_ERROR_INVALID_STRING_LENGTH);
    VerifyOrReturnError(memcmp(uri.data(), kScheme, kSchemeLen) == 0, CHIP_ERROR_INVALID_SCHEME_PREFIX);

    uint8_t nodeIdBytes[sizeof(NodeId)];
    VerifyOrReturnError(Encoding::HexToBytes(uri.data() + kHostPos, kHostLen, nodeIdBytes, sizeof(nodeIdBytes)) ==
                            sizeof(nodeIdBytes),
                        CHIP_ERROR_INVALID_DESTINATION_NODE_ID);

    nodeId = Encoding::BigEndian::Get64(nodeIdBytes);
    VerifyOrReturnError(IsOperationalNodeId(nodeId), CHIP_ERROR_INVALID_DESTINATION_NODE_ID);
    VerifyOrReturnError(uri.data()[kHostEnd] == '/', CHIP_ERROR_MISSING_URI_SEPARATOR);

    fileDesignator = uri.SubSpan(kHostEnd + kSeparatorLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR MakeURI(NodeId nodeId, CharSpan fileDesignator, MutableCharSpan & uri)
{
    VerifyOrReturnError(fileDesignator.size() > 0, CHIP_ERROR_INVALID_STRING_LENGTH);

    uint8_t nodeIdBytes[sizeof(NodeId)];
    Encoding::BigEndian::Put64(nodeIdBytes, nodeId);

    char nodeIdHex[sizeof(NodeId) * 2];
    ReturnErrorOnFailure(Encoding::BytesToUppercaseHexBuffer(nodeIdBytes, sizeof(nodeIdBytes), nodeIdHex, sizeof(nodeIdHex)));

    char * buffer     = uri.data();
    size_t bufferSize = uri.size();
    memset(buffer, 0, bufferSize);

    // Reduce the buffer writer size by one to reserve the last byte for the null-terminator
    Encoding::BufferWriter writer(Uint8::from_char(buffer), bufferSize - 1);
    writer.Put(kScheme, kSchemeLen);
    writer.Put(nodeIdHex, sizeof(nodeIdHex));
    writer.Put("/");
    writer.Put(fileDesignator.data(), fileDesignator.size());

    VerifyOrReturnError(writer.Fit(), CHIP_ERROR_BUFFER_TOO_SMALL);
    uri.reduce_size(writer.WritePos());

    return CHIP_NO_ERROR;
}

} // namespace bdx
} // namespace chip
