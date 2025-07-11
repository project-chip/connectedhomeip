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

#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/TestConstants.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/ServerClusterContext.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/Descriptor/ClusterId.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <data-model-providers/codedriven/endpoint/SpanEndpoint.h>
#include <lib/core/TLV.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <lib/support/Span.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <algorithm>
#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Testing;
using SemanticTag = chip::app::Clusters::Descriptor::Structs::SemanticTagStruct::Type;

class TestProviderChangeListener : public DataModel::ProviderChangeListener
{
public:
    void MarkDirty(const AttributePathParams & path) override { mDirtyList.push_back(path); }
    std::vector<AttributePathParams> mDirtyList;
};

class TestEventGenerator : public DataModel::EventsGenerator
{
    CHIP_ERROR GenerateEvent(EventLoggingDelegate * eventPayloadWriter, const EventOptions & options,
                             EventNumber & generatedEventNumber) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
};

class TestActionContext : public DataModel::ActionContext
{
public:
    Messaging::ExchangeContext * CurrentExchange() override { return nullptr; }
};

class TestCodeDrivenDataModelProvider : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

protected:
    TestProviderChangeListener mChangeListener;
    TestEventGenerator mEventGenerator;
    TestActionContext mActionContext;
    DataModel::InteractionModelContext mContext{
        .eventsGenerator         = &mEventGenerator,
        .dataModelChangeListener = &mChangeListener,
        .actionContext           = &mActionContext,
    };
    CodeDrivenDataModelProvider mProvider;
    std::vector<std::unique_ptr<SpanEndpoint>> mEndpointStorage;                     // To keep providers alive
    std::vector<std::unique_ptr<EndpointInterfaceRegistration>> mOwnedRegistrations; // To keep registration objects alive
    chip::Test::TestServerClusterContext mServerClusterTestContext;

    TestCodeDrivenDataModelProvider()
    {
        mProvider.SetPersistentStorageDelegate(&mServerClusterTestContext.StorageDelegate());
        EXPECT_EQ(mProvider.Startup(mContext), CHIP_NO_ERROR);
    }

    ~TestCodeDrivenDataModelProvider() override
    {
        mProvider.Shutdown();
        mEndpointStorage.clear();
        mOwnedRegistrations.clear();
    }
};

// MockServerCluster implementation
class MockServerCluster : public DefaultServerCluster
{
public:
    MockServerCluster(std::initializer_list<ConcreteClusterPath> paths, DataVersion dataVersion,
                      BitFlags<DataModel::ClusterQualityFlags> flags) :
        DefaultServerCluster({ 0, 0 }), mPaths(paths), mDataVersion(dataVersion), mFlags(flags),
        mAttributeEntry(1, BitMask<DataModel::AttributeQualityFlags>(), std::nullopt, std::nullopt)
    {}

    MockServerCluster(ConcreteClusterPath path, DataVersion dataVersion, BitFlags<DataModel::ClusterQualityFlags> flags) :
        MockServerCluster({ path }, dataVersion, flags)
    {}

    ~MockServerCluster() override = default;

    chip::Span<const ConcreteClusterPath> GetPaths() const override
    {
        return chip::Span<const ConcreteClusterPath>(mPaths.data(), mPaths.size());
    }
    chip::DataVersion GetDataVersion(const ConcreteClusterPath &) const override { return mDataVersion; }
    BitFlags<DataModel::ClusterQualityFlags> GetClusterFlags(const ConcreteClusterPath &) const override { return mFlags; }

    bool IsPathHandled(const ConcreteClusterPath & path) const
    {
        for (const auto & p : mPaths)
        {
            if (p.mEndpointId == path.mEndpointId && p.mClusterId == path.mClusterId)
            {
                return true;
            }
        }
        return false;
    }

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override
    {
        if (!IsPathHandled(request.path))
        {
            return DataModel::ActionReturnStatus(CHIP_ERROR_INVALID_ARGUMENT);
        }
        if (request.path.mAttributeId != mAttributeEntry.attributeId)
        {
            return DataModel::ActionReturnStatus(Protocols::InteractionModel::Status::UnsupportedAttribute);
        }
        mLastReadRequest = request;
        return encoder.Encode(mAttributeValue);
    }

    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override
    {
        if (!IsPathHandled(request.path))
        {
            return DataModel::ActionReturnStatus(CHIP_ERROR_INVALID_ARGUMENT);
        }
        if (request.path.mAttributeId != mAttributeEntry.attributeId)
        {
            return DataModel::ActionReturnStatus(Protocols::InteractionModel::Status::UnsupportedAttribute);
        }
        mLastWriteRequest = request;
        return decoder.Decode(mAttributeValue);
    }

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override
    {
        if (!IsPathHandled(request.path))
        {
            return DataModel::ActionReturnStatus(CHIP_ERROR_INVALID_ARGUMENT);
        }
        if (request.path.mCommandId != mAcceptedCommandEntry.commandId)
        {
            return DataModel::ActionReturnStatus(Protocols::InteractionModel::Status::UnsupportedCommand);
        }
        mLastInvokeRequest = request;
        return DataModel::ActionReturnStatus(Protocols::InteractionModel::Status::Success);
    }

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override
    {
        if (!IsPathHandled(path))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        ReturnErrorOnFailure(builder.EnsureAppendCapacity(1));
        return builder.Append(mAttributeEntry);
    }

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override
    {
        if (!IsPathHandled(path))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        ReturnErrorOnFailure(builder.EnsureAppendCapacity(1));
        return builder.Append(mAcceptedCommandEntry);
    }

    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<chip::CommandId> & builder) override
    {
        if (!IsPathHandled(path))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        ReturnErrorOnFailure(builder.EnsureAppendCapacity(1));
        return builder.Append(mGeneratedCommandId);
    }

    CHIP_ERROR EventInfo(const ConcreteEventPath & path, DataModel::EventEntry & eventInfo) override
    {
        if (!IsPathHandled(path))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        eventInfo = mEventEntry;
        return CHIP_NO_ERROR;
    }

    void ListAttributeWriteNotification(const ConcreteAttributePath & path, DataModel::ListWriteOperation opType) override
    {
        mLastListWriteOpPath = path;
        mLastListWriteOpType = opType;
    }

    CHIP_ERROR Startup(ServerClusterContext & context) override
    {
        startupCalled = true;
        return DefaultServerCluster::Startup(context);
    }

    void Shutdown() override
    {
        shutdownCalled = true;
        DefaultServerCluster::Shutdown();
    }

    bool startupCalled  = false;
    bool shutdownCalled = false;

    DataModel::ReadAttributeRequest mLastReadRequest;
    DataModel::WriteAttributeRequest mLastWriteRequest;
    DataModel::InvokeRequest mLastInvokeRequest;
    uint32_t mAttributeValue = 42;
    std::vector<ConcreteClusterPath> mPaths;
    DataVersion mDataVersion;
    BitFlags<DataModel::ClusterQualityFlags> mFlags;
    DataModel::AttributeEntry mAttributeEntry;
    DataModel::AcceptedCommandEntry mAcceptedCommandEntry = { 1, true };
    CommandId mGeneratedCommandId                         = 2;
    DataModel::EventEntry mEventEntry;
    std::optional<ConcreteAttributePath> mLastListWriteOpPath;
    std::optional<DataModel::ListWriteOperation> mLastListWriteOpType;
};

