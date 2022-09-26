/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <lib/core/CHIPEncoding.h>

namespace mdns {
namespace Minimal {

/**
 * Wrapper around a MDNS bit-packed flags in a DNS header as defined in
 * RFC 1035 and RFC 6762
 *
 * | 0| 1 2 3 4| 5| 6| 7| 8| 9| 0| 1| 2 3 4 5 |
 * |QR| OPCODE |AA|TC|RD|RA| Z|AD|CD| RCODE   |
 *
 * RFC 6762 states:
 *   - OPCODE must be 0 on transmission and queries received that are not 0 MUST be ignored
 *   - RCODE must be 0 on transmission and messages received with non-zero must be silently ignored
 *
 *   - AA (Authoritative Answer) MUST be 0 on transmission, ignored on reception
 *   - RD (Recursion desired) MUST be 0 on transmission, ignored on reception
 *   - RA (Recursion available) MUST be 0 on transmission, ignored on reception
 *   - AD (Authentic data) MUST be 0 on transmission, ignored on reception
 *   - CD (Checking Disabled) MUST be 0 on transmission, ignored on reception
 *
 * Accessors are only provided on useful values
 */
class BitPackedFlags
{
public:
    explicit BitPackedFlags(uint16_t value) : mValue(value) {}

    uint16_t RawValue() const { return mValue & kMdnsNonIgnoredMask; }

    bool IsQuery() const { return (mValue & kIsResponseMask) == 0; }
    BitPackedFlags & SetQuery() { return ClearMask(kIsResponseMask); }

    bool IsResponse() const { return (mValue & kIsResponseMask) == kIsResponseMask; }
    BitPackedFlags & SetResponse() { return SetMask(kIsResponseMask); }

    bool IsAuthoritative() const { return (mValue & kAuthoritativeMask) == kAuthoritativeMask; }
    BitPackedFlags & SetAuthoritative() { return SetMask(kAuthoritativeMask); }

    bool IsTruncated() const { return (mValue & kTruncationMask) != 0; }
    BitPackedFlags & SetTruncated(bool value) { return value ? SetMask(kTruncationMask) : ClearMask(kTruncationMask); }

    /// Validates that the message does not need to be ignored according to
    /// RFC 6762
    bool IsValidMdns() const { return (mValue & (kOpcodeMask | kReturnCodeMask)) == 0; }

private:
    uint16_t mValue = 0;

    inline BitPackedFlags & ClearMask(uint16_t mask)
    {
        mValue &= static_cast<uint16_t>(~mask);
        return *this;
    }

    inline BitPackedFlags & SetMask(uint16_t mask)
    {
        mValue |= mask;
        return *this;
    }

    // Mask to limit values to what RFC 6762 consideres useful
    // 1111 1110 0000 0000 = FE0F
    // TODO(cecille): need to better document this value. Why is the comment different than the value?
    static constexpr uint16_t kMdnsNonIgnoredMask = 0x8E08;
    static constexpr uint16_t kAuthoritativeMask  = 0x0400;
    static constexpr uint16_t kIsResponseMask     = 0x8000;
    static constexpr uint16_t kOpcodeMask         = 0x7000;
    static constexpr uint16_t kTruncationMask     = 0x0200;
    static constexpr uint16_t kReturnCodeMask     = 0x000F;
};

/**
 * Allows operations on a DNS header. A DNS Header is defined in RFC 1035
 * and looks like this:
 *
 * | 0| 1 2 3 4| 5| 6| 7| 8| 9| 0| 1| 2 3 4 5 |
 * |               Message ID                 |
 * |QR| OPCODE |AA|TC|RD|RA| Z|AD|CD| RCODE   |
 * |       Items in QUESTION Section          |
 * |       Items in ANSWER Section            |
 * |       Items in AUTHORITY Section         |
 * |       Items in ADDITIONAL Section        |
 */
class ConstHeaderRef
{
public:
    static constexpr size_t kSizeBytes = 12; /// size of a DNS header structure
    ConstHeaderRef(const uint8_t * buffer) : mBuffer(buffer) {}

    uint16_t GetMessageId() const { return Get16At(kMessageIdOffset); }

    BitPackedFlags GetFlags() const { return BitPackedFlags(Get16At(kFlagsOffset)); }

    uint16_t GetQueryCount() const { return Get16At(kQueryCountOffset); }
    uint16_t GetAnswerCount() const { return Get16At(kAnswerCountOffset); }
    uint16_t GetAuthorityCount() const { return Get16At(kAuthorityCountOffset); }
    uint16_t GetAdditionalCount() const { return Get16At(kAdditionalCountOffset); }

protected:
    const uint8_t * mBuffer;

    inline uint16_t Get16At(size_t offset) const { return chip::Encoding::BigEndian::Get16(mBuffer + offset); }
    uint16_t GetRawFlags() const { return Get16At(kFlagsOffset); }

    static constexpr size_t kMessageIdOffset       = 0;
    static constexpr size_t kFlagsOffset           = 2;
    static constexpr size_t kQueryCountOffset      = 4;
    static constexpr size_t kAnswerCountOffset     = 6;
    static constexpr size_t kAuthorityCountOffset  = 8;
    static constexpr size_t kAdditionalCountOffset = 10;
};

class HeaderRef : public ConstHeaderRef
{
public:
    HeaderRef(uint8_t * buffer) : ConstHeaderRef(buffer) {}
    HeaderRef(const HeaderRef & other) = default;
    HeaderRef & operator=(const HeaderRef & other) = default;

    HeaderRef & Clear()
    {
        memset(GetWritable(), 0, kSizeBytes);
        return *this;
    }

    HeaderRef & SetMessageId(uint16_t value) { return Set16At(kMessageIdOffset, value); }
    HeaderRef & SetFlags(BitPackedFlags flags) { return Set16At(kFlagsOffset, flags.RawValue()); }
    HeaderRef & SetQueryCount(uint16_t value) { return Set16At(kQueryCountOffset, value); }
    HeaderRef & SetAnswerCount(uint16_t value) { return Set16At(kAnswerCountOffset, value); }
    HeaderRef & SetAuthorityCount(uint16_t value) { return Set16At(kAuthorityCountOffset, value); }
    HeaderRef & SetAdditionalCount(uint16_t value) { return Set16At(kAdditionalCountOffset, value); }

private:
    /// Returns the internal buffer as writable. Const-cast is correct because
    /// the construct took a non-const buffer as well.
    uint8_t * GetWritable() { return const_cast<uint8_t *>(mBuffer); }

    inline HeaderRef & Set16At(size_t offset, uint16_t value)
    {
        chip::Encoding::BigEndian::Put16(GetWritable() + offset, value);
        return *this;
    }

    HeaderRef & SetRawFlags(uint16_t value) { return Set16At(kFlagsOffset, value); }
};

} // namespace Minimal
} // namespace mdns
