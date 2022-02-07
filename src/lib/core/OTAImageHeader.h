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

#pragma once

#include <lib/core/Optional.h>
#include <lib/support/Span.h>

#include <cstdint>

namespace chip {

constexpr uint32_t kOTAImageFileIdentifier = 0x1BEEF11E;

enum class OTAImageDigestType : uint8_t
{
    kSha256     = 1,
    kSha256_128 = 2,
    kSha256_120 = 3,
    kSha256_96  = 4,
    kSha256_64  = 5,
    kSha256_32  = 6,
    kSha384     = 7,
    kSha512     = 8,
    kSha3_224   = 9,
    kSha3_256   = 10,
    kSha3_384   = 11,
    kSha3_512   = 12,
};

struct OTAImageHeader
{
    uint64_t mTotalSize;
    uint32_t mHeaderSize;
    uint16_t mVendorId;
    uint16_t mProductId;
    uint32_t mSoftwareVersion;
    CharSpan mSoftwareVersionString;
    uint64_t mPayloadSize;
    Optional<uint32_t> mMinApplicableVersion;
    Optional<uint32_t> mMaxApplicableVersion;
    CharSpan mReleaseNotesURL;
    OTAImageDigestType mImageDigestType;
    ByteSpan mImageDigest;
};

CHIP_ERROR DecodeOTAImageHeader(ByteSpan buffer, OTAImageHeader & header);

} // namespace chip