namespace {

// Static Data Used for Tests

constexpr DataModel::EndpointEntry endpointEntry1 = { .id                 = 1,
                                                      .parentId           = kInvalidEndpointId,
                                                      .compositionPattern = DataModel::EndpointCompositionPattern::kFullFamily };

constexpr DataModel::EndpointEntry endpointEntry2 = { .id                 = 2,
                                                      .parentId           = kInvalidEndpointId,
                                                      .compositionPattern = DataModel::EndpointCompositionPattern::kTree };

constexpr DataModel::EndpointEntry endpointEntry3 = { .id                 = 3,
                                                      .parentId           = kInvalidEndpointId,
                                                      .compositionPattern = DataModel::EndpointCompositionPattern::kFullFamily };

constexpr DataModel::EndpointEntry endpointEntry4 = { .id                 = 4,
                                                      .parentId           = kInvalidEndpointId,
                                                      .compositionPattern = DataModel::EndpointCompositionPattern::kFullFamily };

SemanticTag semanticTag1 = { .mfgCode     = VendorId::Google,
                             .namespaceID = 1,
                             .tag         = 1,
                             .label       = chip::Optional<chip::app::DataModel::Nullable<chip::CharSpan>>(
                                 { chip::app::DataModel::MakeNullable(chip::CharSpan("label1", 6)) }) };
SemanticTag semanticTag2 = { .mfgCode     = VendorId::Google,
                             .namespaceID = 2,
                             .tag         = 2,
                             .label       = chip::Optional<chip::app::DataModel::Nullable<chip::CharSpan>>(
                                 { chip::app::DataModel::MakeNullable(chip::CharSpan("label1", 6)) }) };

MockServerCluster mockServerCluster1(ConcreteClusterPath(1, 10), 1, BitFlags<DataModel::ClusterQualityFlags>());
MockServerCluster
    mockServerCluster2(ConcreteClusterPath(1, 20), 2,
                       BitFlags<DataModel::ClusterQualityFlags>().Set(DataModel::ClusterQualityFlags::kDiagnosticsData));
MockServerCluster mockServerCluster3(ConcreteClusterPath(2, 30), 3, BitFlags<DataModel::ClusterQualityFlags>());

MockServerCluster mockServerClusterMultiPath({ { 3, 40 }, { 3, 50 } }, 1, BitFlags<DataModel::ClusterQualityFlags>());

constexpr chip::EndpointId clientClusterId1 = 1;
constexpr chip::EndpointId clientClusterId2 = 2;

// Define kMax constants locally for testing purposes
constexpr unsigned int kTestMaxDeviceTypes = 5;

MockServerCluster descriptorClusterEP1({ endpointEntry1.id, chip::app::Clusters::Descriptor::Id }, 1,
                                       BitFlags<DataModel::ClusterQualityFlags>());
MockServerCluster descriptorClusterEP2({ endpointEntry2.id, chip::app::Clusters::Descriptor::Id }, 1,
                                       BitFlags<DataModel::ClusterQualityFlags>());
MockServerCluster descriptorClusterEP3({ endpointEntry3.id, chip::app::Clusters::Descriptor::Id }, 1,
                                       BitFlags<DataModel::ClusterQualityFlags>());
MockServerCluster descriptorClusterEP4({ endpointEntry4.id, chip::app::Clusters::Descriptor::Id }, 1,
                                       BitFlags<DataModel::ClusterQualityFlags>());

CHIP_ERROR ReadU32Attribute(DataModel::Provider & provider, const ConcreteAttributePath & path, uint32_t & value)
{

    ReadOperation testRequest(path);
    testRequest.SetSubjectDescriptor(kAdminSubjectDescriptor);

    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding();
    ReturnErrorOnFailure(provider.ReadAttribute(testRequest.GetRequest(), *encoder).GetUnderlyingError());
    ReturnErrorOnFailure(testRequest.FinishEncoding());

    std::vector<DecodedAttributeData> attribute_data;

    ReturnErrorOnFailure(testRequest.GetEncodedIBs().Decode(attribute_data));
    VerifyOrReturnError(attribute_data.size() == 1u, CHIP_ERROR_INCORRECT_STATE);

    DecodedAttributeData & encodedData = attribute_data[0];

    VerifyOrReturnError(encodedData.attributePath == testRequest.GetRequest().path, CHIP_ERROR_INCORRECT_STATE);

    return chip::app::DataModel::Decode<uint32_t>(encodedData.dataReader, value);
}

CHIP_ERROR WriteU32Attribute(DataModel::Provider & provider, const ConcreteAttributePath & path, uint32_t value)
{

    WriteOperation testRequest(path);
    testRequest.SetSubjectDescriptor(kAdminSubjectDescriptor);

    AttributeValueDecoder decoder = testRequest.DecoderFor(value);
    return provider.WriteAttribute(testRequest.GetRequest(), decoder).GetUnderlyingError();
}

} // namespace

TEST_F(TestCodeDrivenDataModelProvider, IterateOverEndpoints)
{
    static ServerClusterInterface * serverClusters1[] = { &descriptorClusterEP1 };
    static ServerClusterInterface * serverClusters2[] = { &descriptorClusterEP2 };
    static ServerClusterInterface * serverClusters3[] = { &descriptorClusterEP3 };

    // Create 3 SpanEndpoints with different IDs and server clusters
    // EP1
    auto build_pair1 = SpanEndpoint::Builder(endpointEntry1.id)
                           .SetComposition(endpointEntry1.compositionPattern)
                           .SetParentId(endpointEntry1.parentId)
                           .SetServerClusters(Span<ServerClusterInterface *>(serverClusters1))
                           .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_pair1));
    auto ep1Provider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair1)));

    // EP2
    auto build_pair2 = SpanEndpoint::Builder(endpointEntry2.id)
                           .SetComposition(endpointEntry2.compositionPattern)
                           .SetParentId(endpointEntry2.parentId)
                           .SetServerClusters(Span<ServerClusterInterface *>(serverClusters2))
                           .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_pair2));
    auto ep2Provider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair2)));

    // EP3
    auto build_pair3 = SpanEndpoint::Builder(endpointEntry3.id)
                           .SetComposition(endpointEntry3.compositionPattern)
                           .SetParentId(endpointEntry3.parentId)
                           .SetServerClusters(Span<ServerClusterInterface *>(serverClusters3))
                           .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_pair3));
    auto ep3Provider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair3)));

    mEndpointStorage.push_back(std::move(ep1Provider));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back()));
    ASSERT_EQ(mProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);
    mEndpointStorage.push_back(std::move(ep2Provider));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back()));
    ASSERT_EQ(mProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);
    mEndpointStorage.push_back(std::move(ep3Provider));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back()));
    ASSERT_EQ(mProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::EndpointEntry> endpointsBuilder;

    ASSERT_EQ(mProvider.Endpoints(endpointsBuilder), CHIP_NO_ERROR);

    ReadOnlyBuffer<DataModel::EndpointEntry> endpoints_rb = endpointsBuilder.TakeBuffer();
    // ReadOnlyBuffer<T> inherits from Span<const T>, so we can use its iterators directly.
    std::vector<DataModel::EndpointEntry> actual_endpoints(endpoints_rb.begin(), endpoints_rb.end());
    // Sort actual endpoints by ID to make the test order-independent
    std::sort(actual_endpoints.begin(), actual_endpoints.end(),
              [](const DataModel::EndpointEntry & a, const DataModel::EndpointEntry & b) { return a.id < b.id; });

    ASSERT_EQ(actual_endpoints.size(), 3u);

    // Compare against expected entries, assuming expected entries are defined in ID-sorted order
    EXPECT_EQ(actual_endpoints[0], endpointEntry1);
    EXPECT_EQ(actual_endpoints[1], endpointEntry2);
    EXPECT_EQ(actual_endpoints[2], endpointEntry3);
}

