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

#include <lib/core/TLVVectorWriter.h>

#include <cstdint>
#include <vector>

#include <lib/core/CHIPError.h>
#include <lib/core/TLVCommon.h>

namespace chip {
namespace TLV {

namespace {

constexpr uint32_t kIpv6MtuSize = 1280;

} // namespace

TlvVectorWriter::TlvVectorWriter(std::vector<uint8_t> & buffer) : mVectorBuffer(buffer)
{
    Init(mVectorBuffer);
}

TlvVectorWriter::~TlvVectorWriter() = default;

TlvVectorWriter::TlvVectorBuffer::TlvVectorBuffer(std::vector<uint8_t> & buffer) : mFinalBuffer(buffer) {}

TlvVectorWriter::TlvVectorBuffer::~TlvVectorBuffer() {}

CHIP_ERROR TlvVectorWriter::TlvVectorBuffer::OnInit(TLVWriter & /*writer*/, uint8_t *& bufStart, uint32_t & bufLen)
{
    VerifyOrReturnError(mFinalBuffer.empty(), CHIP_ERROR_INCORRECT_STATE);

    ResizeWriteBuffer(bufStart, bufLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR TlvVectorWriter::TlvVectorBuffer::GetNewBuffer(TLVWriter & /*writer*/, uint8_t *& bufStart, uint32_t & bufLen)
{
    VerifyOrReturnError(!mFinalBuffer.empty(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mWritingBuffer.data() == bufStart, CHIP_ERROR_INCORRECT_STATE);

    ResizeWriteBuffer(bufStart, bufLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR TlvVectorWriter::TlvVectorBuffer::FinalizeBuffer(TLVWriter & /*writer*/, uint8_t * bufStart, uint32_t bufLen)
{
    VerifyOrReturnError(mWritingBuffer.data() == bufStart, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(bufLen <= mWritingBuffer.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    mWritingBuffer.resize(bufLen);

    mFinalBuffer.insert(mFinalBuffer.end(), mWritingBuffer.begin(), mWritingBuffer.end());
    mWritingBuffer.resize(0);

    mFinalBuffer.shrink_to_fit();

    return CHIP_NO_ERROR;
}

void TlvVectorWriter::TlvVectorBuffer::ResizeWriteBuffer(uint8_t *& bufStart, uint32_t & bufLen)
{
    VerifyOrReturn(mWritingBuffer.empty());

    mWritingBuffer.resize(kIpv6MtuSize);
    bufStart = mWritingBuffer.data();

    auto size = mWritingBuffer.size();
    VerifyOrReturn(size <= std::numeric_limits<uint32_t>::max());
    bufLen = static_cast<uint32_t>(size);
}

} // namespace TLV
} // namespace chip
