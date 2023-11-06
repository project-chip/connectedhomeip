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

#pragma once

#include <lib/core/TLV.h>
#include <system/SystemPacketBuffer.h>

#include <utility>

namespace chip {
namespace System {

/**
 * An implementation of TLVBackingStore using PacketBuffers.
 */
class TLVPacketBufferBackingStore : public chip::TLV::TLVBackingStore
{
public:
    TLVPacketBufferBackingStore() : mHeadBuffer(nullptr), mCurrentBuffer(nullptr), mUseChainedBuffers(false) {}
    TLVPacketBufferBackingStore(chip::System::PacketBufferHandle && buffer, bool useChainedBuffers = false)
    {
        Init(std::move(buffer), useChainedBuffers);
    }
    ~TLVPacketBufferBackingStore() override {}

    /**
     * Take ownership of a backing packet buffer.
     *
     * @param[in]    buffer  A handle to a packet buffer, to be used as backing store for a TLV class.
     * @param[in]    useChainedBuffers
     *                       If true, advance to the next buffer in the chain once all data or space
     *                       in the current buffer has been consumed; a write will allocate new
     *                       packet buffers if necessary.
     *
     * @note This must take place before initializing a TLV class with this backing store.
     */
    void Init(chip::System::PacketBufferHandle && buffer, bool useChainedBuffers = false)
    {
        mHeadBuffer        = std::move(buffer);
        mCurrentBuffer     = mHeadBuffer.Retain();
        mUseChainedBuffers = useChainedBuffers;
    }
    void Adopt(chip::System::PacketBufferHandle && buffer) { Init(std::move(buffer), mUseChainedBuffers); }

    /**
     * Release ownership of the backing packet buffer.
     *
     * @note TLV operations must no longer be performed on this store.
     */
    CHECK_RETURN_VALUE chip::System::PacketBufferHandle Release()
    {
        mCurrentBuffer = nullptr;
        return std::move(mHeadBuffer);
    }

    // TLVBackingStore overrides:
    CHIP_ERROR OnInit(chip::TLV::TLVReader & reader, const uint8_t *& bufStart, uint32_t & bufLen) override;
    CHIP_ERROR GetNextBuffer(chip::TLV::TLVReader & reader, const uint8_t *& bufStart, uint32_t & bufLen) override;
    CHIP_ERROR OnInit(chip::TLV::TLVWriter & writer, uint8_t *& bufStart, uint32_t & bufLen) override;
    CHIP_ERROR GetNewBuffer(chip::TLV::TLVWriter & writer, uint8_t *& bufStart, uint32_t & bufLen) override;
    CHIP_ERROR FinalizeBuffer(chip::TLV::TLVWriter & writer, uint8_t * bufStart, uint32_t bufLen) override;

protected:
    chip::System::PacketBufferHandle mHeadBuffer;
    chip::System::PacketBufferHandle mCurrentBuffer;
    bool mUseChainedBuffers;
};

class DLL_EXPORT PacketBufferTLVReader : public TLV::ContiguousBufferTLVReader
{
public:
    /**
     * Initializes a TLVReader object to read from a PacketBuffer.
     *
     * @param[in]    buffer  A handle to PacketBuffer, to be used as backing
     *                       store for a TLV class.  If the buffer is chained,
     *                       only the head of the chain will be used.
     */
    void Init(chip::System::PacketBufferHandle && buffer)
    {
        mBuffer = std::move(buffer);
        TLV::ContiguousBufferTLVReader::Init(mBuffer->Start(), mBuffer->DataLength());
    }

private:
    PacketBufferHandle mBuffer;
};

class DLL_EXPORT PacketBufferTLVWriter : public chip::TLV::TLVWriter
{
public:
    /**
     * Initializes a TLVWriter object to write to a PacketBuffer.
     *
     * @param[in]    buffer  A handle to PacketBuffer, to be used as backing store for a TLV class.
     * @param[in]    useChainedBuffers
     *                       If true, advance to the next buffer in the chain once all space
     *                       in the current buffer has been consumed. Once all existing buffers
     *                       have been used, new PacketBuffers will be allocated as necessary.
     */
    void Init(chip::System::PacketBufferHandle && buffer, bool useChainedBuffers = false)
    {
        mBackingStore.Init(std::move(buffer), useChainedBuffers);
        chip::TLV::TLVWriter::Init(mBackingStore);
    }
    /**
     * Finish the writing of a TLV encoding and release ownership of the underlying PacketBuffer.
     *
     * @param[in,out] outBuffer     The backing packet buffer.
     *
     * @retval #CHIP_NO_ERROR       If the encoding was finalized successfully.
     * @retval #CHIP_ERROR_TLV_CONTAINER_OPEN
     *                              If a container writer has been opened on the current writer and not
     *                              yet closed.
     * @retval #CHIP_ERROR_INVALID_ARGUMENT
     *                              If the apparent data length does not fit in uint16_t.
     *
     * @note No further TLV operations may be performed, unless or until this PacketBufferTLVWriter is re-initialized.
     */
    CHIP_ERROR Finalize(chip::System::PacketBufferHandle * outBuffer)
    {
        CHIP_ERROR err = Finalize();
        *outBuffer     = mBackingStore.Release();
        return err;
    }
    /**
     * Free the underlying PacketBuffer.
     *
     * @note No further TLV operations may be performed, unless or until this PacketBufferTLVWriter is re-initialized.
     */
    void Reset() { static_cast<void>(mBackingStore.Release()); }

private:
    CHIP_ERROR Finalize() { return chip::TLV::TLVWriter::Finalize(); }
    TLVPacketBufferBackingStore mBackingStore;
};

} // namespace System
} // namespace chip