TEST_F(TestCodeDrivenDataModelProvider, IterateOverServerClusters)
{
    // mockServerCluster1 and mockServerCluster2 are global.
    // These need to be static to ensure their lifetime extends beyond the scope of this function,
    // as the SpanEndpoint will hold a Span pointing to them.
    static ServerClusterInterface * sServerClustersArray[] = { &descriptorClusterEP1, &mockServerCluster1, &mockServerCluster2 };

    auto build_pair = SpanEndpoint::Builder(endpointEntry1.id)
                          .SetComposition(endpointEntry1.compositionPattern)
                          .SetParentId(endpointEntry1.parentId)
                          .SetServerClusters(Span<ServerClusterInterface *>(sServerClustersArray))
                          .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_pair));
    auto epProvider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair)));

    mEndpointStorage.push_back(std::move(epProvider));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back()));
    ASSERT_EQ(mProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::ServerClusterEntry> builder;
    ASSERT_EQ(mProvider.ServerClusters(endpointEntry1.id, builder), CHIP_NO_ERROR);

    auto serverClusters = builder.TakeBuffer();
    ASSERT_EQ(serverClusters.size(), 3u);

    EXPECT_EQ(serverClusters[0].clusterId, descriptorClusterEP1.GetPaths()[0].mClusterId);
    EXPECT_EQ(serverClusters[0].dataVersion, descriptorClusterEP1.GetDataVersion({}));
    EXPECT_EQ(serverClusters[0].flags, descriptorClusterEP1.GetClusterFlags({}));

    EXPECT_EQ(serverClusters[1].clusterId, mockServerCluster1.GetPaths()[0].mClusterId);
    EXPECT_EQ(serverClusters[1].dataVersion, mockServerCluster1.GetDataVersion({}));
    EXPECT_EQ(serverClusters[1].flags, mockServerCluster1.GetClusterFlags({}));

    EXPECT_EQ(serverClusters[2].clusterId, mockServerCluster2.GetPaths()[0].mClusterId);
    EXPECT_EQ(serverClusters[2].dataVersion, mockServerCluster2.GetDataVersion({}));
    EXPECT_EQ(serverClusters[2].flags, mockServerCluster2.GetClusterFlags({}));
}

TEST_F(TestCodeDrivenDataModelProvider, IterateOverClientClusters)
{
    // clientClusterId1 and clientClusterId2 are global.
    static ServerClusterInterface * sServerClustersArray[] = { &descriptorClusterEP1 };
    static const ClusterId sClientClustersArray[]          = { clientClusterId1, clientClusterId2 };

    auto build_pair = SpanEndpoint::Builder(endpointEntry1.id)
                          .SetComposition(endpointEntry1.compositionPattern)
                          .SetServerClusters(Span<ServerClusterInterface *>(sServerClustersArray))
                          .SetParentId(endpointEntry1.parentId)
                          .SetClientClusters(Span<const ClusterId>(sClientClustersArray))
                          .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_pair));
    auto epProvider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair)));

    mEndpointStorage.push_back(std::move(epProvider));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back()));
    ASSERT_EQ(mProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<chip::ClusterId> builder;
    ASSERT_EQ(mProvider.ClientClusters(endpointEntry1.id, builder), CHIP_NO_ERROR);

    auto clientClusters = builder.TakeBuffer();
    ASSERT_EQ(clientClusters.size(), 2u);
    EXPECT_EQ(clientClusters[0], clientClusterId1);
    EXPECT_EQ(clientClusters[1], clientClusterId2);
}

TEST_F(TestCodeDrivenDataModelProvider, IterateOverTags)
{
    // semanticTag1 and semanticTag2 are global.
    static ServerClusterInterface * sServerClustersArray[] = { &descriptorClusterEP1 };
    static const SemanticTag sSemanticTagsArray[]          = { semanticTag1, semanticTag2 };

    auto build_pair = SpanEndpoint::Builder(endpointEntry1.id)
                          .SetComposition(endpointEntry1.compositionPattern)
                          .SetServerClusters(Span<ServerClusterInterface *>(sServerClustersArray))
                          .SetParentId(endpointEntry1.parentId)
                          .SetSemanticTags(Span<const SemanticTag>(sSemanticTagsArray))
                          .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_pair));
    auto epProvider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair)));

    mEndpointStorage.push_back(std::move(epProvider));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back()));
    ASSERT_EQ(mProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<SemanticTag> builder;
    ASSERT_EQ(mProvider.SemanticTags(endpointEntry1.id, builder), CHIP_NO_ERROR);

    auto tags = builder.TakeBuffer();
    ASSERT_EQ(tags.size(), 2u);
    // SemanticTag doesn't have operator==, compare members
    EXPECT_EQ(tags[0].mfgCode, semanticTag1.mfgCode);
    EXPECT_EQ(tags[0].namespaceID, semanticTag1.namespaceID);
    EXPECT_EQ(tags[0].tag, semanticTag1.tag);
    EXPECT_EQ(tags[1].mfgCode, semanticTag2.mfgCode);
    EXPECT_EQ(tags[1].namespaceID, semanticTag2.namespaceID);
    EXPECT_EQ(tags[1].tag, semanticTag2.tag);
}

TEST_F(TestCodeDrivenDataModelProvider, IterateOverDeviceTypes)
{
    static DataModel::DeviceTypeEntry sDeviceTypesData[kTestMaxDeviceTypes];
    for (unsigned int i = 0; i < kTestMaxDeviceTypes; i++)
    {
        sDeviceTypesData[i].deviceTypeId       = static_cast<uint32_t>(i);
        sDeviceTypesData[i].deviceTypeRevision = static_cast<uint8_t>((i % 255) + 1); // Ensure non-zero and varying revision
    }
    static ServerClusterInterface * sServerClustersArray[] = { &descriptorClusterEP1 };

    auto build_pair = SpanEndpoint::Builder(endpointEntry1.id)
                          .SetComposition(endpointEntry1.compositionPattern)
                          .SetParentId(endpointEntry1.parentId)
                          .SetDeviceTypes(Span<const DataModel::DeviceTypeEntry>(sDeviceTypesData))
                          .SetServerClusters(Span<ServerClusterInterface *>(sServerClustersArray))
                          .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_pair));
    auto epProvider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair)));

    mEndpointStorage.push_back(std::move(epProvider));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back()));
    ASSERT_EQ(mProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> builder;
    ASSERT_EQ(mProvider.DeviceTypes(endpointEntry1.id, builder), CHIP_NO_ERROR);

    auto deviceTypesResult = builder.TakeBuffer();
    ASSERT_EQ(deviceTypesResult.size(), static_cast<size_t>(kTestMaxDeviceTypes));
    for (unsigned int i = 0; i < kTestMaxDeviceTypes; i++)
    {
        EXPECT_EQ(deviceTypesResult[i].deviceTypeId, static_cast<uint32_t>(i));
        EXPECT_EQ(deviceTypesResult[i].deviceTypeRevision, static_cast<uint8_t>((i % 255) + 1));
    }
}

