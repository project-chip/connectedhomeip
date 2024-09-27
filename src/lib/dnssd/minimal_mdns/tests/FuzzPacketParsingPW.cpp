#include <cstddef>
#include <cstdint>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <lib/dnssd/minimal_mdns/Parser.h>
#include <lib/dnssd/minimal_mdns/RecordData.h>

namespace {

using namespace fuzztest;

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

void PacketParserFuzz(const std::vector<std::uint8_t> & bytes)
{
    BytesRange packet(bytes.data(), bytes.data() + bytes.size());
    FuzzDelegate delegate(packet);

    mdns::Minimal::ParsePacket(packet, &delegate);
}

FUZZ_TEST(MinimalmDNS, PacketParserFuzz).WithDomains(Arbitrary<std::vector<uint8_t>>());

class TxtRecordAccumulator : public TxtRecordDelegate
{
public:
    using DataType = std::vector<std::pair<std::string, std::string>>;

    void OnRecord(const BytesRange & name, const BytesRange & value) override
    {
        mData.push_back(std::make_pair(AsString(name), AsString(value)));
    }

    DataType & Data() { return mData; }
    const DataType & Data() const { return mData; }

private:
    DataType mData;

    static std::string AsString(const BytesRange & range)
    {
        return std::string(reinterpret_cast<const char *>(range.Start()), reinterpret_cast<const char *>(range.End()));
    }
};

// The Property Function
void TxtResponderFuzz(const std::vector<std::uint8_t> & aRecord)
{

    bool equal_sign_present = false;
    auto equal_sign_pos     = aRecord.end();

    // This test is only giving a set of values, it can be gives more
    std::vector<uint8_t> prefixedRecord{ static_cast<std::uint8_t>(aRecord.size()) };

    prefixedRecord.insert(prefixedRecord.end(), aRecord.begin(), aRecord.end());

    TxtRecordAccumulator accumulator;

    // The Function under Test, Check that the function does not Crash
    ParseTxtRecord(BytesRange(prefixedRecord.data(), (&prefixedRecord.back() + 1)), &accumulator);

    for (auto it = aRecord.begin(); it != aRecord.end(); it++)
    {
        // if this is first `=` found in the fuzzed record
        if ('=' == static_cast<char>(*it) && false == equal_sign_present)
        {
            equal_sign_present = true;
            equal_sign_pos     = it;
        }
    }

    // The Fuzzed Input (record) needs to have at least two characters in order for ParseTxtRecord to do something
    if (aRecord.size() > 1)
    {
        if (true == equal_sign_present)
        {
            std::string input_record_value(equal_sign_pos + 1, aRecord.end());
            EXPECT_EQ(accumulator.Data().at(0).second, input_record_value);
        }
    }
}

FUZZ_TEST(MinimalmDNS, TxtResponderFuzz).WithDomains(Arbitrary<std::vector<std::uint8_t>>().WithMaxSize(254));

} // namespace
