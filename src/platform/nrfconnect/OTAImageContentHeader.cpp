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

#include "OTAImageContentHeader.h"

#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TypeTraits.h>

namespace chip {
namespace DeviceLayer {

namespace {

enum class HeaderTag : uint8_t
{
    kFileInfoList = 0,
};

enum class FileInfoTag : uint8_t
{
    kFileId   = 0,
    kFileSize = 1,
};

/// Maximum supported Matter OTA image content header size
constexpr uint32_t kMaxHeaderSize = 1024;
} // namespace

void OTAImageContentHeaderParser::Init()
{
    mState         = State::kInitialized;
    mBufferOffset  = 0;
    mHeaderTlvSize = 0;
    mBuffer.Alloc(sizeof(mHeaderTlvSize));
}

void OTAImageContentHeaderParser::Clear()
{
    mState         = State::kNotInitialized;
    mBufferOffset  = 0;
    mHeaderTlvSize = 0;
    mBuffer.Free();
}

CHIP_ERROR OTAImageContentHeaderParser::AccumulateAndDecode(ByteSpan & buffer, OTAImageContentHeader & header)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    if (mState == State::kInitialized)
    {
        ReturnErrorCodeIf(!mBuffer, CHIP_ERROR_NO_MEMORY);
        Append(buffer, sizeof(mHeaderTlvSize) - mBufferOffset);
        error = DecodeFixed();
    }

    if (mState == State::kTlv)
    {
        Append(buffer, mHeaderTlvSize - mBufferOffset);
        error = DecodeTlv(header);
    }

    if (error != CHIP_NO_ERROR && error != CHIP_ERROR_BUFFER_TOO_SMALL)
    {
        Clear();
    }

    return error;
}

void OTAImageContentHeaderParser::Append(ByteSpan & buffer, uint32_t numBytes)
{
    numBytes = chip::min(numBytes, static_cast<uint32_t>(buffer.size()));
    memcpy(&mBuffer[mBufferOffset], buffer.data(), numBytes);
    mBufferOffset += numBytes;
    buffer = buffer.SubSpan(numBytes);
}

CHIP_ERROR OTAImageContentHeaderParser::DecodeFixed()
{
    ReturnErrorCodeIf(mBufferOffset < sizeof(mHeaderTlvSize), CHIP_ERROR_BUFFER_TOO_SMALL);

    mHeaderTlvSize = Encoding::LittleEndian::Get32(mBuffer.Get());
    // Safety check against malicious headers.
    ReturnErrorCodeIf(mHeaderTlvSize > kMaxHeaderSize, CHIP_ERROR_NO_MEMORY);
    ReturnErrorCodeIf(!mBuffer.Alloc(mHeaderTlvSize), CHIP_ERROR_NO_MEMORY);

    mState        = State::kTlv;
    mBufferOffset = 0;

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageContentHeaderParser::DecodeTlv(OTAImageContentHeader & header)
{
    ReturnErrorCodeIf(mBufferOffset < mHeaderTlvSize, CHIP_ERROR_BUFFER_TOO_SMALL);

    TLV::TLVReader tlvReader;
    tlvReader.Init(mBuffer.Get(), mBufferOffset);
    ReturnErrorOnFailure(tlvReader.Next(TLV::TLVType::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(tlvReader.EnterContainer(outerType));

    TLV::TLVType headerType;
    ReturnErrorOnFailure(tlvReader.Next(TLV::TLVType::kTLVType_Array, TLV::ContextTag(to_underlying(HeaderTag::kFileInfoList))));
    ReturnErrorOnFailure(tlvReader.EnterContainer(headerType));

    size_t fileNo = 0;

    while (tlvReader.Next(TLV::TLVType::kTLVType_Structure, TLV::AnonymousTag()) == CHIP_NO_ERROR)
    {
        ReturnErrorCodeIf(fileNo >= OTAImageContentHeader::kMaxFiles, CHIP_ERROR_NO_MEMORY);

        TLV::TLVType arrayType;
        ReturnErrorOnFailure(tlvReader.EnterContainer(arrayType));

        ReturnErrorOnFailure(tlvReader.Next(TLV::ContextTag(to_underlying(FileInfoTag::kFileId))));
        ReturnErrorOnFailure(tlvReader.Get(header.mFiles[fileNo].mFileId));
        ReturnErrorOnFailure(tlvReader.Next(TLV::ContextTag(to_underlying(FileInfoTag::kFileSize))));
        ReturnErrorOnFailure(tlvReader.Get(header.mFiles[fileNo].mFileSize));
        ++fileNo;

        ReturnErrorOnFailure(tlvReader.ExitContainer(arrayType));
    }

    ReturnErrorOnFailure(tlvReader.ExitContainer(headerType));
    ReturnErrorOnFailure(tlvReader.ExitContainer(outerType));

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