TEST_F(TestCodeDrivenDataModelProvider, AddAndRemoveEndpoints)
{
    static ServerClusterInterface * serverClusters1[] = { &descriptorClusterEP1 };
    static ServerClusterInterface * serverClusters2[] = { &descriptorClusterEP2 };
    static ServerClusterInterface * serverClusters3[] = { &descriptorClusterEP3 };

    // Add 3 endpoints
    auto build_pair1 = SpanEndpoint::Builder(endpointEntry1.id)
                           .SetComposition(endpointEntry1.compositionPattern)
                           .SetParentId(endpointEntry1.parentId)
                           .SetServerClusters(Span<ServerClusterInterface *>(serverClusters1))
                           .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_pair1));
    auto ep1Provider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair1)));

    auto build_pair2 = SpanEndpoint::Builder(endpointEntry2.id)
                           .SetComposition(endpointEntry2.compositionPattern)
                           .SetParentId(endpointEntry2.parentId)
                           .SetServerClusters(Span<ServerClusterInterface *>(serverClusters2))
                           .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_pair2));
    auto ep2Provider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair2)));

    auto build_pair3 = SpanEndpoint::Builder(endpointEntry3.id)
                           .SetComposition(endpointEntry3.compositionPattern)
                           .SetParentId(endpointEntry3.parentId)
                           .SetServerClusters(Span<ServerClusterInterface *>(serverClusters3))
                           .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_pair3));
    auto ep3Provider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair3)));

    mEndpointStorage.push_back(std::move(ep1Provider));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back()));
    ASSERT_EQ(mProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);
    mEndpointStorage.push_back(std::move(ep2Provider));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back()));
    ASSERT_EQ(mProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);
    mEndpointStorage.push_back(std::move(ep3Provider));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back()));
    ASSERT_EQ(mProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);

    // Remove endpoints 1 and 3
    EXPECT_EQ(mProvider.RemoveEndpoint(endpointEntry1.id), CHIP_NO_ERROR);
    EXPECT_EQ(mProvider.RemoveEndpoint(endpointEntry3.id), CHIP_NO_ERROR);

    // Assert that the remaining endpoints are as expected
    ReadOnlyBufferBuilder<DataModel::EndpointEntry> out;
    EXPECT_EQ(mProvider.Endpoints(out), CHIP_NO_ERROR);
    EXPECT_EQ(out.Size(), 1U);

    auto endpoints = out.TakeBuffer();
    EXPECT_EQ(endpoints[0].id, endpointEntry2.id);
}

TEST_F(TestCodeDrivenDataModelProvider, EndpointWithStaticData)
{
    CodeDrivenDataModelProvider localProvider; // Use a local provider for focused testing
    localProvider.SetPersistentStorageDelegate(&mServerClusterTestContext.StorageDelegate());
    ASSERT_EQ(localProvider.Startup(mContext), CHIP_NO_ERROR);

    // Define static data directly in arrays. MockServerCluster instances need to be static
    // or have a lifetime that outlasts their use by the Span.
    static MockServerCluster mockEpServerCluster1(ConcreteClusterPath(endpointEntry4.id, 300), 1,
                                                  BitFlags<DataModel::ClusterQualityFlags>());
    static MockServerCluster mockEpServerCluster2(ConcreteClusterPath(endpointEntry4.id, 301), 2,
                                                  BitFlags<DataModel::ClusterQualityFlags>());

    static ServerClusterInterface * serverClustersArray[] = { &descriptorClusterEP4, &mockEpServerCluster1, &mockEpServerCluster2 };
    static const ClusterId clientClustersArray[]          = { 0xD001, 0xD002 };
    static const SemanticTag semanticTagsArray[]          = { { .mfgCode = VendorId::Google, .namespaceID = 10, .tag = 100 },
                                                              { .mfgCode = VendorId::Google, .namespaceID = 11, .tag = 101 } };
    static const DataModel::DeviceTypeEntry deviceTypesArray[] = { { .deviceTypeId = 0x7001, .deviceTypeRevision = 1 },
                                                                   { .deviceTypeId = 0x7002, .deviceTypeRevision = 2 } };

    auto build_pair = SpanEndpoint::Builder(endpointEntry4.id)
                          .SetComposition(endpointEntry4.compositionPattern)
                          .SetParentId(endpointEntry4.parentId)
                          .SetServerClusters(chip::Span<ServerClusterInterface *>(serverClustersArray))
                          .SetClientClusters(chip::Span<const ClusterId>(clientClustersArray))
                          .SetSemanticTags(chip::Span<const SemanticTag>(semanticTagsArray))
                          .SetDeviceTypes(chip::Span<const DataModel::DeviceTypeEntry>(deviceTypesArray))
                          .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_pair));
    SpanEndpoint ep = std::move(std::get<SpanEndpoint>(build_pair));

    EndpointInterfaceRegistration registration(ep);
    ASSERT_EQ(localProvider.AddEndpoint(registration), CHIP_NO_ERROR);

    // Verify Endpoints through localProvider
    ReadOnlyBufferBuilder<DataModel::EndpointEntry> epBuilder;
    ASSERT_EQ(localProvider.Endpoints(epBuilder), CHIP_NO_ERROR);
    auto eps = epBuilder.TakeBuffer();
    ASSERT_EQ(eps.size(), 1u);
    EXPECT_EQ(eps[0].id, endpointEntry4.id);
    EXPECT_EQ(eps[0].compositionPattern, endpointEntry4.compositionPattern);

    // Verify Server Clusters
    ReadOnlyBufferBuilder<DataModel::ServerClusterEntry> scBuilder;
    ASSERT_EQ(localProvider.ServerClusters(endpointEntry4.id, scBuilder), CHIP_NO_ERROR);
    auto scs = scBuilder.TakeBuffer();
    ASSERT_EQ(scs.size(), 3u);
    EXPECT_EQ(scs[0].clusterId, descriptorClusterEP4.GetPaths()[0].mClusterId);
    EXPECT_EQ(scs[1].clusterId, mockEpServerCluster1.GetPaths()[0].mClusterId);
    EXPECT_EQ(scs[2].clusterId, mockEpServerCluster2.GetPaths()[0].mClusterId);

    // Verify Client Clusters
    ReadOnlyBufferBuilder<ClusterId> ccBuilder;
    ASSERT_EQ(localProvider.ClientClusters(endpointEntry4.id, ccBuilder), CHIP_NO_ERROR);
    auto ccs = ccBuilder.TakeBuffer();
    ASSERT_EQ(ccs.size(), std::size(clientClustersArray));
    EXPECT_EQ(ccs[0], clientClustersArray[0]);
    EXPECT_EQ(ccs[1], clientClustersArray[1]);

    // Verify Semantic Tags
    ReadOnlyBufferBuilder<SemanticTag> stBuilder;
    ASSERT_EQ(localProvider.SemanticTags(endpointEntry4.id, stBuilder), CHIP_NO_ERROR);
    auto sts = stBuilder.TakeBuffer();
    ASSERT_EQ(sts.size(), std::size(semanticTagsArray));
    EXPECT_EQ(sts[0].mfgCode, semanticTagsArray[0].mfgCode);
    EXPECT_EQ(sts[0].namespaceID, semanticTagsArray[0].namespaceID);
    EXPECT_EQ(sts[0].tag, semanticTagsArray[0].tag);
    EXPECT_EQ(sts[1].mfgCode, semanticTagsArray[1].mfgCode);
    EXPECT_EQ(sts[1].namespaceID, semanticTagsArray[1].namespaceID);
    EXPECT_EQ(sts[1].tag, semanticTagsArray[1].tag);

    // Verify Device Types
    ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> dtBuilder;
    ASSERT_EQ(localProvider.DeviceTypes(endpointEntry4.id, dtBuilder), CHIP_NO_ERROR);
    auto dts = dtBuilder.TakeBuffer();
    ASSERT_EQ(dts.size(), std::size(deviceTypesArray));
    EXPECT_EQ(dts[0].deviceTypeId, deviceTypesArray[0].deviceTypeId);
    EXPECT_EQ(dts[0].deviceTypeRevision, deviceTypesArray[0].deviceTypeRevision);
    EXPECT_EQ(dts[1].deviceTypeId, deviceTypesArray[1].deviceTypeId);
    EXPECT_EQ(dts[1].deviceTypeRevision, deviceTypesArray[1].deviceTypeRevision);

    // Verify GetServerCluster on Endpoint instance
    EXPECT_EQ(ep.GetServerCluster(mockEpServerCluster1.GetPaths()[0].mClusterId), &mockEpServerCluster1);
    EXPECT_EQ(ep.GetServerCluster(mockEpServerCluster2.GetPaths()[0].mClusterId), &mockEpServerCluster2);
    EXPECT_EQ(ep.GetServerCluster(0xFFFF /* Non-existent cluster ID */), nullptr);

    localProvider.Shutdown();
}

