/*
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
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
#include "pw_unit_test/framework.h"
#include <pw_unit_test/framework.h>

#include <lib/core/DataModelTypes.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/SpanSearchValue.h>

namespace {

using namespace chip;

struct ClusterData
{
    const ClusterId id;
    const char * name;
};

struct EndpointData
{
    const EndpointId id;

    const char * name;
    Span<const ClusterData> serverClusters;
    Span<const ClusterData> clientClusters;
};

struct EndpointItemsWrapper
{
    Span<const EndpointData> data;

    template <size_t N>
    EndpointItemsWrapper(EndpointData (&d)[N]) : data(d)
    {}
};

const ClusterData gClusterList1[] = {
    {
        .id   = 100,
        .name = "one hundred",
    },
    {
        .id   = 200,
        .name = "two hundred",
    },
};

const ClusterData gClusterList2[] = {
    {
        .id   = 1,
        .name = "just one",
    },
};

EndpointData gEndpointDataItems[] = {
    {
        .id             = 123,
        .name           = "foo",
        .serverClusters = Span<const ClusterData>(gClusterList1),
        .clientClusters = Span<const ClusterData>(gClusterList2),
    },
    {
        .id             = 456,
        .name           = "bar",
        .serverClusters = Span<const ClusterData>(gClusterList2),
        .clientClusters = Span<const ClusterData>(),
    },
    {
        .id             = 1000,
        .name           = "Empty",
        .serverClusters = Span<const ClusterData>(),
        .clientClusters = Span<const ClusterData>(),
    },
};

/// search index definitions
struct ByEndpoint
{
    using Key  = EndpointId;
    using Type = const EndpointData;
    static Span<Type> GetSpan(EndpointItemsWrapper & data) { return data.data; }
    static bool HasKey(const Key & id, const Type & instance) { return instance.id == id; }
};

struct ByServerCluster
{
    using Key  = ClusterId;
    using Type = const ClusterData;
    static Span<Type> GetSpan(const EndpointData & data) { return data.serverClusters; }
    static bool HasKey(const Key & id, const Type & instance) { return instance.id == id; }
};

struct ByClientCluster
{
    using Key  = ClusterId;
    using Type = const ClusterData;
    static Span<Type> GetSpan(const EndpointData & data) { return data.clientClusters; }
    static bool HasKey(const Key & id, const Type & instance) { return instance.id == id; }
};

} // namespace

TEST(TestSpanSearchValue, TestFunctionality)
{
    EndpointItemsWrapper wrapper(gEndpointDataItems);
    SpanSearchValue<EndpointItemsWrapper> tree(&wrapper);

    EXPECT_EQ(tree.Value(), &wrapper); // value getting to start matches

    // search first items
    {
        unsigned hint1 = 0;
        auto ep        = tree.First<ByEndpoint>(hint1);

        unsigned hint2 = 0;
        auto cl        = ep.First<ByServerCluster>(hint2);

        ASSERT_NE(cl.Value(), nullptr);
        EXPECT_EQ(cl.Value()->id, 100u);
        EXPECT_STREQ(cl.Value()->name, "one hundred");
    }

    // one level search, with hint
    {
        unsigned hint = 0;
        ASSERT_NE(tree.Find<ByEndpoint>(123, hint).Value(), nullptr);
        ASSERT_STREQ(tree.Find<ByEndpoint>(123, hint).Value()->name, "foo");
        EXPECT_EQ(hint, 0u);

        ASSERT_NE(tree.Find<ByEndpoint>(456, hint).Value(), nullptr);
        EXPECT_EQ(hint, 1u);
        EXPECT_STREQ(tree.Find<ByEndpoint>(456, hint).Value()->name, "bar");
        EXPECT_EQ(hint, 1u);

        // hint is ignored here
        EXPECT_STREQ(tree.Find<ByEndpoint>(123, hint).Value()->name, "foo");
        EXPECT_EQ(hint, 0u);

        EXPECT_STREQ(tree.Find<ByEndpoint>(1000, hint).Value()->name, "Empty");
        EXPECT_EQ(hint, 2u);

        // Invalid searches
        EXPECT_EQ(tree.Find<ByEndpoint>(12345, hint).Value(), nullptr);
        EXPECT_EQ(tree.Find<ByEndpoint>(0, hint).Value(), nullptr);
    }

    // searches for "next"
    {
        unsigned hint = 0;
        auto next     = tree.Next<ByEndpoint>(123, hint);

        ASSERT_NE(next.Value(), nullptr);
        EXPECT_EQ(hint, 1u);
        EXPECT_EQ(next.Value()->id, 456u);
        EXPECT_STREQ(next.Value()->name, "bar");

        next = tree.Next<ByEndpoint>(456, hint);
        ASSERT_NE(next.Value(), nullptr);
        EXPECT_EQ(hint, 2u);
        EXPECT_EQ(next.Value()->id, 1000u);
        EXPECT_STREQ(next.Value()->name, "Empty");

        /// search at the end
        next = tree.Next<ByEndpoint>(1000, hint);
        EXPECT_EQ(next.Value(), nullptr);

        // null value preserves the failure
        unsigned clusterHint = 0;
        auto sub_item        = next.Find<ByServerCluster>(123, clusterHint);
        EXPECT_EQ(sub_item.Value(), nullptr);
    }
}
