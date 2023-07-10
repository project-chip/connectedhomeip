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

#include <lib/dnssd/minimal_mdns/records/Txt.h>
#include <lib/dnssd/minimal_mdns/responders/RecordResponder.h>

namespace mdns {
namespace Minimal {

class TxtResponder : public RecordResponder
{
public:
    TxtResponder(const TxtResourceRecord & record) : RecordResponder(QType::TXT, record.GetName()), mRecord(record) {}

    void AddAllResponses(const chip::Inet::IPPacketInfo * source, ResponderDelegate * delegate,
                         const ResponseConfiguration & configuration) override
    {
        if (!delegate->ShouldSend(*this))
        {
            return;
        }

        TxtResourceRecord record = mRecord;
        configuration.Adjust(record);
        delegate->AddResponse(record);
        delegate->ResponsesAdded(*this);
    }

private:
    const TxtResourceRecord mRecord;
};

} // namespace Minimal
} // namespace mdns
