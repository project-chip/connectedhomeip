
/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <pw_unit_test/framework.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>

#include <cstdlib>
#include <optional>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;

namespace {

// Initialize memory as ReadOnlyBufferBuilder may allocate
struct TestAttributeListBuilder : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

} // namespace

TEST_F(TestAttributeListBuilder, Append)
{
    const size_t global_attribute_count = DefaultServerCluster::GlobalAttributes().size();

    constexpr BitFlags<DataModel::AttributeQualityFlags> kNoFlags;

    // Only mandatory attributes
    {
        const AttributeEntry mandatory[] = {
            { 1, kNoFlags, Access::Privilege::kView, std::nullopt },
            { 2, kNoFlags, Access::Privilege::kView, std::nullopt },
        };

        ReadOnlyBufferBuilder<AttributeEntry> builder;
        ASSERT_EQ(AttributeListBuilder(builder).Append(Span(mandatory), {}, {}), CHIP_NO_ERROR);

        ReadOnlyBuffer<AttributeEntry> result = builder.TakeBuffer();
        ASSERT_EQ(result.size(), 2 + global_attribute_count);
        ASSERT_EQ(result[0].attributeId, 1u);
        ASSERT_EQ(result[1].attributeId, 2u);
        ASSERT_EQ(result[2].attributeId, Globals::Attributes::FeatureMap::Id);
    }

    // Only optional attributes
    {
        const AttributeEntry optional1_meta(10, kNoFlags, Access::Privilege::kView, std::nullopt);
        const AttributeEntry optional2_meta(11, kNoFlags, Access::Privilege::kView, std::nullopt);
        const AttributeEntry optional3_meta(12, kNoFlags, Access::Privilege::kView, std::nullopt);

        const AttributeEntry optionalEntries[] = {
            optional1_meta,
            optional2_meta,
            optional3_meta,
        };

        ReadOnlyBufferBuilder<AttributeEntry> builder;
        ASSERT_EQ(AttributeListBuilder(builder).Append({}, Span(optionalEntries), AttributeSet().ForceSet<10>().ForceSet<12>()),
                  CHIP_NO_ERROR);

        ReadOnlyBuffer<AttributeEntry> result = builder.TakeBuffer();
        ASSERT_EQ(result.size(), 2 + global_attribute_count);
        ASSERT_EQ(result[0].attributeId, 10u);
        ASSERT_EQ(result[1].attributeId, 12u);
        ASSERT_EQ(result[2].attributeId, Globals::Attributes::FeatureMap::Id);
    }

    // Mix of mandatory and optional attributes
    {
        const AttributeEntry mandatory[] = {
            { 1, kNoFlags, Access::Privilege::kView, std::nullopt },
        };
        const AttributeEntry optional1_meta(10, kNoFlags, Access::Privilege::kView, std::nullopt);
        const AttributeEntry optional2_meta(11, kNoFlags, Access::Privilege::kView, std::nullopt);

        ReadOnlyBufferBuilder<AttributeEntry> builder;
        const AttributeEntry optionalEntries[] = {
            optional1_meta,
            optional2_meta,
        };
        ASSERT_EQ(AttributeListBuilder(builder).Append(Span(mandatory), Span(optionalEntries), AttributeSet().ForceSet<10>()),
                  CHIP_NO_ERROR);

        ReadOnlyBuffer<AttributeEntry> result = builder.TakeBuffer();
        ASSERT_EQ(result.size(), 2 + global_attribute_count);
        ASSERT_EQ(result[0].attributeId, 1u);
        ASSERT_EQ(result[1].attributeId, 10u);
        ASSERT_EQ(result[2].attributeId, Globals::Attributes::FeatureMap::Id);
    }

    // No attributes
    {
        ReadOnlyBufferBuilder<AttributeEntry> builder;
        ASSERT_EQ(AttributeListBuilder(builder).Append({}, {}, {}), CHIP_NO_ERROR);

        ReadOnlyBuffer<AttributeEntry> result = builder.TakeBuffer();
        ASSERT_EQ(result.size(), global_attribute_count);
        ASSERT_EQ(result[0].attributeId, Globals::Attributes::FeatureMap::Id);
    }
}

