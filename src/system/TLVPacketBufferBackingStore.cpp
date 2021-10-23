/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file contains an implementation of TLVBackingStore using PacketBuffers.
 */

#include <system/TLVPacketBufferBackingStore.h>

#include <lib/support/SafeInt.h>

namespace chip {
namespace System {

CHIP_ERROR TLVPacketBufferBackingStore::OnInit(chip::TLV::TLVReader & reader, const uint8_t *& bufStart, uint32_t & bufLen)
{
    bufStart = mHeadBuffer->Start();
    bufLen   = mHeadBuffer->DataLength();
    return CHIP_NO_ERROR;
}

CHIP_ERROR TLVPacketBufferBackingStore::GetNextBuffer(chip::TLV::TLVReader & reader, const uint8_t *& bufStart, uint32_t & bufLen)
{
    if (mUseChainedBuffers)
    {
        mCurrentBuffer.Advance();
    }
    else
    {
        mCurrentBuffer = nullptr;
    }

    if (mCurrentBuffer.IsNull())
    {
        bufStart = nullptr;
        bufLen   = 0;
    }
    else
    {
        bufStart = mCurrentBuffer->Start();
        bufLen   = mCurrentBuffer->DataLength();
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TLVPacketBufferBackingStore::OnInit(chip::TLV::TLVWriter & writer, uint8_t *& bufStart, uint32_t & bufLen)
{
    bufStart = mHeadBuffer->Start() + mHeadBuffer->DataLength();
    bufLen   = mHeadBuffer->AvailableDataLength();
    return CHIP_NO_ERROR;
}

CHIP_ERROR TLVPacketBufferBackingStore::FinalizeBuffer(chip::TLV::TLVWriter & writer, uint8_t * bufStart, uint32_t dataLen)
{
    uint8_t * endPtr = bufStart + dataLen;

    intptr_t length = endPtr - mCurrentBuffer->Start();
    if (!CanCastTo<uint16_t>(length))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    mCurrentBuffer->SetDataLength(static_cast<uint16_t>(length));

    return CHIP_NO_ERROR;
}

CHIP_ERROR TLVPacketBufferBackingStore::GetNewBuffer(chip::TLV::TLVWriter & writer, uint8_t *& bufStart, uint32_t & bufLen)
{
    if (!mUseChainedBuffers)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    mCurrentBuffer.Advance();
    if (mCurrentBuffer.IsNull())
    {
        mCurrentBuffer = PacketBufferHandle::New(System::PacketBuffer::kMaxSizeWithoutReserve, 0);
        if (mCurrentBuffer.IsNull())
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        mHeadBuffer->AddToEnd(mCurrentBuffer.Retain());
    }

    if (mCurrentBuffer.IsNull())
    {
        bufStart = nullptr;
        bufLen   = 0;
    }
    else
    {
        bufStart = mCurrentBuffer->Start();
        bufLen   = mCurrentBuffer->MaxDataLength();
    }

    return CHIP_NO_ERROR;
}

} // namespace System
} // namespace chip
