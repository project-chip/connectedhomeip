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
#include <lib/mdns/minimal/RecordData.h>

#include <string>
#include <vector>

#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace std;
using namespace chip;
using namespace mdns::Minimal;

void SrvRecordSimpleParsing(nlTestSuite * inSuite, void * inContext)
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

    NL_TEST_ASSERT(inSuite, srv.Parse(data, packet));
    NL_TEST_ASSERT(inSuite, srv.GetPriority() == 12);
    NL_TEST_ASSERT(inSuite, srv.GetWeight() == 3);
    NL_TEST_ASSERT(inSuite, srv.GetPort() == 0x1234);

    // name can be read several times
    for (int i = 0; i < 3; i++)
    {
        SerializedQNameIterator name = srv.GetName();

        NL_TEST_ASSERT(inSuite, name.Next());
        NL_TEST_ASSERT(inSuite, strcmp(name.Value(), "some") == 0);
        NL_TEST_ASSERT(inSuite, name.Next());
        NL_TEST_ASSERT(inSuite, strcmp(name.Value(), "test") == 0);
        NL_TEST_ASSERT(inSuite, name.Next());
        NL_TEST_ASSERT(inSuite, strcmp(name.Value(), "local") == 0);
        NL_TEST_ASSERT(inSuite, name.Next() == false);
    }
}

void SrvWithPtrRecord(nlTestSuite * inSuite, void * inContext)
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

    NL_TEST_ASSERT(inSuite, srv.Parse(data, packet));
    NL_TEST_ASSERT(inSuite, srv.GetPriority() == 12);
    NL_TEST_ASSERT(inSuite, srv.GetWeight() == 3);
    NL_TEST_ASSERT(inSuite, srv.GetPort() == 0x1234);

    // name can be read several times
    for (int i = 0; i < 3; i++)
    {
        SerializedQNameIterator name = srv.GetName();

        NL_TEST_ASSERT(inSuite, name.Next());
        NL_TEST_ASSERT(inSuite, strcmp(name.Value(), "foo") == 0);
        NL_TEST_ASSERT(inSuite, name.Next());
        NL_TEST_ASSERT(inSuite, strcmp(name.Value(), "some") == 0);
        NL_TEST_ASSERT(inSuite, name.Next());
        NL_TEST_ASSERT(inSuite, strcmp(name.Value(), "test") == 0);
        NL_TEST_ASSERT(inSuite, name.Next() == false);
    }
}

#if INET_CONFIG_ENABLE_IPV4
void ARecordParsing(nlTestSuite * inSuite, void * inContext)
{
    const uint8_t record[] = {
        10,
        11,
        12,
        13,
    };

    Inet::IPAddress addr;
    Inet::IPAddress expected;

    NL_TEST_ASSERT(inSuite, ParseARecord(BytesRange(record, record + sizeof(record)), &addr));
    NL_TEST_ASSERT(inSuite, Inet::IPAddress::FromString("10.11.12.13", expected));
    NL_TEST_ASSERT(inSuite, addr == expected);
}
#endif // INET_CONFIG_ENABLE_IPV4

void AAAARecordParsing(nlTestSuite * inSuite, void * inContext)
{
    const uint8_t record[] = {
        0x12, 0x23, 0x00, 0x00, //
        0x00, 0x00, 0x00, 0x00, //
        0x00, 0x00, 0x00, 0x00, //
        0x34, 0x56, 0x78, 0x9a  //
    };

    Inet::IPAddress addr;
    Inet::IPAddress expected;

    NL_TEST_ASSERT(inSuite, ParseAAAARecord(BytesRange(record, record + sizeof(record)), &addr));
    NL_TEST_ASSERT(inSuite, Inet::IPAddress::FromString("1223::3456:789A", expected));
    NL_TEST_ASSERT(inSuite, addr == expected);
}

