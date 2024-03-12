/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <cstdint>
#include <vector>

#include <lib/core/CHIPError.h>
#include <lib/core/TLVBackingStore.h>
#include <lib/core/TLVCommon.h>

namespace chip {
namespace TLV {

// Implementation of TLVWriter that writes to a std::vector, automatically
// resizing the vector as needed when writing.
// Users of TlvVectorWriter may call any public API of TLVWriter, except for the
// Init functions.
// This class is not thread-safe, it must be constructed, used, and destroyed on
// a single thread.
class TlvVectorWriter : public TLVWriter
{
public:
    // All data will be written to and read from the provided buffer, which must
    // outlive this object.
    TlvVectorWriter(std::vector<uint8_t> & buffer);
    TlvVectorWriter(const TlvVectorWriter &)             = delete;
    TlvVectorWriter & operator=(const TlvVectorWriter &) = delete;
    ~TlvVectorWriter();

private:
    class TlvVectorBuffer : public TLVBackingStore
    {
    public:
        TlvVectorBuffer(std::vector<uint8_t> & buffer);
        TlvVectorBuffer(const TlvVectorBuffer &)             = delete;
        TlvVectorBuffer & operator=(const TlvVectorBuffer &) = delete;
        ~TlvVectorBuffer() override;

        // TLVBackingStore implementation:
        CHIP_ERROR OnInit(TLVReader & reader, const uint8_t *& bufStart, uint32_t & bufLen) override
        {
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }
        CHIP_ERROR GetNextBuffer(TLVReader & reader, const uint8_t *& bufStart, uint32_t & bufLen) override
        {
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }
        CHIP_ERROR OnInit(TLVWriter & writer, uint8_t *& bufStart, uint32_t & bufLen) override;
        CHIP_ERROR GetNewBuffer(TLVWriter & writer, uint8_t *& bufStart, uint32_t & bufLen) override;
        CHIP_ERROR FinalizeBuffer(TLVWriter & writer, uint8_t * bufStart, uint32_t bufLen) override;

    private:
        void ResizeWriteBuffer(uint8_t *& bufStart, uint32_t & bufLen);

        // mWritingBuffer is the mutable buffer exposed via the TLVBackingStore
        // interface. When FinalizeBuffer is called the contents of mWritingBuffer
        // are appended to mFinalBuffer and mWritingBuffer is cleared. This allows
        // for reading all written data from a single, contiguous buffer
        // (mFinalBuffer).
        std::vector<uint8_t> mWritingBuffer;
        std::vector<uint8_t> & mFinalBuffer;
    };

    TlvVectorBuffer mVectorBuffer;
};

} // namespace TLV
} // namespace chip
