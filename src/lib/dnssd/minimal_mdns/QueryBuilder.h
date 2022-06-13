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

#include <lib/dnssd/minimal_mdns/Query.h>
#include <lib/dnssd/minimal_mdns/core/DnsHeader.h>

namespace mdns {
namespace Minimal {

class QueryBuilder
{
public:
    QueryBuilder() : mHeader(nullptr) {}
    QueryBuilder(chip::System::PacketBufferHandle && packet) : mHeader(nullptr) { Reset(std::move(packet)); }

    QueryBuilder & Reset(chip::System::PacketBufferHandle && packet)
    {
        mPacket = std::move(packet);
        mHeader = HeaderRef(mPacket->Start());

        if (mPacket->AvailableDataLength() >= HeaderRef::kSizeBytes)
        {
            mPacket->SetDataLength(HeaderRef::kSizeBytes);
            mHeader.Clear();
        }
        else
        {
            mQueryBuildOk = false;
        }

        mHeader.SetFlags(mHeader.GetFlags().SetQuery());
        return *this;
    }

    CHECK_RETURN_VALUE
    chip::System::PacketBufferHandle && ReleasePacket()
    {
        mHeader       = HeaderRef(nullptr);
        mQueryBuildOk = false;
        return std::move(mPacket);
    }

    HeaderRef & Header() { return mHeader; }

    QueryBuilder & AddQuery(const Query & query)
    {
        if (!mQueryBuildOk)
        {
            return *this;
        }

        chip::Encoding::BigEndian::BufferWriter out(mPacket->Start() + mPacket->DataLength(), mPacket->AvailableDataLength());
        RecordWriter writer(&out);

        if (!query.Append(mHeader, writer))
        {
            mQueryBuildOk = false;
        }
        else
        {
            mPacket->SetDataLength(static_cast<uint16_t>(mPacket->DataLength() + out.Needed()));
        }
        return *this;
    }

    bool Ok() const { return mQueryBuildOk; }

private:
    chip::System::PacketBufferHandle mPacket;
    HeaderRef mHeader;
    bool mQueryBuildOk = true;
};

} // namespace Minimal
} // namespace mdns
