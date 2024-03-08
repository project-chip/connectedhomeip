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

TlvVectorWriter::TlvVectorWriter(std::vector<uint8_t> * buffer) : buffer_(buffer)
{
    Init(buffer_);
}

TlvVectorWriter::~TlvVectorWriter() = default;

TlvVectorWriter::TlvVectorBuffer::TlvVectorBuffer(std::vector<uint8_t> * buffer) : final_buffer_(*buffer) {}

TlvVectorWriter::TlvVectorBuffer::~TlvVectorBuffer() {}

CHIP_ERROR TlvVectorWriter::TlvVectorBuffer::OnInit(TLVWriter & /*writer*/, uint8_t *& bufStart, uint32_t & bufLen)
{
    VerifyOrReturnError(final_buffer_.empty(), CHIP_ERROR_INCORRECT_STATE);

    ResizeWriteBuffer(bufStart, bufLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR TlvVectorWriter::TlvVectorBuffer::GetNewBuffer(TLVWriter & /*writer*/, uint8_t *& bufStart, uint32_t & bufLen)
{
    VerifyOrReturnError(!final_buffer_.empty(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(writing_buffer_.data() == bufStart, CHIP_ERROR_INCORRECT_STATE);

    ResizeWriteBuffer(bufStart, bufLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR TlvVectorWriter::TlvVectorBuffer::FinalizeBuffer(TLVWriter & /*writer*/, uint8_t * bufStart, uint32_t bufLen)
{
    VerifyOrReturnError(writing_buffer_.data() == bufStart, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(bufLen <= writing_buffer_.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    writing_buffer_.resize(bufLen);

    final_buffer_.insert(final_buffer_.end(), writing_buffer_.begin(), writing_buffer_.end());
    writing_buffer_.resize(0);

    final_buffer_.shrink_to_fit();

    return CHIP_NO_ERROR;
}

void TlvVectorWriter::TlvVectorBuffer::ResizeWriteBuffer(uint8_t *& bufStart, uint32_t & bufLen)
{
    VerifyOrReturn(writing_buffer_.empty());

    writing_buffer_.resize(kIpv6MtuSize);
    bufStart = writing_buffer_.data();

    auto size = writing_buffer_.size();
    VerifyOrReturn(size <= std::numeric_limits<uint32_t>::max());
    bufLen = static_cast<uint32_t>(size);
}

} // namespace TLV
} // namespace chip
