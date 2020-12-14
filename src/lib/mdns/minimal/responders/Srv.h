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

#include <mdns/minimal/records/Srv.h>
#include <mdns/minimal/responders/Responder.h>

namespace mdns {
namespace Minimal {

class SrvResponder : public Responder
{
public:
    SrvResponder(const FullQName & qname, const SrvResourceRecord & record) : Responder(QType::SRV, qname), mRecord(record) {}

    void SetRecord(const SrvResourceRecord & record) { mRecord = record; }

    void AddAllResponses(const chip::Inet::IPPacketInfo * source, ResponderDelegate * delegate) override
    {
        delegate->AddResponse(mRecord);
    }

private:
    SrvResourceRecord mRecord;
};

} // namespace Minimal
} // namespace mdns
