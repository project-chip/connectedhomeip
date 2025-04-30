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

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/minimal_mdns/Query.h>
#include <lib/dnssd/minimal_mdns/QueryBuilder.h>
#include <lib/dnssd/minimal_mdns/core/tests/QNameStrings.h>
#include <lib/dnssd/minimal_mdns/tests/CheckOnlyServer.h>

namespace {

using namespace chip;
using namespace mdns::Minimal;

const auto kTestOperationalName = testing::TestQName<4>({ "1234567898765432-ABCDEFEDCBAABCDE", "_matter", "_tcp", "local" });

class TestQuery : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestQuery, DefinitionAndRename)
{
    mdns::Minimal::Query query(kTestOperationalName.Full());

    EXPECT_EQ(query.GetClass(), QClass::IN);
    EXPECT_EQ(query.GetType(), QType::ANY);
    EXPECT_TRUE(query.IsAnswerViaUnicast());

    query.SetClass(QClass::ANY);
    query.SetType(QType::AAAA);
    query.SetAnswerViaUnicast(false);

    EXPECT_EQ(query.GetClass(), QClass::ANY);
    EXPECT_EQ(query.GetType(), QType::AAAA);
    EXPECT_FALSE(query.IsAnswerViaUnicast());
}
} // namespace
