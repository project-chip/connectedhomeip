#include <cstddef>
#include <cstdint>

#include <lib/dnssd/wire/Parser.h>
#include <lib/dnssd/wire/RecordData.h>

namespace {

using namespace chip;
using namespace mdns::Minimal;
using namespace chip::Dnssd;

class FuzzDelegate : public ParserDelegate
{
public:
    FuzzDelegate(const chip::Dnssd::BytesRange & packet) : mPacketRange(packet) {}
    virtual ~FuzzDelegate() {}

    void OnHeader(ConstHeaderRef & header) override {}
    void OnQuery(const QueryData & data) override {}
    void OnResource(ResourceType type, const ResourceData & data) override
    {
        switch (data.GetType())
        {
        case QType::SRV: {
            chip::Dnssd::SrvRecord srv;
            (void) srv.Parse(data.GetData(), mPacketRange);
            break;
        }
        case QType::A: {
            chip::Inet::IPAddress addr;
            (void) chip::Dnssd::ParseARecord(data.GetData(), &addr);
            break;
        }
        case QType::AAAA: {
            chip::Inet::IPAddress addr;
            (void) chip::Dnssd::ParseAAAARecord(data.GetData(), &addr);
            break;
        }
        case QType::PTR: {
            chip::Dnssd::SerializedQNameIterator name;
            (void) chip::Dnssd::ParsePtrRecord(data.GetData(), mPacketRange, &name);
            break;
        }
        default:
            // nothing to do
            break;
        }
    }

private:
    chip::Dnssd::BytesRange mPacketRange;
};

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t * data, size_t len)
{

    BytesRange packet(data, data + len);
    FuzzDelegate delegate(packet);

    chip::Dnssd::ParsePacket(packet, &delegate);

    return 0;
}
