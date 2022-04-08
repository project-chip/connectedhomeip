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
#include <lib/dnssd/minimal_mdns/QueryReplyFilter.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace chip;
using namespace mdns::Minimal;

const QNamePart kName1[] = { "some", "local" };
const QNamePart kName2[] = { "something", "else" };
const QNamePart kName3[] = { "more", "specific", "some", "local" };
const QNamePart kName4[] = { "this", "is", "a", "test" };

template <size_t N>
QueryData buildQueryData(QType qType, QClass qClass, const uint8_t (&query)[N])
{
    return QueryData(qType, qClass, false, query, BytesRange(query, query + N));
}

void TestQueryReplyFilter(nlTestSuite * inSuite, void * inContext)
{
    const uint8_t query[] = {
        4, 's', 'o', 'm', 'e',      //
        5, 'l', 'o', 'c', 'a', 'l', //
        0                           //
    };

    // sanity test that the serialized qname was build correctly
    NL_TEST_ASSERT(inSuite, SerializedQNameIterator(BytesRange(query, query + sizeof(query)), query) == FullQName(kName1));
    NL_TEST_ASSERT(inSuite, SerializedQNameIterator(BytesRange(query, query + sizeof(query)), query) != FullQName(kName2));
    NL_TEST_ASSERT(inSuite, SerializedQNameIterator(BytesRange(query, query + sizeof(query)), query) != FullQName(kName3));
    NL_TEST_ASSERT(inSuite, SerializedQNameIterator(BytesRange(query, query + sizeof(query)), query) != FullQName(kName4));

    // Acceptable cases
    NL_TEST_ASSERT(
        inSuite, QueryReplyFilter(buildQueryData(QType::ANY, QClass::ANY, query)).Accept(QType::A, QClass::IN, FullQName(kName1)));
    NL_TEST_ASSERT(inSuite,
                   QueryReplyFilter(buildQueryData(QType::A, QClass::ANY, query)).Accept(QType::A, QClass::IN, FullQName(kName1)));

    NL_TEST_ASSERT(inSuite,
                   QueryReplyFilter(buildQueryData(QType::ANY, QClass::IN, query)).Accept(QType::A, QClass::IN, FullQName(kName1)));

    NL_TEST_ASSERT(inSuite,
                   QueryReplyFilter(buildQueryData(QType::A, QClass::IN, query)).Accept(QType::A, QClass::IN, FullQName(kName1)));

    // Reject cases
    NL_TEST_ASSERT(
        inSuite, !QueryReplyFilter(buildQueryData(QType::ANY, QClass::ANY, query)).Accept(QType::A, QClass::IN, FullQName(kName2)));

    NL_TEST_ASSERT(
        inSuite, !QueryReplyFilter(buildQueryData(QType::ANY, QClass::ANY, query)).Accept(QType::A, QClass::IN, FullQName(kName3)));

    NL_TEST_ASSERT(
        inSuite, !QueryReplyFilter(buildQueryData(QType::ANY, QClass::ANY, query)).Accept(QType::A, QClass::IN, FullQName(kName4)));

    NL_TEST_ASSERT(
        inSuite,
        !QueryReplyFilter(buildQueryData(QType::AAAA, QClass::ANY, query)).Accept(QType::A, QClass::IN, FullQName(kName1)));

    NL_TEST_ASSERT(
        inSuite, !QueryReplyFilter(buildQueryData(QType::SRV, QClass::IN, query)).Accept(QType::A, QClass::IN, FullQName(kName1)));

    NL_TEST_ASSERT(
        inSuite, !QueryReplyFilter(buildQueryData(QType::PTR, QClass::ANY, query)).Accept(QType::A, QClass::IN, FullQName(kName1)));
}

void TestLongerQueryPath(nlTestSuite * inSuite, void * inContext)
{
    const uint8_t query[] = {
        4, 'm', 'o', 'r', 'e',                     //
        8, 's', 'p', 'e', 'c', 'i', 'f', 'i', 'c', //
        4, 's', 'o', 'm', 'e',                     //
        5, 'l', 'o', 'c', 'a', 'l',                //
        0                                          //
    };

    // sanity test that the serialized qname was build correctly
    NL_TEST_ASSERT(inSuite, SerializedQNameIterator(BytesRange(query, query + sizeof(query)), query) != FullQName(kName1));
    NL_TEST_ASSERT(inSuite, SerializedQNameIterator(BytesRange(query, query + sizeof(query)), query) != FullQName(kName2));
    NL_TEST_ASSERT(inSuite, SerializedQNameIterator(BytesRange(query, query + sizeof(query)), query) == FullQName(kName3));
    NL_TEST_ASSERT(inSuite, SerializedQNameIterator(BytesRange(query, query + sizeof(query)), query) != FullQName(kName4));

    NL_TEST_ASSERT(
        inSuite, !QueryReplyFilter(buildQueryData(QType::ANY, QClass::ANY, query)).Accept(QType::A, QClass::IN, FullQName(kName1)));
    NL_TEST_ASSERT(
        inSuite, QueryReplyFilter(buildQueryData(QType::ANY, QClass::ANY, query)).Accept(QType::A, QClass::IN, FullQName(kName3)));
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestQueryReplyFilter", TestQueryReplyFilter), //
    NL_TEST_DEF("TestLongerQueryPath", TestLongerQueryPath),   //
    NL_TEST_SENTINEL()                                         //
};

} // namespace

int TestQueryReplyFilter(void)
{
    nlTestSuite theSuite = { "QueryReplyFilter", sTests, nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestQueryReplyFilter)
