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

#include <lib/dnssd/minimal_mdns/RecordData.h>

#include <string>
#include <vector>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>

namespace {

using namespace chip;
using namespace mdns::Minimal;

TEST(TestRecordData, SrvRecordSimpleParsing)
{
    const uint8_t record[] = {
        0,    12,                       // Priority
        0,    3,                        // weight
        0x12, 0x34,                     // port
        4,    's',  'o', 'm', 'e',      // QNAME part: some
        4,    't',  'e', 's', 't',      // QNAME part: test
        5,    'l',  'o', 'c', 'a', 'l', // QNAME part: local
        0,                              // QNAME ends
    };

    BytesRange packet(record, record + sizeof(record));
    BytesRange data(record, record + sizeof(record));

    SrvRecord srv;

    EXPECT_TRUE(srv.Parse(data, packet));
    EXPECT_EQ(srv.GetPriority(), 12);
    EXPECT_EQ(srv.GetWeight(), 3);
    EXPECT_EQ(srv.GetPort(), 0x1234);

    // name can be read several times
    for (int i = 0; i < 3; i++)
    {
        SerializedQNameIterator name = srv.GetName();

        EXPECT_TRUE(name.Next());
        EXPECT_STREQ(name.Value(), "some");
        EXPECT_TRUE(name.Next());
        EXPECT_STREQ(name.Value(), "test");
        EXPECT_TRUE(name.Next());
        EXPECT_STREQ(name.Value(), "local");
        EXPECT_EQ(name.Next(), false);
    }
}

TEST(TestRecordData, SrvWithPtrRecord)
{
    const uint8_t record[] = {
        'x',  'y',  'z',           // dummy data (3 bytes)
        4,    's',  'o', 'm', 'e', // QNAME part: some
        4,    't',  'e', 's', 't', // QNAME part: test
        0,                         // QNAME ends

        0,    12,             // Priority
        0,    3,              // weight
        0x12, 0x34,           // port
        3,    'f',  'o', 'o', // QNAME part: foo
        0xC0, 0x03,           // PTR
    };

    BytesRange packet(record, record + sizeof(record));
    BytesRange data(record + 14, record + sizeof(record));

    SrvRecord srv;

    EXPECT_TRUE(srv.Parse(data, packet));
    EXPECT_EQ(srv.GetPriority(), 12);
    EXPECT_EQ(srv.GetWeight(), 3);
    EXPECT_EQ(srv.GetPort(), 0x1234);

    // name can be read several times
    for (int i = 0; i < 3; i++)
    {
        SerializedQNameIterator name = srv.GetName();

        EXPECT_TRUE(name.Next());
        EXPECT_STREQ(name.Value(), "foo");
        EXPECT_TRUE(name.Next());
        EXPECT_STREQ(name.Value(), "some");
        EXPECT_TRUE(name.Next());
        EXPECT_STREQ(name.Value(), "test");
        EXPECT_EQ(name.Next(), false);
    }
}

TEST(TestRecordData, ARecordParsing)
{
    const uint8_t record[] = {
        10,
        11,
        12,
        13,
    };

    Inet::IPAddress addr;

#if INET_CONFIG_ENABLE_IPV4
    Inet::IPAddress expected;

    EXPECT_TRUE(ParseARecord(BytesRange(record, record + sizeof(record)), &addr));
    EXPECT_TRUE(Inet::IPAddress::FromString("10.11.12.13", expected));
    EXPECT_EQ(addr, expected);
#else
    EXPECT_FALSE(ParseARecord(BytesRange(record, record + sizeof(record)), &addr));
#endif // INET_CONFIG_ENABLE_IPV4
}

TEST(TestRecordData, AAAARecordParsing)
{
    const uint8_t record[] = {
        0x12, 0x23, 0x00, 0x00, //
        0x00, 0x00, 0x00, 0x00, //
        0x00, 0x00, 0x00, 0x00, //
        0x34, 0x56, 0x78, 0x9a  //
    };

    Inet::IPAddress addr;
    Inet::IPAddress expected;

    EXPECT_TRUE(ParseAAAARecord(BytesRange(record, record + sizeof(record)), &addr));
    EXPECT_TRUE(Inet::IPAddress::FromString("1223::3456:789A", expected));
    EXPECT_EQ(addr, expected);
}

TEST(TestRecordData, PtrRecordSimpleParsing)
{
    const uint8_t record[] = {
        4, 's', 'o', 'm', 'e',      // QNAME part: some
        4, 't', 'e', 's', 't',      // QNAME part: test
        5, 'l', 'o', 'c', 'a', 'l', // QNAME part: local
        0,                          // QNAME ends
    };

    BytesRange packet(record, record + sizeof(record));
    BytesRange data(record, record + sizeof(record));

    SerializedQNameIterator name;

    EXPECT_TRUE(ParsePtrRecord(data, packet, &name));
    EXPECT_TRUE(name.Next());
    EXPECT_STREQ(name.Value(), "some");
    EXPECT_TRUE(name.Next());
    EXPECT_STREQ(name.Value(), "test");
    EXPECT_TRUE(name.Next());
    EXPECT_STREQ(name.Value(), "local");
    EXPECT_EQ(name.Next(), false);
}

TEST(TestRecordData, PtrRecordComplexParsing)
{
    const uint8_t record[] = {
        'x',  'y',  'z',           // dummy data (3 bytes)
        4,    's',  'o', 'm', 'e', // QNAME part: some
        4,    't',  'e', 's', 't', // QNAME part: test
        0,                         // QNAME ends
        3,    'b',  'a', 'r',      // QNAME part: bar
        3,    'b',  'a', 'z',      // QNAME part: baz
        0xC0, 0x03,                // PTR
        3,    'f',  'o', 'o',      // QNAME part: foo
        0xC0, 0x0E,                // PTR
    };

    BytesRange packet(record, record + sizeof(record));
    BytesRange data(record + 24, record + sizeof(record));
    SerializedQNameIterator name;

    EXPECT_TRUE(ParsePtrRecord(data, packet, &name));
    EXPECT_TRUE(name.Next());
    EXPECT_STREQ(name.Value(), "foo");
    EXPECT_TRUE(name.Next());
    EXPECT_STREQ(name.Value(), "bar");
    EXPECT_TRUE(name.Next());
    EXPECT_STREQ(name.Value(), "baz");
    EXPECT_TRUE(name.Next());
    EXPECT_STREQ(name.Value(), "some");
    EXPECT_TRUE(name.Next());
    EXPECT_STREQ(name.Value(), "test");
    EXPECT_EQ(name.Next(), false);
}

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

TEST(TestRecordData, TxtRecord)
{
    const uint8_t record[] = {
        4, 's', 'o', 'm', 'e',                // some
        7, 'f', 'o', 'o', '=', 'b', 'a', 'r', // foo=bar
        5, 'x', '=', 'y', '=', 'z',           // x=y=z
        2, 'a', '=',                          // a=
    };

    TxtRecordAccumulator accumulator;

    EXPECT_TRUE(ParseTxtRecord(BytesRange(record, record + sizeof(record)), &accumulator));
    EXPECT_EQ(accumulator.Data().size(), 4u);
    EXPECT_EQ(accumulator.Data()[0], (std::make_pair<std::string, std::string>("some", "")));
    EXPECT_EQ(accumulator.Data()[1], (std::make_pair<std::string, std::string>("foo", "bar")));
    EXPECT_EQ(accumulator.Data()[2], (std::make_pair<std::string, std::string>("x", "y=z")));
    EXPECT_EQ(accumulator.Data()[3], (std::make_pair<std::string, std::string>("a", "")));
}
} // namespace
