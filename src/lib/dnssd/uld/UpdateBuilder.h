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

#include <lib/core/CHIPError.h>
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
 * Add* methods have documented ordering preconditions. Not respecting them will result in VerifyOrDie failures.
 * Buffer overflow is sticky and makes subsequent Add* calls no-ops.
 * Call GetPacket() to obtain the encoded message or report CHIP_ERROR_BUFFER_TOO_SMALL.
 */
class UpdateBuilder
{
public:
    UpdateBuilder(uint8_t * buffer, size_t size) :
        mHeader(nullptr), mOutput(nullptr, 0), mWriter(&mOutput), mBuffer(buffer), mSize(size)
    {}

    /**
     * @brief Starts a new UPDATE query with the given message id.
     */
    void Begin(uint16_t messageId)
    {
        mBuildStarted = true;
        mBuildOk      = (mBuffer != nullptr) && (mSize >= HeaderRef::kSizeBytes);
        VerifyOrReturn(mBuildOk);

        mHeader = HeaderRef(mBuffer);
        mHeader.Clear();
        mHeader.SetMessageId(messageId);
        mHeader.SetAllFlags(BitPackedFlags(0).SetQuery().SetOpcode(Opcode::kUpdate));

        mOutput = Encoding::BigEndian::BufferWriter(mBuffer, mSize);
        mOutput.Skip(HeaderRef::kSizeBytes);
        mWriter.Reset();
    }

    /**
     * @brief Appends the ZONE question (SOA / IN).
     *
     * Shall be called exactly once, before any records are added.
     */
    void AddZone(FullQName zoneName)
    {
        VerifyOrDie(mBuildStarted);
        VerifyOrReturn(mBuildOk);
        VerifyOrDie((mHeader.GetQueryCount() == 0) && (mHeader.GetAnswerCount() == 0) && (mHeader.GetAuthorityCount() == 0) &&
                    (mHeader.GetAdditionalCount() == 0));

        Query zone(zoneName);
        zone.SetType(QType::SOA).SetClass(QClass::IN).SetAnswerViaUnicast(false);

        mBuildOk = zone.Append(mHeader, mWriter);
    }

    /**
     * @brief Appends a prerequisite record (answer section).
     *
     * Shall be called after AddZone() and before AddUpdate() or AddAdditional().
     */
    void AddPrerequisite(const ResourceRecord & record) { AddRecord(ResourceType::kAnswer, record); }

    /**
     * @brief Appends an update record (authority section).
     *
     * Shall be called after AddZone() and before AddAdditional().
     */
    void AddUpdate(const ResourceRecord & record) { AddRecord(ResourceType::kAuthority, record); }

    /**
     * @brief Appends an additional record (additional section).
     *
     * Shall be called after AddZone().
     */
    void AddAdditional(const ResourceRecord & record) { AddRecord(ResourceType::kAdditional, record); }

    HeaderRef & Header()
    {
        VerifyOrDie(mBuildStarted && mBuildOk);
        return mHeader;
    }

    /** @brief Returns the encoded message built so far. */
    CHIP_ERROR GetPacket(ByteSpan & out) const
    {
        VerifyOrReturnError(mBuildOk, CHIP_ERROR_BUFFER_TOO_SMALL);
        out = ByteSpan(mBuffer, mOutput.Needed());
        return CHIP_NO_ERROR;
    }

private:
    void AddRecord(ResourceType type, const ResourceRecord & record)
    {
        VerifyOrDie(mBuildStarted);
        VerifyOrReturn(mBuildOk);
        VerifyOrDie(mHeader.GetQueryCount() == 1);

        if (type == ResourceType::kAnswer)
        {
            VerifyOrDie((mHeader.GetAuthorityCount() == 0) && (mHeader.GetAdditionalCount() == 0));
        }
        else if (type == ResourceType::kAuthority)
        {
            VerifyOrDie(mHeader.GetAdditionalCount() == 0);
        }

        mBuildOk = record.Append(mHeader, type, mWriter);
    }

    HeaderRef mHeader;
    Encoding::BigEndian::BufferWriter mOutput;
    RecordWriter mWriter;

    uint8_t * mBuffer  = nullptr;
    size_t mSize       = 0;
    bool mBuildOk      = false;
    bool mBuildStarted = false;
};

} // namespace Uld
} // namespace Dnssd
} // namespace chip
