/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
