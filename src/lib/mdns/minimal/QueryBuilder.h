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
#include "Query.h"

namespace mdns {
namespace Minimal {

class QueryBuilder
{
public:
    QueryBuilder(chip::System::PacketBuffer * packet) : mPacket(packet), mHeader(mPacket->Start())
    {

        if (mPacket->AvailableDataLength() >= HeaderRef::kSizeBytes)
        {
            mPacket->SetDataLength(HeaderRef::kSizeBytes);
            mHeader.Clear();
        }
        else
        {
            mQueryBuidOk = false;
        }

        mHeader.SetFlags(mHeader.GetFlags().SetQuery());
    }

    HeaderRef & Header() { return mHeader; }

    QueryBuilder & AddQuery(const Query & query)
    {
        if (!mQueryBuidOk)
        {
            return *this;
        }

        if (mPacket->AvailableDataLength() < query.WriteSizeBytes())
        {
            mQueryBuidOk = false;
            return *this;
        }

        if (query.Append(mHeader, mPacket->Start() + mPacket->DataLength(), mPacket->AvailableDataLength()) == nullptr)
        {
            mQueryBuidOk = false;
            return *this;
        }

        mPacket->SetDataLength(static_cast<uint16_t>(mPacket->DataLength() + query.WriteSizeBytes()));
        return *this;
    }

    bool Ok() const { return mQueryBuidOk; }

private:
    chip::System::PacketBuffer * mPacket;
    HeaderRef mHeader;
    bool mQueryBuidOk = true;
};

} // namespace Minimal

} // namespace mdns
