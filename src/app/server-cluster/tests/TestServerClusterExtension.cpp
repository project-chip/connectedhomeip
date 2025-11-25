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

#include <access/Privilege.h>
#include <app/ConcreteClusterPath.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/ServerClusterExtension.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/shared/GlobalIds.h>
#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <protocols/interaction_model/Constants.h>

#include <app/clusters/testing/AttributeTesting.h>
#include <app/clusters/testing/ClusterTester.h>
#include <app/clusters/testing/ValidateGlobalAttributes.h>

#include <string>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::Test;
using namespace chip::Protocols::InteractionModel;

constexpr uint32_t kMockRevision   = 123;
constexpr uint32_t kMockFeatureMap = 0x112233;

/// a basic mock cluster, just supporting one path
class MockServerCluster : public DefaultServerCluster
{
public:
    MockServerCluster(const ConcreteClusterPath & path) : DefaultServerCluster(path) {}

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override
    {
        switch (request.path.mAttributeId)
        {
        case chip::app::Clusters::Globals::Attributes::ClusterRevision::Id:
            return encoder.Encode(kMockRevision);
        case chip::app::Clusters::Globals::Attributes::FeatureMap::Id:
            return encoder.Encode(kMockFeatureMap);
        default:
            return Status::UnsupportedAttribute;
        }
    }
};

/// A dual path server cluster (just mocks GetPaths, nothing else)
class MockDualPathServerCluster : public DefaultServerCluster
{
public:
    MockDualPathServerCluster(const ConcreteClusterPath & p1, const ConcreteClusterPath & p2) :
        DefaultServerCluster(p1), mPaths{ p1, p2 }
    {}

    // mock GetPaths, to have something
    Span<const ConcreteClusterPath> GetPaths() const override { return Span(mPaths); }

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override
    {
        switch (request.path.mAttributeId)
        {
        case chip::app::Clusters::Globals::Attributes::ClusterRevision::Id:
            return encoder.Encode(kMockRevision);
        case chip::app::Clusters::Globals::Attributes::FeatureMap::Id:
            return encoder.Encode(kMockFeatureMap);
        default:
            return Status::UnsupportedAttribute;
        }
    }

private:
    const ConcreteClusterPath mPaths[2] = {};
};

constexpr AttributeId kTestAttribute1 = 0xFFF10000;
constexpr AttributeId kTestAttribute2 = 0xFFF10001;

constexpr DataModel::AttributeEntry kExtraAttributeMetadata[] = {
    { kTestAttribute1, {} /* qualities */, Access::Privilege::kView /* readPriv */, Access::Privilege::kOperate /* writePriv */ },
    { kTestAttribute2, {} /* qualities */, Access::Privilege::kView /* readPriv */, std::nullopt /* writePriv */ },
};

class TestableServerClusterExtension : public ServerClusterExtension
{
public:
    TestableServerClusterExtension(const ConcreteClusterPath & path, ServerClusterInterface & underlying) :
        ServerClusterExtension(path, underlying)
    {}

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override
    {
        if (mClusterPath == request.path)
        {
            switch (request.path.mAttributeId)
            {
            case kTestAttribute1:
                return encoder.Encode<CharSpan>({ mStringAttribute.data(), mStringAttribute.size() });
            case kTestAttribute2:
                return encoder.Encode<uint32_t>(1234);
            }
        }
        return mUnderlying.ReadAttribute(request, encoder);
    }

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override
    {
        if (path == mClusterPath)
        {
            ReturnErrorOnFailure(builder.ReferenceExisting(kExtraAttributeMetadata));
        }

        // Delegate to underlying for other paths
        return mUnderlying.Attributes(path, builder);
    }

    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override
    {
        if (mClusterPath == request.path)
        {
            // we are guaranteed to only be called for writable attributes
            if (request.path.mAttributeId == kTestAttribute1)
            {
                CharSpan value;
                ReturnErrorOnFailure(decoder.Decode(value));
                mStringAttribute = std::string(value.data(), value.size());
                return Status::Success;
            }
        }

        return mUnderlying.WriteAttribute(request, decoder);
    }

    void TestNotifyAttributeChanged(AttributeId id) { NotifyAttributeChanged(id); }

private:
    std::string mStringAttribute = "Sample String";
    std::vector<DataModel::AttributeEntry> mCombinedAttributes;
};

struct TestServerClusterExtension : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestServerClusterExtension, TestExtensionPath)
{
    const ConcreteClusterPath mockPath = { 1, 2 };
    MockServerCluster underlying(mockPath);
    const ConcreteClusterPath extensionPath = { 1, 2 };

    ServerClusterExtension extension(extensionPath, underlying);

