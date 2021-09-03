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

#include <system/SystemPacketBuffer.h>

#include <lib/mdns/minimal/core/DnsHeader.h>
#include <lib/mdns/minimal/records/ResourceRecord.h>

namespace mdns {
namespace Minimal {

/// Writes a MDNS reply into a given packet buffer.
class ResponseBuilder
{
public:
    ResponseBuilder() : mHeader(nullptr) {}
    ResponseBuilder(chip::System::PacketBufferHandle && packet) : mHeader(nullptr) { Reset(std::move(packet)); }

    ResponseBuilder & Reset(chip::System::PacketBufferHandle && packet)
    {
        mPacket = std::move(packet);
        mHeader = HeaderRef(mPacket->Start());

        if (mPacket->AvailableDataLength() >= HeaderRef::kSizeBytes)
        {
            mPacket->SetDataLength(HeaderRef::kSizeBytes);
            mHeader.Clear();
            mBuildOk = true;
        }
        else
        {
            mBuildOk = false;
        }

        mHeader.SetFlags(mHeader.GetFlags().SetResponse());
        return *this;
    }

    CHECK_RETURN_VALUE
    chip::System::PacketBufferHandle ReleasePacket()
    {
        mHeader  = HeaderRef(nullptr);
        mBuildOk = false;
        return std::move(mPacket);
    }

    bool HasResponseRecords() const
    {
        return (mHeader.GetAnswerCount() != 0) || (mHeader.GetAuthorityCount() != 0) || (mHeader.GetAdditionalCount() != 0);
    }

    HeaderRef & Header() { return mHeader; }

    /// Attempts to add a record to the currentsystem packet buffer.
    /// On success, the packet buffer data length is updated.
    /// On failure, the packet buffer data length is NOT updated and header is unchanged.
    ResponseBuilder & AddRecord(ResourceType type, const ResourceRecord & record)
    {
        if (!mBuildOk)
        {
            return *this;
        }

        chip::Encoding::BigEndian::BufferWriter out(mPacket->Start() + mPacket->DataLength(), mPacket->AvailableDataLength());

        if (!record.Append(mHeader, type, out))
        {
            mBuildOk = false;
        }
        else
        {
            mPacket->SetDataLength(static_cast<uint16_t>(mPacket->DataLength() + out.Needed()));
        }
        return *this;
    }

    ResponseBuilder & AddQuery(const QueryData & query)
    {
        if (!mBuildOk)
        {
            return *this;
        }

        chip::Encoding::BigEndian::BufferWriter out(mPacket->Start() + mPacket->DataLength(), mPacket->AvailableDataLength());

        if (!query.Append(mHeader, out))
        {
            mBuildOk = false;
        }
        else
        {
            mPacket->SetDataLength(static_cast<uint16_t>(mPacket->DataLength() + out.Needed()));
        }
        return *this;
    }

    bool Ok() const { return mBuildOk; }
    bool HasPacketBuffer() const { return !mPacket.IsNull(); }

private:
    chip::System::PacketBufferHandle mPacket;
    HeaderRef mHeader;
    bool mBuildOk = false;
};

} // namespace Minimal
} // namespace mdns
