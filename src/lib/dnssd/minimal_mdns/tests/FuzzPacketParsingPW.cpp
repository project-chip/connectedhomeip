#include <cstddef>
#include <cstdint>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <lib/dnssd/minimal_mdns/Parser.h>
#include <lib/dnssd/minimal_mdns/RecordData.h>

using namespace fuzztest;

namespace {

using namespace chip;
using namespace mdns::Minimal;

class FuzzDelegate : public ParserDelegate
{
public:
    FuzzDelegate(const mdns::Minimal::BytesRange & packet) : mPacketRange(packet) {}
    virtual ~FuzzDelegate() {}

    void OnHeader(ConstHeaderRef & header) override {}
    void OnQuery(const QueryData & data) override {}
    void OnResource(ResourceType type, const ResourceData & data) override
    {
        switch (data.GetType())
        {
        case QType::SRV: {
            mdns::Minimal::SrvRecord srv;
            (void) srv.Parse(data.GetData(), mPacketRange);
            break;
        }
        case QType::A: {
            chip::Inet::IPAddress addr;
            (void) mdns::Minimal::ParseARecord(data.GetData(), &addr);
            break;
        }
        case QType::AAAA: {
            chip::Inet::IPAddress addr;
            (void) mdns::Minimal::ParseAAAARecord(data.GetData(), &addr);
            break;
        }
        case QType::PTR: {
            mdns::Minimal::SerializedQNameIterator name;
            (void) mdns::Minimal::ParsePtrRecord(data.GetData(), mPacketRange, &name);
            break;
        }
        default:
            // nothing to do
            break;
        }
    }

private:
    mdns::Minimal::BytesRange mPacketRange;
};

} // namespace

void PacketParserFuzz(const std::vector<std::uint8_t> & bytes)
{
    BytesRange packet(bytes.data(), bytes.data() + bytes.size());
    FuzzDelegate delegate(packet);

    mdns::Minimal::ParsePacket(packet, &delegate);
}

FUZZ_TEST(MinimalmDNS, PacketParserFuzz).WithDomains(Arbitrary<std::vector<std::uint8_t>>());
