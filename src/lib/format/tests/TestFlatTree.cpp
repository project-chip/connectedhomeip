/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <lib/format/flat_tree.h>

#include <lib/core/TLVTags.h>
#include <lib/support/UnitTestRegistration.h>

#include <array>

#include <string.h>

#include <nlunit-test.h>


namespace {

using namespace chip::FlatTree;
using namespace chip::TLV;

Entry<Tag, const char *> node1[] = {
    {ContextTag(1), "hello"},
    {ContextTag(2), "world"},
};

Entry<Tag, const char *> node2[] = {
    {ProfileTag(123, 1), "a"},
    {ProfileTag(234, 2), "b"},
    {ProfileTag(345, 3), "c"},
};

Entry<Tag, const char *> node3[] = {
    {AnonymousTag(), "foo"},
};

#define _ENTRY(n) {sizeof(n)/sizeof(n[0]), n}

std::array<Node<Tag, const char *>, 3> tree = {{
        _ENTRY(node1),
        _ENTRY(node2),
        _ENTRY(node3),
}};

// DO NOT SUBMIT: tests for layouts of data
struct Data {
    uint32_t id;
    const char *name;
    uint8_t  tlv_type;
    uint8_t  flags;
};


void TestFlatTreeFind(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, strcmp(FindEntry(tree, 0, ContextTag(1))->value, "hello") == 0);
    NL_TEST_ASSERT(inSuite, strcmp(FindEntry(tree, 0, ContextTag(2))->value, "world") == 0);
    NL_TEST_ASSERT(inSuite, FindEntry(tree, 0, ContextTag(3)) == nullptr);

    NL_TEST_ASSERT(inSuite, FindEntry(tree, 1, ContextTag(1)) == nullptr);
    NL_TEST_ASSERT(inSuite, strcmp(FindEntry(tree, 1, ProfileTag(234, 2))->value, "b") == 0);
    NL_TEST_ASSERT(inSuite, strcmp(FindEntry(tree, 1, ProfileTag(345, 3))->value, "c") == 0);
    NL_TEST_ASSERT(inSuite, FindEntry(tree, 1, AnonymousTag()) == nullptr);

    NL_TEST_ASSERT(inSuite, FindEntry(tree, 2, ContextTag(1)) == nullptr);
    NL_TEST_ASSERT(inSuite, strcmp(FindEntry(tree, 2, AnonymousTag())->value, "foo") == 0);

    // out of array
    NL_TEST_ASSERT(inSuite, FindEntry(tree, 3, AnonymousTag()) == nullptr);
    NL_TEST_ASSERT(inSuite, FindEntry(tree, 100, AnonymousTag()) == nullptr);
    NL_TEST_ASSERT(inSuite, FindEntry(tree, 1000, AnonymousTag()) == nullptr);
    NL_TEST_ASSERT(inSuite, FindEntry(tree, 9999999, AnonymousTag()) == nullptr);
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestFlatTreeFind", TestFlatTreeFind), //
    NL_TEST_SENTINEL()                                   //
};

} // namespace

int TestFlatTree()
{
    nlTestSuite theSuite = { "FlatTree", sTests, nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestFlatTree)