TEST_F(TestCodeDrivenDataModelProvider, EndpointWithEmptyStaticData)
{
    static ServerClusterInterface * serverClustersArray[] = { &descriptorClusterEP4 };

    auto build_pair = SpanEndpoint::Builder(endpointEntry4.id)
                          .SetComposition(endpointEntry4.compositionPattern)
                          .SetParentId(endpointEntry4.parentId)
                          .SetServerClusters(Span<ServerClusterInterface *>(serverClustersArray))
                          .SetClientClusters(Span<const ClusterId>())
                          .SetSemanticTags(Span<const SemanticTag>())
                          .SetDeviceTypes(Span<const DataModel::DeviceTypeEntry>())
                          .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_pair));
    SpanEndpoint ep = std::move(std::get<SpanEndpoint>(build_pair));

    ReadOnlyBufferBuilder<ServerClusterInterface *> scBuilder;
    EXPECT_EQ(ep.ServerClusters(scBuilder), CHIP_NO_ERROR);
    EXPECT_EQ(scBuilder.Size(), 1u);
}

TEST_F(TestCodeDrivenDataModelProvider, EndpointSetInvalidServerCluster)
{
    ServerClusterInterface * serverClustersArrayWithNull[] = { &descriptorClusterEP1, nullptr };

    {
        auto build_pair = SpanEndpoint::Builder(endpointEntry4.id)
                              .SetComposition(endpointEntry4.compositionPattern)
                              .SetParentId(endpointEntry4.parentId)
                              .SetServerClusters(chip::Span<ServerClusterInterface *>(serverClustersArrayWithNull))
                              .Build();
        ASSERT_TRUE(std::holds_alternative<CHIP_ERROR>(build_pair));
        EXPECT_EQ(std::get<CHIP_ERROR>(build_pair), CHIP_ERROR_INVALID_ARGUMENT);
    }
}

TEST_F(TestCodeDrivenDataModelProvider, ClusterStartupIsCalledWhenAddingToStartedProvider)
{
    // Use a local provider, started up, to test adding an endpoint to an already started provider.
    // This ensures its lifecycle (including Shutdown) is fully contained within this test,
    // happening before local stack objects like testCluster are destroyed.
    CodeDrivenDataModelProvider localProvider;
    localProvider.SetPersistentStorageDelegate(&mServerClusterTestContext.StorageDelegate());
    ASSERT_EQ(localProvider.Startup(mContext), CHIP_NO_ERROR); // Provider is now "already started"

    MockServerCluster testCluster(ConcreteClusterPath(endpointEntry1.id, 100), 1, BitFlags<DataModel::ClusterQualityFlags>());
    ASSERT_FALSE(testCluster.startupCalled);

    ServerClusterInterface * serverClustersArray[] = { &descriptorClusterEP1, &testCluster };

    auto build_pair = SpanEndpoint::Builder(endpointEntry1.id)
                          .SetComposition(endpointEntry1.compositionPattern)
                          .SetParentId(endpointEntry1.parentId)
                          .SetServerClusters(Span<ServerClusterInterface *>(serverClustersArray))
                          .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_pair));
    auto epProvider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair)));
    // Keep epProvider alive for the scope of localProvider by storing it locally.
    std::vector<std::unique_ptr<SpanEndpoint>> localEndpointStorage;
    localEndpointStorage.push_back(std::move(epProvider));

    EndpointInterfaceRegistration registration(*localEndpointStorage.back());
    ASSERT_EQ(localProvider.AddEndpoint(registration), CHIP_NO_ERROR);

    EXPECT_TRUE(testCluster.startupCalled);

    localProvider.Shutdown(); // Explicitly shutdown the local provider before local objects (testCluster, epProvider in
                              // localEndpointStorage) are destroyed.
}

TEST_F(TestCodeDrivenDataModelProvider, ClusterStartupNotCalledWhenAddingToNonStartedProviderThenCalledOnProviderStartup)
{
    CodeDrivenDataModelProvider localProvider; // Not started yet
    localProvider.SetPersistentStorageDelegate(&mServerClusterTestContext.StorageDelegate());

    MockServerCluster testCluster(ConcreteClusterPath(endpointEntry1.id, 101), 1, BitFlags<DataModel::ClusterQualityFlags>());
    ASSERT_FALSE(testCluster.startupCalled);

    ServerClusterInterface * serverClustersArray[] = { &descriptorClusterEP1, &testCluster };

    auto build_pair = SpanEndpoint::Builder(endpointEntry1.id)
                          .SetComposition(endpointEntry1.compositionPattern)
                          .SetParentId(endpointEntry1.parentId)
                          .SetServerClusters(Span<ServerClusterInterface *>(serverClustersArray))
                          .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_pair));
    auto epProvider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair)));
    std::vector<std::unique_ptr<SpanEndpoint>> localEndpointStorage;
    localEndpointStorage.push_back(std::move(epProvider));

    EndpointInterfaceRegistration registration(*localEndpointStorage.back());
    ASSERT_EQ(localProvider.AddEndpoint(registration), CHIP_NO_ERROR);

    EXPECT_FALSE(
        testCluster.startupCalled); // Startup on cluster should not be called directly when adding to a non-started provider

    // Now startup the provider
    ASSERT_EQ(localProvider.Startup(mContext), CHIP_NO_ERROR);
    EXPECT_TRUE(testCluster.startupCalled); // Should be called now

    localProvider.Shutdown();
}