void PtrRecordSimpleParsing(nlTestSuite * inSuite, void * inContext)
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

    NL_TEST_ASSERT(inSuite, ParsePtrRecord(data, packet, &name));
    NL_TEST_ASSERT(inSuite, name.Next());
    NL_TEST_ASSERT(inSuite, strcmp(name.Value(), "some") == 0);
    NL_TEST_ASSERT(inSuite, name.Next());
    NL_TEST_ASSERT(inSuite, strcmp(name.Value(), "test") == 0);
    NL_TEST_ASSERT(inSuite, name.Next());
    NL_TEST_ASSERT(inSuite, strcmp(name.Value(), "local") == 0);
    NL_TEST_ASSERT(inSuite, name.Next() == false);
}

void PtrRecordComplexParsing(nlTestSuite * inSuite, void * inContext)
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

    NL_TEST_ASSERT(inSuite, ParsePtrRecord(data, packet, &name));
    NL_TEST_ASSERT(inSuite, name.Next());
    NL_TEST_ASSERT(inSuite, strcmp(name.Value(), "foo") == 0);
    NL_TEST_ASSERT(inSuite, name.Next());
    NL_TEST_ASSERT(inSuite, strcmp(name.Value(), "bar") == 0);
    NL_TEST_ASSERT(inSuite, name.Next());
    NL_TEST_ASSERT(inSuite, strcmp(name.Value(), "baz") == 0);
    NL_TEST_ASSERT(inSuite, name.Next());
    NL_TEST_ASSERT(inSuite, strcmp(name.Value(), "some") == 0);
    NL_TEST_ASSERT(inSuite, name.Next());
    NL_TEST_ASSERT(inSuite, strcmp(name.Value(), "test") == 0);
    NL_TEST_ASSERT(inSuite, name.Next() == false);
}

class TxtRecordAccumulator : public TxtRecordDelegate
{
public:
    using DataType = vector<pair<string, string>>;

    void OnRecord(const BytesRange & name, const BytesRange & value) override
    {
        mData.push_back(make_pair(AsString(name), AsString(value)));
    }

    DataType & Data() { return mData; }
    const DataType & Data() const { return mData; }

private:
    DataType mData;

    static string AsString(const BytesRange & range)
    {
        return string(reinterpret_cast<const char *>(range.Start()), reinterpret_cast<const char *>(range.End()));
    }
};

void TxtRecord(nlTestSuite * inSuite, void * inContext)
{
    const uint8_t record[] = {
        4, 's', 'o', 'm', 'e',                // some
        7, 'f', 'o', 'o', '=', 'b', 'a', 'r', // foo=bar
        5, 'x', '=', 'y', '=', 'z',           // x=y=z
        2, 'a', '=',                          // a=
    };

    TxtRecordAccumulator accumulator;

    NL_TEST_ASSERT(inSuite, ParseTxtRecord(BytesRange(record, record + sizeof(record)), &accumulator));
    NL_TEST_ASSERT(inSuite, accumulator.Data().size() == 4);
    NL_TEST_ASSERT(inSuite, (accumulator.Data()[0] == make_pair<std::string, std::string>("some", "")));
    NL_TEST_ASSERT(inSuite, (accumulator.Data()[1] == make_pair<std::string, std::string>("foo", "bar")));
    NL_TEST_ASSERT(inSuite, (accumulator.Data()[2] == make_pair<std::string, std::string>("x", "y=z")));
    NL_TEST_ASSERT(inSuite, (accumulator.Data()[3] == make_pair<std::string, std::string>("a", "")));
}

const nlTest sTests[] = {
    NL_TEST_DEF("SrvRecordSimpleParsing", SrvRecordSimpleParsing), //
    NL_TEST_DEF("SrvWithPtrRecord", SrvWithPtrRecord),             //
#if INET_CONFIG_ENABLE_IPV4
    NL_TEST_DEF("ARecordParsing", ARecordParsing),                   //
#endif                                                               // INET_CONFIG_ENABLE_IPV4
    NL_TEST_DEF("AAAARecordParsing", AAAARecordParsing),             //
    NL_TEST_DEF("PtrRecordSimpleParsing", PtrRecordSimpleParsing),   //
    NL_TEST_DEF("PtrRecordComplexParsing", PtrRecordComplexParsing), //
    NL_TEST_DEF("TxtRecord", TxtRecord),                             //
    NL_TEST_SENTINEL()                                               //
};

} // namespace

int TestRecordData(void)
{
    nlTestSuite theSuite = { "RecordData", sTests, nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestRecordData)
