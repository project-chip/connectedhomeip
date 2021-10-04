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