TEST_F(TestAttributeListBuilder, AppendWithOptionalAttributeEntry)
{
    const size_t global_attribute_count = DefaultServerCluster::GlobalAttributes().size();

    constexpr BitFlags<DataModel::AttributeQualityFlags> kNoFlags;

    // Only mandatory attributes
    {
        const AttributeEntry mandatory[] = {
            { 1, kNoFlags, Access::Privilege::kView, std::nullopt },
            { 2, kNoFlags, Access::Privilege::kView, std::nullopt },
        };

        ReadOnlyBufferBuilder<AttributeEntry> builder;
        ASSERT_EQ(AttributeListBuilder(builder).Append(Span(mandatory), {}), CHIP_NO_ERROR);

        ReadOnlyBuffer<AttributeEntry> result = builder.TakeBuffer();
        ASSERT_EQ(result.size(), 2 + global_attribute_count);
        ASSERT_EQ(result[0].attributeId, 1u);
        ASSERT_EQ(result[1].attributeId, 2u);
        ASSERT_EQ(result[2].attributeId, Globals::Attributes::FeatureMap::Id);
    }

    // Only optional attributes
    {
        const AttributeEntry optional1_meta(10, kNoFlags, Access::Privilege::kView, std::nullopt);
        const AttributeEntry optional2_meta(11, kNoFlags, Access::Privilege::kView, std::nullopt);
        const AttributeEntry optional3_meta(12, kNoFlags, Access::Privilege::kView, std::nullopt);

        ReadOnlyBufferBuilder<AttributeEntry> builder;
        const AttributeListBuilder::OptionalAttributeEntry optionalEntries[] = {
            { .enabled = true, .metadata = optional1_meta },
            { .enabled = false, .metadata = optional2_meta },
            { .enabled = true, .metadata = optional3_meta },
        };
        ASSERT_EQ(AttributeListBuilder(builder).Append({}, Span(optionalEntries)), CHIP_NO_ERROR);

        ReadOnlyBuffer<AttributeEntry> result = builder.TakeBuffer();
        ASSERT_EQ(result.size(), 2 + global_attribute_count);
        ASSERT_EQ(result[0].attributeId, 10u);
        ASSERT_EQ(result[1].attributeId, 12u);
        ASSERT_EQ(result[2].attributeId, Globals::Attributes::FeatureMap::Id);
    }

    // Mix of mandatory and optional attributes
    {
        const AttributeEntry mandatory[] = {
            { 1, kNoFlags, Access::Privilege::kView, std::nullopt },
        };
        const AttributeEntry optional1_meta(10, kNoFlags, Access::Privilege::kView, std::nullopt);
        const AttributeEntry optional2_meta(11, kNoFlags, Access::Privilege::kView, std::nullopt);

        ReadOnlyBufferBuilder<AttributeEntry> builder;
        const AttributeListBuilder::OptionalAttributeEntry optionalEntries[] = {
            { .enabled = true, .metadata = optional1_meta },
            { .enabled = false, .metadata = optional2_meta },
        };
        ASSERT_EQ(AttributeListBuilder(builder).Append(Span(mandatory), Span(optionalEntries)), CHIP_NO_ERROR);

        ReadOnlyBuffer<AttributeEntry> result = builder.TakeBuffer();
        ASSERT_EQ(result.size(), 2 + global_attribute_count);
        ASSERT_EQ(result[0].attributeId, 1u);
        ASSERT_EQ(result[1].attributeId, 10u);
        ASSERT_EQ(result[2].attributeId, Globals::Attributes::FeatureMap::Id);
    }

    // No attributes
    {
        ReadOnlyBufferBuilder<AttributeEntry> builder;
        ASSERT_EQ(AttributeListBuilder(builder).Append({}, {}), CHIP_NO_ERROR);

        ReadOnlyBuffer<AttributeEntry> result = builder.TakeBuffer();
        ASSERT_EQ(result.size(), global_attribute_count);
        ASSERT_EQ(result[0].attributeId, Globals::Attributes::FeatureMap::Id);
    }
}