    // The extension should return the paths of its underlying interface.
    ASSERT_EQ(extension.GetPaths().size(), 1u);
    ASSERT_EQ(extension.GetPaths()[0], mockPath);
}

TEST_F(TestServerClusterExtension, TestGetDataVersion)
{
    const ConcreteClusterPath mockPath = { 2, 3 };
    MockServerCluster underlying(mockPath);

    TestableServerClusterExtension extension(mockPath, underlying);

    // Initially, version is the same as underlying (since mVersionDelta is 0).
    ASSERT_EQ(extension.GetDataVersion(mockPath), underlying.GetDataVersion(mockPath));

    // When NotifyAttributeChanged is called WITHOUT a context, mVersionDelta should still increment,
    // but the underlying listener is not marked dirty.
    // Since mContext is initially nullptr, mVersionDelta should NOT increase in the original code
    // (due to VerifyOrReturn). So the version should remain the same as underlying initially
    extension.TestNotifyAttributeChanged(4);
    ASSERT_EQ(extension.GetDataVersion(mockPath),
              underlying.GetDataVersion(mockPath)); // Should still be the same as no context is set

    // Set a context and then notify change. This time mVersionDelta should increment.
    TestServerClusterContext context;
    ASSERT_EQ(extension.Startup(context.Get()), CHIP_NO_ERROR);

    DataVersion oldVersion = extension.GetDataVersion(mockPath);
    extension.TestNotifyAttributeChanged(5);
    ASSERT_EQ(extension.GetDataVersion(mockPath), oldVersion + 1);
}

TEST_F(TestServerClusterExtension, TestNotifyAttributeChangedWithContext)
{
    const ConcreteClusterPath mockPath  = { 1, 2 };
    const ConcreteClusterPath mockPath2 = { 1, 3 };
    MockDualPathServerCluster underlying(mockPath, mockPath2);
    TestableServerClusterExtension extension(mockPath, underlying);

    // Verify that NotifyAttributeChanged does NOT mark dirty when no context is set.
    extension.TestNotifyAttributeChanged(123);
    // No context set, so no dirty list updates. Default TestServerClusterContext has an empty list.
    // We cannot easily check mVersionDelta directly as it's protected in ServerClusterExtension.
    // The effect of mVersionDelta is visible through GetDataVersion.

    // Create a ServerClusterContext and verify that attribute change notifications are processed.
    TestServerClusterContext context;
    ASSERT_EQ(extension.Startup(context.Get()), CHIP_NO_ERROR);

    // Clear any previous dirty marks from calls before Startup.
    context.ChangeListener().DirtyList().clear();

    DataVersion oldVersion = extension.GetDataVersion(mockPath);
    extension.TestNotifyAttributeChanged(234);
    ASSERT_EQ(extension.GetDataVersion(mockPath), oldVersion + 1);
    ASSERT_EQ(extension.GetDataVersion(mockPath2), oldVersion);

    ASSERT_EQ(context.ChangeListener().DirtyList().size(), 1u);
    ASSERT_EQ(context.ChangeListener().DirtyList()[0], AttributePathParams(mockPath.mEndpointId, mockPath.mClusterId, 234));
}

TEST_F(TestServerClusterExtension, TestExtensionAttributes)
{
    const ConcreteClusterPath mockPath = { 1, 2 };
    MockServerCluster underlying(mockPath);
    TestableServerClusterExtension extension(mockPath, underlying);

    chip::Test::ClusterTester tester(extension);

    // Verify attribute listing includes global and extended attributes
    ASSERT_TRUE(chip::Testing::IsAttributesListEqualTo(extension, { kExtraAttributeMetadata[0], kExtraAttributeMetadata[1] }));

    // Test reading kTestAttribute1 (string)
    CharSpan readString;
    ASSERT_EQ(tester.ReadAttribute(kTestAttribute1, readString), Status::Success);
    ASSERT_TRUE(readString.data_equal("Sample String"_span));

    // Test reading kTestAttribute2 (uint32_t)
    uint32_t readUint;
    ASSERT_EQ(tester.ReadAttribute(kTestAttribute2, readUint), Status::Success);
    ASSERT_EQ(readUint, 1234u);

    // Test writing kTestAttribute1 (string)
    CharSpan newString = "New Test String"_span;
    ASSERT_EQ(tester.WriteAttribute(kTestAttribute1, newString), Status::Success);

    // Read back to verify write
    CharSpan verifiedString;
    ASSERT_EQ(tester.ReadAttribute(kTestAttribute1, verifiedString), Status::Success);
    ASSERT_TRUE(verifiedString.data_equal(newString));
}

} // namespace
