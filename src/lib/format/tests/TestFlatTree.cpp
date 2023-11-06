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
#include <lib/format/FlatTree.h>

#include <lib/core/TLVTags.h>
#include <lib/support/UnitTestRegistration.h>

#include <array>

#include <string.h>

#include <nlunit-test.h>

namespace {

using namespace chip::FlatTree;
using namespace chip::TLV;

struct NamedTag
{
    Tag tag;
    const char * name;
};

Entry<NamedTag> node1[] = {
    { { ContextTag(1), "hello" } },
    { { ContextTag(2), "world" } },
};

Entry<NamedTag> node2[] = {
    { { ProfileTag(123, 1), "a" } },
    { { ProfileTag(234, 2), "b" } },
    { { ProfileTag(345, 3), "c" } },
};

Entry<NamedTag> node3[] = {
    { { AnonymousTag(), "foo" } },
};

#define _ENTRY(n)                                                                                                                  \
    {                                                                                                                              \
        sizeof(n) / sizeof(n[0]), n                                                                                                \
    }

std::array<Node<NamedTag>, 3> tree = { {
    _ENTRY(node1),
    _ENTRY(node2),
    _ENTRY(node3),
} };

class ByTag
{
public:
    constexpr ByTag(Tag tag) : mTag(tag) {}
    bool operator()(const NamedTag & item) { return item.tag == mTag; }

private:
    const Tag mTag;
};

class ByName
{
public:
    constexpr ByName(const char * name) : mName(name) {}
    bool operator()(const NamedTag & item) { return strcmp(item.name, mName) == 0; }

private:
    const char * mName;
};

void TestFlatTreeFind(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, strcmp(FindEntry(tree, 0, ByTag(ContextTag(1)))->data.name, "hello") == 0);
    NL_TEST_ASSERT(inSuite, strcmp(FindEntry(tree, 0, ByTag(ContextTag(2)))->data.name, "world") == 0);
    NL_TEST_ASSERT(inSuite, FindEntry(tree, 0, ByTag(ContextTag(3))) == nullptr);

    NL_TEST_ASSERT(inSuite, FindEntry(tree, 0, ByName("hello"))->data.tag == ContextTag(1));
    NL_TEST_ASSERT(inSuite, FindEntry(tree, 0, ByName("world"))->data.tag == ContextTag(2));
    NL_TEST_ASSERT(inSuite, FindEntry(tree, 0, ByName("foo")) == nullptr);

    NL_TEST_ASSERT(inSuite, FindEntry(tree, 1, ByTag(ContextTag(1))) == nullptr);
    NL_TEST_ASSERT(inSuite, strcmp(FindEntry(tree, 1, ByTag(ProfileTag(234, 2)))->data.name, "b") == 0);
    NL_TEST_ASSERT(inSuite, strcmp(FindEntry(tree, 1, ByTag(ProfileTag(345, 3)))->data.name, "c") == 0);
    NL_TEST_ASSERT(inSuite, FindEntry(tree, 1, ByTag(AnonymousTag())) == nullptr);

    NL_TEST_ASSERT(inSuite, FindEntry(tree, 2, ByTag(ContextTag(1))) == nullptr);
    NL_TEST_ASSERT(inSuite, strcmp(FindEntry(tree, 2, ByTag(AnonymousTag()))->data.name, "foo") == 0);

    // out of array
    NL_TEST_ASSERT(inSuite, FindEntry(tree, 3, ByTag(AnonymousTag())) == nullptr);
    NL_TEST_ASSERT(inSuite, FindEntry(tree, 100, ByTag(AnonymousTag())) == nullptr);
    NL_TEST_ASSERT(inSuite, FindEntry(tree, 1000, ByTag(AnonymousTag())) == nullptr);
    NL_TEST_ASSERT(inSuite, FindEntry(tree, 9999999, ByTag(AnonymousTag())) == nullptr);
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestFlatTreeFind", TestFlatTreeFind), //
    NL_TEST_SENTINEL()                                 //
};

} // namespace

int TestFlatTree()
{
    nlTestSuite theSuite = { "FlatTree", sTests, nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestFlatTree)
