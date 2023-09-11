/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lib/dnssd/minimal_mdns/Parser.h>

namespace MdnsExample {

/// Prints out the contents of a MDNS packet
/// useful for visual debug of data
class PacketReporter : public mdns::Minimal::ParserDelegate
{
public:
    PacketReporter(const char * prefix, const mdns::Minimal::BytesRange & packet) : mPrefix(prefix), mPacketRange(packet) {}

    void OnHeader(mdns::Minimal::ConstHeaderRef & header) override;
    void OnQuery(const mdns::Minimal::QueryData & data) override;
    void OnResource(mdns::Minimal::ResourceType type, const mdns::Minimal::ResourceData & data) override;

private:
    const char * mPrefix;
    mdns::Minimal::BytesRange mPacketRange;
};

} // namespace MdnsExample
