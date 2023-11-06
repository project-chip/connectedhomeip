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

#pragma once

#include <app/StatusResponse.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>

namespace chip {
namespace app {

static CHIP_ERROR InitWriterWithSpaceReserved(System::PacketBufferTLVWriter & aWriter, uint32_t aReserveSpace)
{
    System::PacketBufferHandle msgBuf = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    VerifyOrReturnError(!msgBuf.IsNull(), CHIP_ERROR_NO_MEMORY);
    uint16_t reservedSize = 0;

    if (msgBuf->AvailableDataLength() > kMaxSecureSduLengthBytes)
    {
        reservedSize = static_cast<uint16_t>(msgBuf->AvailableDataLength() - kMaxSecureSduLengthBytes);
    }

    reservedSize = static_cast<uint16_t>(reservedSize + Crypto::CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES + aReserveSpace);
    aWriter.Init(std::move(msgBuf));
    ReturnErrorOnFailure(aWriter.ReserveBuffer(reservedSize));
    return CHIP_NO_ERROR;
};
} // namespace app
} // namespace chip
