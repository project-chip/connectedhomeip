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

#include <lib/dnssd/minimal_mdns/records/ResourceRecord.h>

namespace mdns {
namespace Minimal {

class SrvResourceRecord : public ResourceRecord
{
public:
    SrvResourceRecord(const FullQName & qName, const FullQName & serverName, uint16_t port) :
        ResourceRecord(QType::SRV, qName), mServerName(serverName), mPort(port)
    {}
    SrvResourceRecord(const SrvResourceRecord & other) = default;
    SrvResourceRecord & operator=(const SrvResourceRecord & other) = default;

    FullQName GetServerName() const { return mServerName; }
    uint16_t GetPort() const { return mPort; }
    uint16_t GetPriority() const { return mPriority; }
    uint16_t GetWeight() const { return mWeight; }

    void SetPriority(uint16_t value) { mPriority = value; }
    void SetWeight(uint16_t value) { mWeight = value; }

protected:
    bool WriteData(RecordWriter & out) const override
    {
        return out.Put16(mPriority).Put16(mWeight).Put16(mPort).WriteQName(mServerName).Fit();
    }

private:
    FullQName mServerName;
    uint16_t mPort;
    uint16_t mPriority = 0;
    uint16_t mWeight   = 0;
};

} // namespace Minimal
} // namespace mdns