TEST_F(TestCodeDrivenDataModelProvider, ClusterShutdownIsCalledWhenRemovingFromStartedProvider)
{
    // Use a local provider to ensure its lifecycle is fully contained within this test,
    // avoiding potential issues with the fixture's provider shutdown timing relative to
    // local stack objects like testCluster.
    CodeDrivenDataModelProvider localProvider;
    localProvider.SetPersistentStorageDelegate(&mServerClusterTestContext.StorageDelegate());
    ASSERT_EQ(localProvider.Startup(mContext), CHIP_NO_ERROR);

    MockServerCluster testCluster(ConcreteClusterPath(endpointEntry1.id, 102), 1, BitFlags<DataModel::ClusterQualityFlags>());
    ServerClusterInterface * serverClustersArray[] = { &descriptorClusterEP1, &testCluster };

    auto build_pair = SpanEndpoint::Builder(endpointEntry1.id)
                          .SetComposition(endpointEntry1.compositionPattern)
                          .SetParentId(endpointEntry1.parentId)
                          .SetServerClusters(Span<ServerClusterInterface *>(serverClustersArray))
                          .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_pair));
    auto epProvider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair)));

    // Add the endpoint provider to the localProvider.
    // The localProvider will call Startup() on testCluster's clusters if it's already started.
    std::vector<std::unique_ptr<SpanEndpoint>> localEndpointStorage; // Keep epProvider alive
    localEndpointStorage.push_back(std::move(epProvider));
    EndpointInterfaceRegistration registration(*localEndpointStorage.back());
    ASSERT_EQ(localProvider.AddEndpoint(registration), CHIP_NO_ERROR);

    ASSERT_TRUE(testCluster.startupCalled);
    ASSERT_FALSE(testCluster.shutdownCalled);

    // Remove the endpoint from the localProvider. This should call Shutdown() on testCluster.
    ASSERT_EQ(localProvider.RemoveEndpoint(endpointEntry1.id), CHIP_NO_ERROR);
    EXPECT_TRUE(testCluster.shutdownCalled);

    localProvider
        .Shutdown(); // Explicitly shutdown the local provider before local objects (testCluster, epProvider) are destroyed.
}

TEST_F(TestCodeDrivenDataModelProvider, ClusterShutdownNotCalledWhenRemovingFromNonStartedProvider)
{
    CodeDrivenDataModelProvider localProvider; // Not started
    localProvider.SetPersistentStorageDelegate(&mServerClusterTestContext.StorageDelegate());

    MockServerCluster testCluster(ConcreteClusterPath(endpointEntry1.id, 103), 1, BitFlags<DataModel::ClusterQualityFlags>());
    ServerClusterInterface * serverClustersArray[] = { &descriptorClusterEP1, &testCluster };

    auto build_pair = SpanEndpoint::Builder(endpointEntry1.id)
                          .SetComposition(endpointEntry1.compositionPattern)
                          .SetParentId(endpointEntry1.parentId)
                          .SetServerClusters(Span<ServerClusterInterface *>(serverClustersArray))
                          .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_pair));
    auto epProvider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair)));
    std::vector<std::unique_ptr<SpanEndpoint>> localEndpointStorage;
    localEndpointStorage.push_back(std::move(epProvider));

    EndpointInterfaceRegistration registration(*localEndpointStorage.back());
    ASSERT_EQ(localProvider.AddEndpoint(registration), CHIP_NO_ERROR);

    ASSERT_FALSE(testCluster.startupCalled);
    ASSERT_FALSE(testCluster.shutdownCalled);

    ASSERT_EQ(localProvider.RemoveEndpoint(endpointEntry1.id), CHIP_NO_ERROR);
    EXPECT_FALSE(testCluster.shutdownCalled); // Should not be called as provider was never started

    // localProvider will be destructed, which calls Shutdown, but the endpoint is already removed.
}

// Check that multiple paths for a single cluster are handled correctly
TEST_F(TestCodeDrivenDataModelProvider, ServerClustersMultiPath)
{
    // mockServerCluster1 and mockServerCluster2 are global.
    static ServerClusterInterface * sServerClustersArray[] = { &descriptorClusterEP1, &mockServerClusterMultiPath };

    auto build_pair = SpanEndpoint::Builder(endpointEntry1.id)
                          .SetComposition(endpointEntry1.compositionPattern)
                          .SetParentId(endpointEntry1.parentId)
                          .SetServerClusters(Span<ServerClusterInterface *>(sServerClustersArray))
                          .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_pair));
    auto epProvider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair)));

    mEndpointStorage.push_back(std::move(epProvider));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back()));
    ASSERT_EQ(mProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::ServerClusterEntry> builder;
    ASSERT_EQ(mProvider.ServerClusters(endpointEntry1.id, builder), CHIP_NO_ERROR);

    auto serverClusters = builder.TakeBuffer();
    ASSERT_EQ(serverClusters.size(), 3u);

    EXPECT_EQ(serverClusters[0].clusterId, descriptorClusterEP1.GetPaths()[0].mClusterId);
    EXPECT_EQ(serverClusters[0].dataVersion, descriptorClusterEP1.GetDataVersion({}));
    EXPECT_EQ(serverClusters[0].flags, descriptorClusterEP1.GetClusterFlags({}));

    EXPECT_EQ(serverClusters[1].clusterId, mockServerClusterMultiPath.GetPaths()[0].mClusterId);
    EXPECT_EQ(serverClusters[1].dataVersion, mockServerClusterMultiPath.GetDataVersion({}));
    EXPECT_EQ(serverClusters[1].flags, mockServerClusterMultiPath.GetClusterFlags({}));

    EXPECT_EQ(serverClusters[2].clusterId, mockServerClusterMultiPath.GetPaths()[1].mClusterId);
    EXPECT_EQ(serverClusters[2].dataVersion, mockServerClusterMultiPath.GetDataVersion({}));
    EXPECT_EQ(serverClusters[2].flags, mockServerClusterMultiPath.GetClusterFlags({}));
}

TEST_F(TestCodeDrivenDataModelProvider, ReadAttribute)
{
    static MockServerCluster descriptorCluster({ 1, chip::app::Clusters::Descriptor::Id }, 1, {});
    static MockServerCluster testCluster({ 1, 10 }, 1, {});
    static ServerClusterInterface * serverClusters[] = { &descriptorCluster, &testCluster };
    auto build_pair = SpanEndpoint::Builder(1).SetServerClusters(Span<ServerClusterInterface *>(serverClusters)).Build();
    auto epProvider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair)));
    mEndpointStorage.push_back(std::move(epProvider));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back()));
    mProvider.AddEndpoint(*mOwnedRegistrations.back());

    uint32_t expectedValue = testCluster.mAttributeValue; // Default value set in MockServerCluster
    uint32_t readValue;
    ReadU32Attribute(mProvider, ConcreteDataAttributePath(1, 10, 1), readValue);
    EXPECT_EQ(expectedValue, readValue);
}

TEST_F(TestCodeDrivenDataModelProvider, WriteAttribute)
{
    static MockServerCluster descriptorCluster({ 1, chip::app::Clusters::Descriptor::Id }, 1, {});
    static MockServerCluster testCluster({ 1, 10 }, 1, {});
    static ServerClusterInterface * serverClusters[] = { &descriptorCluster, &testCluster };
    auto build_pair = SpanEndpoint::Builder(1).SetServerClusters(Span<ServerClusterInterface *>(serverClusters)).Build();
    auto epProvider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair)));
    mEndpointStorage.push_back(std::move(epProvider));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back()));
    mProvider.AddEndpoint(*mOwnedRegistrations.back());

    auto path             = ConcreteDataAttributePath(1, 10, 1);
    uint32_t valueToWrite = 123;
    EXPECT_EQ(WriteU32Attribute(mProvider, ConcreteDataAttributePath(1, 10, 1), valueToWrite), CHIP_NO_ERROR);
    EXPECT_EQ(testCluster.mLastWriteRequest.path, path);
    EXPECT_EQ(testCluster.mLastWriteRequest.path, path);

    // Read back the value to verify it was written correctly
    uint32_t readValue;
    ReadU32Attribute(mProvider, ConcreteDataAttributePath(1, 10, 1), readValue);
    EXPECT_EQ(readValue, valueToWrite);
}

