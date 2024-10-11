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

#include <array>
#include <string.h>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/core/TLVTags.h>
#include <lib/format/FlatTree.h>

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

TEST(TestFlatTree, TestFlatTreeFind)
{
    EXPECT_STREQ(FindEntry(tree, 0, ByTag(ContextTag(1)))->data.name, "hello");
    EXPECT_STREQ(FindEntry(tree, 0, ByTag(ContextTag(2)))->data.name, "world");
    EXPECT_EQ(FindEntry(tree, 0, ByTag(ContextTag(3))), nullptr);

    EXPECT_EQ(FindEntry(tree, 0, ByName("hello"))->data.tag, ContextTag(1));
    EXPECT_EQ(FindEntry(tree, 0, ByName("world"))->data.tag, ContextTag(2));
    EXPECT_EQ(FindEntry(tree, 0, ByName("foo")), nullptr);

    EXPECT_EQ(FindEntry(tree, 1, ByTag(ContextTag(1))), nullptr);
    EXPECT_STREQ(FindEntry(tree, 1, ByTag(ProfileTag(234, 2)))->data.name, "b");
    EXPECT_STREQ(FindEntry(tree, 1, ByTag(ProfileTag(345, 3)))->data.name, "c");
    EXPECT_EQ(FindEntry(tree, 1, ByTag(AnonymousTag())), nullptr);

    EXPECT_EQ(FindEntry(tree, 2, ByTag(ContextTag(1))), nullptr);
    EXPECT_STREQ(FindEntry(tree, 2, ByTag(AnonymousTag()))->data.name, "foo");

    // out of array
    EXPECT_EQ(FindEntry(tree, 3, ByTag(AnonymousTag())), nullptr);
    EXPECT_EQ(FindEntry(tree, 100, ByTag(AnonymousTag())), nullptr);
    EXPECT_EQ(FindEntry(tree, 1000, ByTag(AnonymousTag())), nullptr);
    EXPECT_EQ(FindEntry(tree, 9999999, ByTag(AnonymousTag())), nullptr);
}
} // namespace
