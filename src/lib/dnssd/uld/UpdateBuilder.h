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
#pragma once

#include <cstddef>
#include <cstdint>

#include <lib/dnssd/uld/Constants.h>
#include <lib/dnssd/wire/DnsHeader.h>
#include <lib/dnssd/wire/QName.h>
#include <lib/dnssd/wire/Query.h>
#include <lib/dnssd/wire/RecordWriter.h>
#include <lib/dnssd/wire/records/ResourceRecord.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

namespace chip {
namespace Dnssd {
namespace Uld {

/**
 * @brief Assembles a DNS UPDATE message (RFC 2136) for ULD registration.
 *
 * Owns the header and section orders, and appends ResourceRecord / Query objects via RecordWriter.
 *
 * Section mapping:
 * - Query (qdcount) -> ZONE
 * - Answer (ancount) -> Prerequisite
 * - Authority (nscount) -> Update
 * - Additional (arcount) -> Additional
 *
 * @note : All Add* methods can be chained. On failure they set a sticky "error state";
 *         When in a failed state, all Add* calls are no-ops.
 *         The assembly state can be checked by calling Ok().
 *         Reset()/Begin() clear the error state and allows for a new assembly.
 */
class UpdateBuilder
{
public:
    UpdateBuilder() : mHeader(nullptr), mOutput(nullptr, 0), mWriter(&mOutput) {}

    UpdateBuilder(uint8_t * buffer, size_t size) : mHeader(nullptr), mOutput(nullptr, 0), mWriter(&mOutput) { Reset(buffer, size); }

    /**
     * @brief Binds the builder to @p buffer and clears the DNS header.
     */
    UpdateBuilder & Reset(uint8_t * buffer, size_t size)
    {
        mBuffer  = buffer;
        mSize    = size;
        mHeader  = HeaderRef(nullptr);
        mBuildOk = false;

        if ((buffer != nullptr) && (size >= HeaderRef::kSizeBytes))
        {
            mHeader = HeaderRef(buffer);
            mHeader.Clear();
            mOutput = Encoding::BigEndian::BufferWriter(buffer, size);
            mOutput.Skip(HeaderRef::kSizeBytes);
            mWriter.Reset();
            mBuildOk = true;
        }

        return *this;
    }

    /**
     * @brief Starts a new UPDATE query with the given message id.
     */
    UpdateBuilder & Begin(uint16_t messageId)
    {
        // Allow restarting after a previous build failure as long as a valid buffer is bound.
        VerifyOrReturnValue((mBuffer != nullptr) && (mSize >= HeaderRef::kSizeBytes), *this);
        mBuildOk = true;

        mHeader.Clear();
        mHeader.SetMessageId(messageId);
        mHeader.SetAllFlags(BitPackedFlags(0).SetQuery().SetOpcode(kOpcodeUpdate));

        mOutput = Encoding::BigEndian::BufferWriter(mBuffer, mSize);
        mOutput.Skip(HeaderRef::kSizeBytes);
        mWriter.Reset();
        return *this;
    }

    /**
     * @brief Appends the ZONE question (SOA / IN). Must be written before records.
     */
    UpdateBuilder & AddZone(FullQName zoneName)
    {
        VerifyOrReturnValue(mBuildOk, *this);

        Query zone(zoneName);
        zone.SetType(QType::SOA).SetClass(QClass::IN).SetAnswerViaUnicast(false);

        if (!zone.Append(mHeader, mWriter))
        {
            mBuildOk = false;
        }
        return *this;
    }

    /** @brief Appends a prerequisite record (answer section). */
    UpdateBuilder & AddPrerequisite(const ResourceRecord & record) { return AddRecord(ResourceType::kAnswer, record); }

    /** @brief Appends an update record (authority section). */
    UpdateBuilder & AddUpdate(const ResourceRecord & record) { return AddRecord(ResourceType::kAuthority, record); }

    /** @brief Appends an additional record (additional section). */
    UpdateBuilder & AddAdditional(const ResourceRecord & record) { return AddRecord(ResourceType::kAdditional, record); }

    HeaderRef & Header() { return mHeader; }

    bool Ok() const { return mBuildOk; }

    /** @brief Total bytes written (header + body). */
    size_t PacketSize() const { return mOutput.Needed(); }

    /** @brief View of the encoded packet; empty if the builder is not Ok. */
    ByteSpan Packet() const
    {
        VerifyOrReturnValue(mBuildOk && (mBuffer != nullptr), ByteSpan());
        return ByteSpan(mBuffer, mOutput.Needed());
    }

private:
    UpdateBuilder & AddRecord(ResourceType type, const ResourceRecord & record)
    {
        VerifyOrReturnValue(mBuildOk, *this);

        if (!record.Append(mHeader, type, mWriter))
        {
            mBuildOk = false;
        }
        return *this;
    }

    HeaderRef mHeader;
    Encoding::BigEndian::BufferWriter mOutput;
    RecordWriter mWriter;

    uint8_t * mBuffer = nullptr;
    size_t mSize      = 0;
    bool mBuildOk     = false;
};

} // namespace Uld
} // namespace Dnssd
} // namespace chip