TEST_F(TestCodeDrivenDataModelProvider, InvokeCommand)
{
    static MockServerCluster descriptorCluster({ 1, chip::app::Clusters::Descriptor::Id }, 1, {});
    static MockServerCluster testCluster({ 1, 10 }, 1, {});
    static ServerClusterInterface * serverClusters[] = { &descriptorCluster, &testCluster };
    auto build_pair = SpanEndpoint::Builder(1).SetServerClusters(Span<ServerClusterInterface *>(serverClusters)).Build();
    auto epProvider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair)));
    mEndpointStorage.push_back(std::move(epProvider));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back()));
    mProvider.AddEndpoint(*mOwnedRegistrations.back());

    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(128);
    TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());
    DataModel::InvokeRequest request = { .path = ConcreteCommandPath(1, 10, 1) };
    auto result                      = mProvider.InvokeCommand(request, reader, nullptr);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value().GetUnderlyingError(), CHIP_NO_ERROR);
    EXPECT_EQ(testCluster.mLastInvokeRequest.path, request.path);
}

TEST_F(TestCodeDrivenDataModelProvider, IterateOverAttributes)
{
    static MockServerCluster descriptorCluster({ 1, chip::app::Clusters::Descriptor::Id }, 1, {});
    static MockServerCluster testCluster({ 1, 10 }, 1, {});
    static ServerClusterInterface * serverClusters[] = { &descriptorCluster, &testCluster };
    auto build_pair = SpanEndpoint::Builder(1).SetServerClusters(Span<ServerClusterInterface *>(serverClusters)).Build();
    auto epProvider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair)));
    mEndpointStorage.push_back(std::move(epProvider));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back()));
    mProvider.AddEndpoint(*mOwnedRegistrations.back());

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> builder;
    EXPECT_EQ(mProvider.Attributes(ConcreteClusterPath(1, 10), builder), CHIP_NO_ERROR);
    auto attributes = builder.TakeBuffer();
    EXPECT_EQ(attributes.size(), 1u);
    EXPECT_EQ(attributes[0].attributeId, testCluster.mAttributeEntry.attributeId);
}

TEST_F(TestCodeDrivenDataModelProvider, IterateOverAcceptedCommands)
{
    static MockServerCluster descriptorCluster({ 1, chip::app::Clusters::Descriptor::Id }, 1, {});
    static MockServerCluster testCluster({ 1, 10 }, 1, {});
    static ServerClusterInterface * serverClusters[] = { &descriptorCluster, &testCluster };
    auto build_pair = SpanEndpoint::Builder(1).SetServerClusters(Span<ServerClusterInterface *>(serverClusters)).Build();
    auto epProvider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair)));
    mEndpointStorage.push_back(std::move(epProvider));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back()));
    mProvider.AddEndpoint(*mOwnedRegistrations.back());

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> builder;
    EXPECT_EQ(mProvider.AcceptedCommands(ConcreteClusterPath(1, 10), builder), CHIP_NO_ERROR);
    auto commands = builder.TakeBuffer();
    EXPECT_EQ(commands.size(), 1u);
    EXPECT_EQ(commands[0].commandId, testCluster.mAcceptedCommandEntry.commandId);
}

TEST_F(TestCodeDrivenDataModelProvider, IterateOverGeneratedCommands)
{
    static MockServerCluster descriptorCluster({ 1, chip::app::Clusters::Descriptor::Id }, 1, {});
    static MockServerCluster testCluster({ 1, 10 }, 1, {});
    static ServerClusterInterface * serverClusters[] = { &descriptorCluster, &testCluster };
    auto build_pair = SpanEndpoint::Builder(1).SetServerClusters(Span<ServerClusterInterface *>(serverClusters)).Build();
    auto epProvider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair)));
    mEndpointStorage.push_back(std::move(epProvider));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back()));
    mProvider.AddEndpoint(*mOwnedRegistrations.back());

    ReadOnlyBufferBuilder<CommandId> builder;
    EXPECT_EQ(mProvider.GeneratedCommands(ConcreteClusterPath(1, 10), builder), CHIP_NO_ERROR);
    auto commands = builder.TakeBuffer();
    EXPECT_EQ(commands.size(), 1u);
    EXPECT_EQ(commands[0], testCluster.mGeneratedCommandId);
}

TEST_F(TestCodeDrivenDataModelProvider, GetEventInfo)
{
    static MockServerCluster descriptorCluster({ 1, chip::app::Clusters::Descriptor::Id }, 1, {});
    static MockServerCluster testCluster({ 1, 10 }, 1, {});
    static ServerClusterInterface * serverClusters[] = { &descriptorCluster, &testCluster };
    auto build_pair = SpanEndpoint::Builder(1).SetServerClusters(Span<ServerClusterInterface *>(serverClusters)).Build();
    auto epProvider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair)));
    mEndpointStorage.push_back(std::move(epProvider));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back()));
    mProvider.AddEndpoint(*mOwnedRegistrations.back());

    DataModel::EventEntry eventInfo;
    testCluster.mEventEntry = { .readPrivilege = Access::Privilege::kView };
    EXPECT_EQ(mProvider.EventInfo(ConcreteEventPath(1, 10, 1), eventInfo), CHIP_NO_ERROR);
    EXPECT_EQ(eventInfo.readPrivilege, testCluster.mEventEntry.readPrivilege);
}

TEST_F(TestCodeDrivenDataModelProvider, ListAttributeWriteNotification)
{
    static MockServerCluster descriptorCluster({ 1, chip::app::Clusters::Descriptor::Id }, 1, {});
    static MockServerCluster testCluster({ 1, 10 }, 1, {});
    static ServerClusterInterface * serverClusters[] = { &descriptorCluster, &testCluster };
    auto build_pair = SpanEndpoint::Builder(1).SetServerClusters(Span<ServerClusterInterface *>(serverClusters)).Build();
    auto epProvider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair)));
    mEndpointStorage.push_back(std::move(epProvider));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back()));
    mProvider.AddEndpoint(*mOwnedRegistrations.back());

    ConcreteAttributePath path(1, 10, 1);
    mProvider.ListAttributeWriteNotification(path, DataModel::ListWriteOperation::kListWriteSuccess);
    EXPECT_TRUE(testCluster.mLastListWriteOpPath.has_value());
    EXPECT_EQ(testCluster.mLastListWriteOpPath.value(), path);
    EXPECT_TRUE(testCluster.mLastListWriteOpType.has_value());
    EXPECT_EQ(testCluster.mLastListWriteOpType.value(), DataModel::ListWriteOperation::kListWriteSuccess);
}

TEST_F(TestCodeDrivenDataModelProvider, Temporary_ReportAttributeChanged)
{
    static MockServerCluster descriptorCluster({ 1, chip::app::Clusters::Descriptor::Id }, 1, {});
    static MockServerCluster testCluster({ 1, 10 }, 1, {});
    static ServerClusterInterface * serverClusters[] = { &descriptorCluster, &testCluster };
    auto build_pair = SpanEndpoint::Builder(1).SetServerClusters(Span<ServerClusterInterface *>(serverClusters)).Build();
    auto epProvider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair)));
    mEndpointStorage.push_back(std::move(epProvider));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back()));
    mProvider.AddEndpoint(*mOwnedRegistrations.back());

    AttributePathParams path(1, 10, 1);
    mProvider.Temporary_ReportAttributeChanged(path);
    ASSERT_EQ(mChangeListener.mDirtyList.size(), 1u);
    EXPECT_EQ(mChangeListener.mDirtyList[0].mEndpointId, path.mEndpointId);
    EXPECT_EQ(mChangeListener.mDirtyList[0].mClusterId, path.mClusterId);
    EXPECT_EQ(mChangeListener.mDirtyList[0].mAttributeId, path.mAttributeId);
}

