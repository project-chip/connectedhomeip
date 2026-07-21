/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#include "MultiOTAImageHeader.h"

#include <algorithm>

#include <lib/core/CHIPError.h>
#include <lib/support/BufferReader.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedMemoryBuffer.h>
#include <lib/support/Span.h>

#include <string.h>

namespace chip {

void MultiOTAImageHeaderParser::Init()
{
    mState        = State::kInitialized;
    mBufferOffset = 0;
    mNumImages    = 0;
    mHeaderParsed = false;
    mBuffer.Alloc(kFixedHeaderSize);
}

void MultiOTAImageHeaderParser::Clear()
{
    mState        = State::kNotInitialized;
    mBufferOffset = 0;
    mNumImages    = 0;
    mHeaderParsed = false;
    mBuffer.Free();
}

CHIP_ERROR MultiOTAImageHeaderParser::AccumulateAndDecode(ByteSpan & buffer, MultiOTAImageHeader & header)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    if (mState != State::kNotInitialized && mBuffer.Get() == nullptr)
    {
        Clear();
        return CHIP_ERROR_NO_MEMORY;
    }

    if (mState == State::kInitialized)
    {
        Append(buffer, kFixedHeaderSize - mBufferOffset);
        error = DecodeMultiOTAImageHeader();
    }

    if (mState == State::kSubImages)
    {
        Append(buffer, mNumImages * kSubImageHeaderSize - mBufferOffset);
        error = DecodeSubImageHeaders(header);
    }

    if (error != CHIP_NO_ERROR && error != CHIP_ERROR_BUFFER_TOO_SMALL)
    {
        Clear();
    }

    if (error == CHIP_NO_ERROR)
    {
        mHeaderParsed = true;
    }
    return error;
}

void MultiOTAImageHeaderParser::Append(ByteSpan & buffer, uint32_t numBytes)
{
    numBytes = std::min(numBytes, static_cast<uint32_t>(buffer.size()));
    memcpy(&mBuffer[mBufferOffset], buffer.data(), numBytes);
    mBufferOffset += numBytes;
    buffer = buffer.SubSpan(numBytes);
}

CHIP_ERROR MultiOTAImageHeaderParser::DecodeMultiOTAImageHeader()
{
    VerifyOrReturnError(mBufferOffset >= kFixedHeaderSize, CHIP_ERROR_BUFFER_TOO_SMALL);

    Encoding::LittleEndian::Reader reader(mBuffer.Get(), mBufferOffset);
    uint32_t magic;
    uint8_t numImages;
    uint8_t reserved[3];
    ReturnErrorOnFailure(
        reader.Read32(&magic).Read8(&numImages).Read8(&reserved[0]).Read8(&reserved[1]).Read8(&reserved[2]).StatusCode());
    VerifyOrReturnError(magic == kMultiOTAImageFileIdentifier, CHIP_ERROR_INVALID_FILE_IDENTIFIER);
    VerifyOrReturnError(numImages != 0 && numImages <= kMaxNumImages, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(reserved[0] == 0 && reserved[1] == 0 && reserved[2] == 0, CHIP_ERROR_INVALID_ARGUMENT);
    mNumImages = numImages;
    VerifyOrReturnError(mBuffer.Alloc(mNumImages * kSubImageHeaderSize), CHIP_ERROR_NO_MEMORY);
    mBufferOffset = 0;
    mState        = State::kSubImages;
    return CHIP_NO_ERROR;
}

CHIP_ERROR MultiOTAImageHeaderParser::DecodeSubImageHeaders(MultiOTAImageHeader & header)
{
    VerifyOrReturnError(mBufferOffset >= mNumImages * kSubImageHeaderSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    // SubImageHeader has no padding (sizeof == 48), so the buffer can be read directly as an array of structs.
    auto * subImageHeaderEntries = reinterpret_cast<SubImageHeader *>(mBuffer.Get());

    for (uint8_t i = 0; i < mNumImages; i++)
    {
        VerifyOrReturnError(subImageHeaderEntries[i].imageId != 0, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(subImageHeaderEntries[i].length != 0, CHIP_ERROR_INVALID_ARGUMENT);
    }
    header.numImages = mNumImages;
    header.subImages = Span<const SubImageHeader>(subImageHeaderEntries, mNumImages);
    return CHIP_NO_ERROR;
}

} // namespace chip
