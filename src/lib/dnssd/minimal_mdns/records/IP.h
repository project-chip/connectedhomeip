/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <inet/IPAddress.h>

#include <lib/dnssd/minimal_mdns/records/ResourceRecord.h>

namespace mdns {
namespace Minimal {

class IPResourceRecord : public ResourceRecord
{
public:
    IPResourceRecord(const FullQName & qName, const chip::Inet::IPAddress & ip) :
        ResourceRecord(ip.IsIPv6() ? QType::AAAA : QType::A, qName), mIPAddress(ip)
    {}

protected:
    bool WriteData(RecordWriter & out) const override;

private:
    const chip::Inet::IPAddress mIPAddress;
};

} // namespace Minimal

} // namespace mdns