TEST_F(TestCodeDrivenDataModelProvider, GetAndSetPersistentStorageDelegate)
{
    chip::Test::TestServerClusterContext otherTestContext;
    mProvider.SetPersistentStorageDelegate(&otherTestContext.StorageDelegate());
    EXPECT_EQ(mProvider.GetPersistentStorageDelegate(), &otherTestContext.StorageDelegate());
    // TODO: Add more checks to ensure the delegate is used correctly in operations.
    // For example, you could check if the delegate is used in a read/write operation.
    // This would require implementing a mock or a spy for the persistent storage delegate.
    // For now, we just check that the delegate can be set and retrieved without error.
    mProvider.SetPersistentStorageDelegate(nullptr); // Reset to default
    EXPECT_EQ(mProvider.GetPersistentStorageDelegate(), nullptr);
}

TEST_F(TestCodeDrivenDataModelProvider, Shutdown)
{
    static MockServerCluster testCluster(ConcreteClusterPath(endpointEntry1.id, 102), 1,
                                         BitFlags<DataModel::ClusterQualityFlags>());
    static ServerClusterInterface * serverClustersArray[] = { &descriptorClusterEP1, &testCluster };

    auto build_pair = SpanEndpoint::Builder(endpointEntry1.id)
                          .SetComposition(endpointEntry1.compositionPattern)
                          .SetParentId(endpointEntry1.parentId)
                          .SetServerClusters(Span<ServerClusterInterface *>(serverClustersArray))
                          .Build();
    ASSERT_TRUE(std::holds_alternative<SpanEndpoint>(build_pair));
    auto epProvider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair)));

    mEndpointStorage.push_back(std::move(epProvider));
    auto registration = std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back());
    ASSERT_EQ(mProvider.AddEndpoint(*registration), CHIP_NO_ERROR);
    mOwnedRegistrations.push_back(std::move(registration));

    mProvider.Shutdown();
    EXPECT_TRUE(testCluster.shutdownCalled);
}

TEST_F(TestCodeDrivenDataModelProvider, ReadAttributeOnInvalidPath)
{
    static MockServerCluster descriptorCluster({ 1, chip::app::Clusters::Descriptor::Id }, 1, {});
    static ServerClusterInterface * serverClusters[] = { &descriptorCluster };
    auto build_pair = SpanEndpoint::Builder(1).SetServerClusters(Span<ServerClusterInterface *>(serverClusters)).Build();
    auto epProvider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair)));
    mEndpointStorage.push_back(std::move(epProvider));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back()));
    mProvider.AddEndpoint(*mOwnedRegistrations.back());

    uint32_t readValue;
    // Valid endpoint and cluster, but invalid attribute ID
    EXPECT_EQ(ReadU32Attribute(mProvider, ConcreteDataAttributePath(1, chip::app::Clusters::Descriptor::Id, 555), readValue),
              CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));

    // Valid endpoint but invalid cluster
    EXPECT_EQ(ReadU32Attribute(mProvider, ConcreteDataAttributePath(1, 99, 1), readValue),
              CHIP_IM_GLOBAL_STATUS(UnsupportedCluster));

    // Invalid endpoint
    EXPECT_EQ(ReadU32Attribute(mProvider, ConcreteDataAttributePath(99, chip::app::Clusters::Descriptor::Id, 1), readValue),
              CHIP_IM_GLOBAL_STATUS(UnsupportedEndpoint));
}

TEST_F(TestCodeDrivenDataModelProvider, WriteAttributeOnInvalidPath)
{
    static MockServerCluster descriptorCluster({ 1, chip::app::Clusters::Descriptor::Id }, 1, {});
    static MockServerCluster testCluster({ 1, 10 }, 1, {});
    static ServerClusterInterface * serverClusters[] = { &descriptorCluster, &testCluster };
    auto build_pair = SpanEndpoint::Builder(1).SetServerClusters(Span<ServerClusterInterface *>(serverClusters)).Build();
    auto epProvider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair)));
    mEndpointStorage.push_back(std::move(epProvider));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back()));
    mProvider.AddEndpoint(*mOwnedRegistrations.back());

    uint32_t valueToWrite = 123;
    // Valid endpoint and cluster, but invalid attribute ID
    EXPECT_EQ(WriteU32Attribute(mProvider, ConcreteDataAttributePath(1, 10, 99), valueToWrite),
              CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));

    // Valid endpoint but invalid cluster
    EXPECT_EQ(WriteU32Attribute(mProvider, ConcreteDataAttributePath(1, 99, 1), valueToWrite),
              CHIP_IM_GLOBAL_STATUS(UnsupportedCluster));

    // Invalid endpoint
    EXPECT_EQ(WriteU32Attribute(mProvider, ConcreteDataAttributePath(99, 10, 1), valueToWrite),
              CHIP_IM_GLOBAL_STATUS(UnsupportedEndpoint));
}

TEST_F(TestCodeDrivenDataModelProvider, RemoveNonExistentEndpoint)
{
    EXPECT_EQ(mProvider.RemoveEndpoint(999), CHIP_ERROR_NOT_FOUND);
}


TEST_F(TestCodeDrivenDataModelProvider, InvokeCommandOnInvalidEndpoint)
{
    static MockServerCluster descriptorCluster({ 1, chip::app::Clusters::Descriptor::Id }, 1, {});
    static MockServerCluster testCluster({ 1, 10 }, 1, {});
    static ServerClusterInterface * serverClusters[] = { &descriptorCluster, &testCluster };
    auto build_pair = SpanEndpoint::Builder(1).SetServerClusters(Span<ServerClusterInterface *>(serverClusters)).Build();
    auto epProvider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair)));
    mEndpointStorage.push_back(std::move(epProvider));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back()));
    mProvider.AddEndpoint(*mOwnedRegistrations.back());

    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(128);
    TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    // Invalid endpoint, but valid cluster
    DataModel::InvokeRequest requestUnsupportedEndpoint = { .path = ConcreteCommandPath(5, 10, 1) };
    auto result                                         = mProvider.InvokeCommand(requestUnsupportedEndpoint, reader, nullptr);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value().GetUnderlyingError(), CHIP_IM_GLOBAL_STATUS(UnsupportedEndpoint));
}

TEST_F(TestCodeDrivenDataModelProvider, InvokeCommandOnInvalidCluster)
{
    static MockServerCluster descriptorCluster({ 1, chip::app::Clusters::Descriptor::Id }, 1, {});
    static MockServerCluster testCluster({ 1, 10 }, 1, {});
    static ServerClusterInterface * serverClusters[] = { &descriptorCluster, &testCluster };
    auto build_pair = SpanEndpoint::Builder(1).SetServerClusters(Span<ServerClusterInterface *>(serverClusters)).Build();
    auto epProvider = std::make_unique<SpanEndpoint>(std::move(std::get<SpanEndpoint>(build_pair)));
    mEndpointStorage.push_back(std::move(epProvider));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back()));
    mProvider.AddEndpoint(*mOwnedRegistrations.back());

    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(128);
    TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    // Invalid cluster ID but valid endpoint
    DataModel::InvokeRequest requestUnsupportedCluster = { .path = ConcreteCommandPath(1, 99, 1) };
    auto result                                        = mProvider.InvokeCommand(requestUnsupportedCluster, reader, nullptr);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value().GetUnderlyingError(), CHIP_IM_GLOBAL_STATUS(UnsupportedCluster));
}
