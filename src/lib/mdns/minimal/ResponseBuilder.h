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

#include "DnsHeader.h"
#include "ResourceRecord.h"

namespace mdns {
namespace Minimal {

class ResponseBuilder
{
public:
    ResponseBuilder(const chip::System::PacketBufferHandle & packet) : mPacket(packet), mHeader(mPacket->Start())
    {

        if (mPacket->AvailableDataLength() >= HeaderRef::kSizeBytes)
        {
            mPacket->SetDataLength(HeaderRef::kSizeBytes);
            mHeader.Clear();
        }
        else
        {
            mBuildOk = false;
        }

        mHeader.SetFlags(mHeader.GetFlags().SetResponse());
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

        chip::BufBound out(mPacket->Start() + mPacket->DataLength(), mPacket->AvailableDataLength());

        if (!record.Append(mHeader, type, out))
        {
            mBuildOk = false;
        }
        else
        {
            mPacket->SetDataLength(static_cast<uint16_t>(mPacket->DataLength() + out.Written()));
        }
        return *this;
    }

    bool Ok() const { return mBuildOk; }

private:
    const chip::System::PacketBufferHandle & mPacket;
    HeaderRef mHeader;
    bool mBuildOk = true;
};

} // namespace Minimal
} // namespace mdns
