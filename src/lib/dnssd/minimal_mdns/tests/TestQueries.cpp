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
 #include <lib/dnssd/minimal_mdns/QueryReplyFilter.h>
 #include <lib/dnssd/minimal_mdns/core/QName.h>
 #include <lib/dnssd/minimal_mdns/core/tests/QNameStrings.h>

 namespace {

 using namespace chip;
 using namespace mdns::Minimal;

 const auto kTestOperationalName = testing::TestQName<4>({ "1234567898765432-ABCDEFEDCBAABCDE", "_matter", "_tcp", "local" });

 mdns::Minimal::Query query(kTestOperationalName.Full());

 TEST(TestQueries, TestQuery)
 {
     EXPECT_EQ(query.GetClass(), QClass::IN);
     EXPECT_EQ(query.GetType(), QType::ANY);
     EXPECT_EQ(query.IsAnswerViaUnicast(), true);

     EXPECT_EQ(query.SetClass(QClass::ANY).GetClass(), QClass::ANY);
     EXPECT_EQ(query.SetType(QType::AAAA).GetType(), QType::AAAA);
     EXPECT_EQ(query.SetAnswerViaUnicast(false).IsAnswerViaUnicast(), false);
 }
 } // namespace
 