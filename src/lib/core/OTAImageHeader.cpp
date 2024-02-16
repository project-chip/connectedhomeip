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
#include <lib/core/OTAImageHeader.h>

#include <lib/core/CHIPError.h>
#include <lib/core/Optional.h>
#include <lib/core/TLVReader.h>
#include <lib/core/TLVTags.h>
#include <lib/core/TLVTypes.h>
#include <lib/support/BufferReader.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>

#include <string.h>

namespace chip {

namespace {

enum class Tag : uint8_t
{
    kVendorId              = 0,
    kProductId             = 1,
    kSoftwareVersion       = 2,
    kSoftwareVersionString = 3,
    kPayloadSize           = 4,
    kMinApplicableVersion  = 5,
    kMaxApplicableVersion  = 6,
    kReleaseNotesURL       = 7,
    kImageDigestType       = 8,
    kImageDigest           = 9,
};

/// Length of the fixed portion of the Matter OTA image header: FileIdentifier (4B), TotalSize (8B) and HeaderSize (4B)
constexpr uint32_t kFixedHeaderSize = 16;

/// Maximum supported Matter OTA image header size
constexpr uint32_t kMaxHeaderSize = 1024;

/// Maximum size of the software version string
constexpr size_t kMaxSoftwareVersionStringSize = 64;

/// Maximum size of the release notes URL
constexpr size_t kMaxReleaseNotesURLSize = 256;

} // namespace

void OTAImageHeaderParser::Init()
{
    mState         = State::kInitialized;
    mBufferOffset  = 0;
    mHeaderTlvSize = 0;
    mBuffer.Alloc(kFixedHeaderSize);
}

void OTAImageHeaderParser::Clear()
{
    mState         = State::kNotInitialized;
    mBufferOffset  = 0;
    mHeaderTlvSize = 0;
    mBuffer.Free();
}

CHIP_ERROR OTAImageHeaderParser::AccumulateAndDecode(ByteSpan & buffer, OTAImageHeader & header)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    if (mState == State::kInitialized)
    {
        Append(buffer, kFixedHeaderSize - mBufferOffset);
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

void OTAImageHeaderParser::Append(ByteSpan & buffer, uint32_t numBytes)
{
    numBytes = chip::min(numBytes, static_cast<uint32_t>(buffer.size()));
    memcpy(&mBuffer[mBufferOffset], buffer.data(), numBytes);
    mBufferOffset += numBytes;
    buffer = buffer.SubSpan(numBytes);
}

CHIP_ERROR OTAImageHeaderParser::DecodeFixed()
{
    ReturnErrorCodeIf(mBufferOffset < kFixedHeaderSize, CHIP_ERROR_BUFFER_TOO_SMALL);

    Encoding::LittleEndian::Reader reader(mBuffer.Get(), mBufferOffset);
    uint32_t fileIdentifier;
    uint64_t totalSize;
    ReturnErrorOnFailure(reader.Read32(&fileIdentifier).Read64(&totalSize).Read32(&mHeaderTlvSize).StatusCode());
    ReturnErrorCodeIf(fileIdentifier != kOTAImageFileIdentifier, CHIP_ERROR_INVALID_FILE_IDENTIFIER);
    // Safety check against malicious headers.
    ReturnErrorCodeIf(mHeaderTlvSize > kMaxHeaderSize, CHIP_ERROR_NO_MEMORY);
    ReturnErrorCodeIf(!mBuffer.Alloc(mHeaderTlvSize), CHIP_ERROR_NO_MEMORY);

    mState        = State::kTlv;
    mBufferOffset = 0;

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageHeaderParser::DecodeTlv(OTAImageHeader & header)
{
    ReturnErrorCodeIf(mBufferOffset < mHeaderTlvSize, CHIP_ERROR_BUFFER_TOO_SMALL);

    TLV::TLVReader tlvReader;
    tlvReader.Init(mBuffer.Get(), mBufferOffset);
    ReturnErrorOnFailure(tlvReader.Next(TLV::TLVType::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(tlvReader.EnterContainer(outerType));

    ReturnErrorOnFailure(tlvReader.Next(TLV::ContextTag(Tag::kVendorId)));
    ReturnErrorOnFailure(tlvReader.Get(header.mVendorId));
    ReturnErrorOnFailure(tlvReader.Next(TLV::ContextTag(Tag::kProductId)));
    ReturnErrorOnFailure(tlvReader.Get(header.mProductId));
    ReturnErrorOnFailure(tlvReader.Next(TLV::ContextTag(Tag::kSoftwareVersion)));
    ReturnErrorOnFailure(tlvReader.Get(header.mSoftwareVersion));
    ReturnErrorOnFailure(tlvReader.Next(TLV::ContextTag(Tag::kSoftwareVersionString)));
    ReturnErrorOnFailure(tlvReader.Get(header.mSoftwareVersionString));
    ReturnErrorCodeIf(header.mSoftwareVersionString.size() > kMaxSoftwareVersionStringSize, CHIP_ERROR_INVALID_STRING_LENGTH);
    ReturnErrorOnFailure(tlvReader.Next(TLV::ContextTag(Tag::kPayloadSize)));
    ReturnErrorOnFailure(tlvReader.Get(header.mPayloadSize));
    ReturnErrorOnFailure(tlvReader.Next());

    if (tlvReader.GetTag() == TLV::ContextTag(Tag::kMinApplicableVersion))
    {
        ReturnErrorOnFailure(tlvReader.Get(header.mMinApplicableVersion.Emplace()));
        ReturnErrorOnFailure(tlvReader.Next());
    }

    if (tlvReader.GetTag() == TLV::ContextTag(Tag::kMaxApplicableVersion))
    {
        ReturnErrorOnFailure(tlvReader.Get(header.mMaxApplicableVersion.Emplace()));
        ReturnErrorOnFailure(tlvReader.Next());
    }

    if (tlvReader.GetTag() == TLV::ContextTag(Tag::kReleaseNotesURL))
    {
        ReturnErrorOnFailure(tlvReader.Get(header.mReleaseNotesURL));
        ReturnErrorCodeIf(header.mReleaseNotesURL.size() > kMaxReleaseNotesURLSize, CHIP_ERROR_INVALID_STRING_LENGTH);
        ReturnErrorOnFailure(tlvReader.Next());
    }

    ReturnErrorOnFailure(tlvReader.Expect(TLV::ContextTag(Tag::kImageDigestType)));
    ReturnErrorOnFailure(tlvReader.Get(header.mImageDigestType));
    ReturnErrorOnFailure(tlvReader.Next(TLV::ContextTag(Tag::kImageDigest)));
    ReturnErrorOnFailure(tlvReader.Get(header.mImageDigest));

    ReturnErrorOnFailure(tlvReader.ExitContainer(outerType));

    return CHIP_NO_ERROR;
}

} // namespace chip
