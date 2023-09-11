/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
