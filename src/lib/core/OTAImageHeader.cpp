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

#include "OTAImageHeader.h"

#include <lib/core/CHIPTLV.h>
#include <lib/support/BufferReader.h>
#include <lib/support/CodeUtils.h>

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

} // namespace

CHIP_ERROR DecodeOTAImageHeader(ByteSpan buffer, OTAImageHeader & header)
{
    Encoding::LittleEndian::Reader reader(buffer);

    // Parse the fixed part of the header
    uint32_t fileIdentifier;
    ReturnErrorOnFailure(reader.Read32(&fileIdentifier).StatusCode());
    VerifyOrReturnError(fileIdentifier == kOTAImageFileIdentifier, CHIP_ERROR_INVALID_FILE_IDENTIFIER);
    ReturnErrorOnFailure(reader.Read64(&header.mTotalSize).Read32(&header.mHeaderSize).StatusCode());

    // Parse the TLV elements of the header
    VerifyOrReturnError(header.mHeaderSize <= reader.Remaining(), CHIP_ERROR_BUFFER_TOO_SMALL);

    TLV::TLVReader tlvReader;
    tlvReader.Init(buffer.data() + reader.OctetsRead(), header.mHeaderSize);
    ReturnErrorOnFailure(tlvReader.Next(TLV::TLVType::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(tlvReader.EnterContainer(outerType));

    ReturnErrorOnFailure(tlvReader.Next(TLV::ContextTag(to_underlying(Tag::kVendorId))));
    ReturnErrorOnFailure(tlvReader.Get(header.mVendorId));
    ReturnErrorOnFailure(tlvReader.Next(TLV::ContextTag(to_underlying(Tag::kProductId))));
    ReturnErrorOnFailure(tlvReader.Get(header.mProductId));
    ReturnErrorOnFailure(tlvReader.Next(TLV::ContextTag(to_underlying(Tag::kSoftwareVersion))));
    ReturnErrorOnFailure(tlvReader.Get(header.mSoftwareVersion));
    ReturnErrorOnFailure(tlvReader.Next(TLV::ContextTag(to_underlying(Tag::kSoftwareVersionString))));
    ReturnErrorOnFailure(tlvReader.Get(header.mSoftwareVersionString));
    ReturnErrorOnFailure(tlvReader.Next(TLV::ContextTag(to_underlying(Tag::kPayloadSize))));
    ReturnErrorOnFailure(tlvReader.Get(header.mPayloadSize));
    ReturnErrorOnFailure(tlvReader.Next());

    if (tlvReader.GetTag() == TLV::ContextTag(to_underlying(Tag::kMinApplicableVersion)))
    {
        ReturnErrorOnFailure(tlvReader.Get(header.mMinApplicableVersion.Emplace()));
        ReturnErrorOnFailure(tlvReader.Next());
    }

    if (tlvReader.GetTag() == TLV::ContextTag(to_underlying(Tag::kMaxApplicableVersion)))
    {
        ReturnErrorOnFailure(tlvReader.Get(header.mMaxApplicableVersion.Emplace()));
        ReturnErrorOnFailure(tlvReader.Next());
    }

    if (tlvReader.GetTag() == TLV::ContextTag(to_underlying(Tag::kReleaseNotesURL)))
    {
        ReturnErrorOnFailure(tlvReader.Get(header.mReleaseNotesURL));
        ReturnErrorOnFailure(tlvReader.Next());
    }

    VerifyOrReturnError(tlvReader.GetTag() == TLV::ContextTag(to_underlying(Tag::kImageDigestType)),
                        CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);
    ReturnErrorOnFailure(tlvReader.Get(header.mImageDigestType));
    ReturnErrorOnFailure(tlvReader.Next(TLV::ContextTag(to_underlying(Tag::kImageDigest))));
    ReturnErrorOnFailure(tlvReader.Get(header.mImageDigest));

    ReturnErrorOnFailure(tlvReader.ExitContainer(outerType));

    return CHIP_NO_ERROR;
}

} // namespace chip
