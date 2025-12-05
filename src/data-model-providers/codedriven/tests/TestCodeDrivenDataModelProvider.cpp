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
#include <app/server-cluster/testing/TestEventGenerator.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/Descriptor/ClusterId.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <data-model-providers/codedriven/endpoint/SpanEndpoint.h>
#include <lib/core/TLV.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <lib/support/Span.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <algorithm>
#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::Testing;

class TestProviderChangeListener : public DataModel::ProviderChangeListener
{
public:
    void MarkDirty(const AttributePathParams & path) override { mDirtyList.push_back(path); }
    std::vector<AttributePathParams> mDirtyList;
};

class TestActionContext : public DataModel::ActionContext
{
public:
    Messaging::ExchangeContext * CurrentExchange() override { return nullptr; }
};

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

    void ListAttributeWriteNotification(const ConcreteAttributePath & path, DataModel::ListWriteOperation opType,
                                        FabricIndex accessingFabric) override
    {
        mLastListWriteOpPath            = path;
        mLastListWriteOpType            = opType;
        mLastListWriteOpAccessingFabric = accessingFabric;
    }

    CHIP_ERROR Startup(ServerClusterContext & context) override
    {
        startupCallCount++;
        return DefaultServerCluster::Startup(context);
    }

    void Shutdown() override
    {
        shutdownCallCount++;
        DefaultServerCluster::Shutdown();
    }

    int startupCallCount  = 0;
    int shutdownCallCount = 0;

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
    std::optional<FabricIndex> mLastListWriteOpAccessingFabric;
};

class TestCodeDrivenDataModelProvider : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

protected:
    ::TestProviderChangeListener mChangeListener;
    chip::Testing::LogOnlyEvents mEventGenerator;
    TestActionContext mActionContext;
    DataModel::InteractionModelContext mContext{
        .eventsGenerator         = mEventGenerator,
        .dataModelChangeListener = mChangeListener,
        .actionContext           = mActionContext,
    };
    chip::Testing::TestServerClusterContext mServerClusterTestContext;
    CodeDrivenDataModelProvider mProvider;
    std::vector<std::unique_ptr<SpanEndpoint>> mEndpointStorage;                     // To keep providers alive
    std::vector<std::unique_ptr<EndpointInterfaceRegistration>> mOwnedRegistrations; // To keep registration objects alive

    TestCodeDrivenDataModelProvider() :
        mProvider(mServerClusterTestContext.StorageDelegate(), mServerClusterTestContext.AttributePersistenceProvider())
    {
        EXPECT_EQ(mProvider.Startup(mContext), CHIP_NO_ERROR);
    }

    ~TestCodeDrivenDataModelProvider() override
    {
        EXPECT_SUCCESS(mProvider.Shutdown());
        mEndpointStorage.clear();
        mOwnedRegistrations.clear();
    }
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

constexpr chip::EndpointId clientClusterId1 = 1;
constexpr chip::EndpointId clientClusterId2 = 2;

// Define kMax constants for testing purposes
constexpr unsigned int kTestMaxDeviceTypes = 5;

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
    // Create 3 SpanEndpoints with different IDs
    // EP1
    auto endpoint1 = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());

    // EP2
    auto endpoint2 = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());

    // EP3
    auto endpoint3 = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());

    mEndpointStorage.push_back(std::move(endpoint1));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back(), endpointEntry1));
    ASSERT_EQ(mProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);
    mEndpointStorage.push_back(std::move(endpoint2));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back(), endpointEntry2));
    ASSERT_EQ(mProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);
    mEndpointStorage.push_back(std::move(endpoint3));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back(), endpointEntry3));
    ASSERT_EQ(mProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::EndpointEntry> endpointsBuilder;

    ASSERT_EQ(mProvider.Endpoints(endpointsBuilder), CHIP_NO_ERROR);

    ReadOnlyBuffer<DataModel::EndpointEntry> endpoints_rb = endpointsBuilder.TakeBuffer();
    std::vector<DataModel::EndpointEntry> actual_endpoints(endpoints_rb.begin(), endpoints_rb.end());
    std::sort(actual_endpoints.begin(), actual_endpoints.end(),
              [](const DataModel::EndpointEntry & a, const DataModel::EndpointEntry & b) { return a.id < b.id; });

    ASSERT_EQ(actual_endpoints.size(), 3u);
    EXPECT_EQ(actual_endpoints[0], endpointEntry1);
    EXPECT_EQ(actual_endpoints[1], endpointEntry2);
    EXPECT_EQ(actual_endpoints[2], endpointEntry3);
}

TEST_F(TestCodeDrivenDataModelProvider, IterateOverServerClusters)
{
    static MockServerCluster mockServerCluster1(ConcreteClusterPath(1, 10), 1, BitFlags<DataModel::ClusterQualityFlags>());
    static MockServerCluster mockServerCluster2(
        ConcreteClusterPath(1, 20), 2,
        BitFlags<DataModel::ClusterQualityFlags>().Set(DataModel::ClusterQualityFlags::kDiagnosticsData));
    static MockServerCluster descriptorClusterEP1({ endpointEntry1.id, chip::app::Clusters::Descriptor::Id }, 1,
                                                  BitFlags<DataModel::ClusterQualityFlags>());

    static ServerClusterRegistration registration1(mockServerCluster1);
    static ServerClusterRegistration registration2(mockServerCluster2);
    static ServerClusterRegistration registration3(descriptorClusterEP1);

    ASSERT_EQ(mProvider.AddCluster(registration1), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.AddCluster(registration2), CHIP_NO_ERROR);
    ASSERT_EQ(mProvider.AddCluster(registration3), CHIP_NO_ERROR);

    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back(), endpointEntry1));
    ASSERT_EQ(mProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::ServerClusterEntry> builder;
    ASSERT_EQ(mProvider.ServerClusters(endpointEntry1.id, builder), CHIP_NO_ERROR);

    auto serverClusters = builder.TakeBuffer();
    ASSERT_EQ(serverClusters.size(), 3u);
}

TEST_F(TestCodeDrivenDataModelProvider, IterateOverClientClusters)
{
    static const ClusterId sClientClustersArray[] = { clientClusterId1, clientClusterId2 };

    auto endpoint = std::make_unique<SpanEndpoint>(
        SpanEndpoint::Builder().SetClientClusters(Span<const ClusterId>(sClientClustersArray)).Build());

    mEndpointStorage.push_back(std::move(endpoint));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back(), endpointEntry1));
    ASSERT_EQ(mProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<chip::ClusterId> builder;
    ASSERT_EQ(mProvider.ClientClusters(endpointEntry1.id, builder), CHIP_NO_ERROR);

    auto clientClusters = builder.TakeBuffer();
    ASSERT_EQ(clientClusters.size(), 2u);
    EXPECT_EQ(clientClusters[0], clientClusterId1);
    EXPECT_EQ(clientClusters[1], clientClusterId2);
}

TEST_F(TestCodeDrivenDataModelProvider, IterateOverDeviceTypes)
{
    static DataModel::DeviceTypeEntry sDeviceTypesData[kTestMaxDeviceTypes];
    for (unsigned int i = 0; i < kTestMaxDeviceTypes; i++)
    {
        sDeviceTypesData[i].deviceTypeId       = static_cast<uint32_t>(i);
        sDeviceTypesData[i].deviceTypeRevision = static_cast<uint8_t>((i % 255) + 1);
    }

    auto endpoint = std::make_unique<SpanEndpoint>(
        SpanEndpoint::Builder().SetDeviceTypes(Span<const DataModel::DeviceTypeEntry>(sDeviceTypesData)).Build());

    mEndpointStorage.push_back(std::move(endpoint));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back(), endpointEntry1));
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
    auto endpoint1 = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());

    auto endpoint2 = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());

    auto endpoint3 = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());

    mEndpointStorage.push_back(std::move(endpoint1));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back(), endpointEntry1));
    ASSERT_EQ(mProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);
    mEndpointStorage.push_back(std::move(endpoint2));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back(), endpointEntry2));
    ASSERT_EQ(mProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);
    mEndpointStorage.push_back(std::move(endpoint3));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back(), endpointEntry3));
    ASSERT_EQ(mProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);

    EXPECT_EQ(mProvider.RemoveEndpoint(endpointEntry1.id), CHIP_NO_ERROR);
    EXPECT_EQ(mProvider.RemoveEndpoint(endpointEntry3.id), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::EndpointEntry> out;
    EXPECT_EQ(mProvider.Endpoints(out), CHIP_NO_ERROR);
    EXPECT_EQ(out.Size(), 1U);
    auto endpoints = out.TakeBuffer();
    EXPECT_EQ(endpoints[0].id, endpointEntry2.id);
}

TEST_F(TestCodeDrivenDataModelProvider, EndpointWithStaticData)
{
    CodeDrivenDataModelProvider localProvider(mServerClusterTestContext.StorageDelegate(),
                                              mServerClusterTestContext.AttributePersistenceProvider());
    ASSERT_EQ(localProvider.Startup(mContext), CHIP_NO_ERROR);

    static const ClusterId clientClustersArray[]               = { 0xD001, 0xD002 };
    static const DataModel::DeviceTypeEntry deviceTypesArray[] = { { .deviceTypeId = 0x7001, .deviceTypeRevision = 1 },
                                                                   { .deviceTypeId = 0x7002, .deviceTypeRevision = 2 } };

    SpanEndpoint ep = SpanEndpoint::Builder()
                          .SetClientClusters(chip::Span<const ClusterId>(clientClustersArray))
                          .SetDeviceTypes(chip::Span<const DataModel::DeviceTypeEntry>(deviceTypesArray))
                          .Build();

    EndpointInterfaceRegistration registration(ep, endpointEntry4);
    ASSERT_EQ(localProvider.AddEndpoint(registration), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::EndpointEntry> epBuilder;
    ASSERT_EQ(localProvider.Endpoints(epBuilder), CHIP_NO_ERROR);
    auto eps = epBuilder.TakeBuffer();
    ASSERT_EQ(eps.size(), 1u);
    EXPECT_EQ(eps[0].id, endpointEntry4.id);

    EXPECT_SUCCESS(localProvider.Shutdown());
}

TEST_F(TestCodeDrivenDataModelProvider, EndpointWithEmptyStaticData)
{
    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder()
                                                       .SetClientClusters(Span<const ClusterId>())
                                                       .SetDeviceTypes(Span<const DataModel::DeviceTypeEntry>())
                                                       .Build());
    mEndpointStorage.push_back(std::move(endpoint));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back(), endpointEntry4));
    ASSERT_EQ(mProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);
}

TEST_F(TestCodeDrivenDataModelProvider, AddClusterFailsIfEndpointExists)
{
    CodeDrivenDataModelProvider localProvider(mServerClusterTestContext.StorageDelegate(),
                                              mServerClusterTestContext.AttributePersistenceProvider());
    ASSERT_EQ(localProvider.Startup(mContext), CHIP_NO_ERROR);

    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back(), endpointEntry1));
    ASSERT_EQ(localProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);

    MockServerCluster testCluster(ConcreteClusterPath(endpointEntry1.id, 100), 1, {});
    ASSERT_EQ(testCluster.startupCallCount, 0);

    static ServerClusterRegistration registration(testCluster);
    EXPECT_EQ(localProvider.AddCluster(registration), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(testCluster.startupCallCount, 0);

    EXPECT_SUCCESS(localProvider.Shutdown());
}

TEST_F(TestCodeDrivenDataModelProvider, AddClusterDoesntStartCluster)
{
    CodeDrivenDataModelProvider localProvider(mServerClusterTestContext.StorageDelegate(),
                                              mServerClusterTestContext.AttributePersistenceProvider());
    ASSERT_EQ(localProvider.Startup(mContext), CHIP_NO_ERROR);

    MockServerCluster testCluster(ConcreteClusterPath(endpointEntry1.id, 100), 1, {});
    ASSERT_EQ(testCluster.startupCallCount, 0);

    static ServerClusterRegistration registration(testCluster);
    EXPECT_EQ(localProvider.AddCluster(registration), CHIP_NO_ERROR);
    EXPECT_EQ(testCluster.startupCallCount, 0);

    EXPECT_SUCCESS(localProvider.Shutdown());
}

TEST_F(TestCodeDrivenDataModelProvider, AddClusterWithEmptyPathFails)
{
    CodeDrivenDataModelProvider localProvider(mServerClusterTestContext.StorageDelegate(),
                                              mServerClusterTestContext.AttributePersistenceProvider());
    ASSERT_EQ(localProvider.Startup(mContext), CHIP_NO_ERROR);

    MockServerCluster testCluster({}, 1, {});
    static ServerClusterRegistration registration(testCluster);
    EXPECT_EQ(localProvider.AddCluster(registration), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestCodeDrivenDataModelProvider, ClusterStartupNotCalledWhenAddingToNonStartedProviderThenCalledOnProviderStartup)
{
    CodeDrivenDataModelProvider localProvider(mServerClusterTestContext.StorageDelegate(),
                                              mServerClusterTestContext.AttributePersistenceProvider());

    MockServerCluster testCluster(ConcreteClusterPath(endpointEntry1.id, 101), 1, {});
    ASSERT_EQ(testCluster.startupCallCount, 0);

    static ServerClusterRegistration registration(testCluster);
    ASSERT_EQ(localProvider.AddCluster(registration), CHIP_NO_ERROR);
    EXPECT_EQ(testCluster.startupCallCount, 0);

    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back(), endpointEntry1));
    ASSERT_EQ(localProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);
    EXPECT_EQ(testCluster.startupCallCount, 0);

    ASSERT_EQ(localProvider.Startup(mContext), CHIP_NO_ERROR);
    EXPECT_EQ(testCluster.startupCallCount, 1);

    EXPECT_SUCCESS(localProvider.Shutdown());
}

TEST_F(TestCodeDrivenDataModelProvider, RemoveClusterFailsIfEndpointExists)
{
    CodeDrivenDataModelProvider localProvider(mServerClusterTestContext.StorageDelegate(),
                                              mServerClusterTestContext.AttributePersistenceProvider());
    ASSERT_EQ(localProvider.Startup(mContext), CHIP_NO_ERROR);

    MockServerCluster testCluster(ConcreteClusterPath(endpointEntry1.id, 102), 1, {});
    static ServerClusterRegistration registration(testCluster);
    ASSERT_EQ(localProvider.AddCluster(registration), CHIP_NO_ERROR);

    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back(), endpointEntry1));
    ASSERT_EQ(localProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);

    EXPECT_EQ(localProvider.RemoveCluster(&testCluster), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(testCluster.shutdownCallCount, 0);

    EXPECT_SUCCESS(localProvider.Shutdown());
}

TEST_F(TestCodeDrivenDataModelProvider, RemoveClusterSucceedsIfEndpointRemoved)
{
    CodeDrivenDataModelProvider localProvider(mServerClusterTestContext.StorageDelegate(),
                                              mServerClusterTestContext.AttributePersistenceProvider());
    ASSERT_EQ(localProvider.Startup(mContext), CHIP_NO_ERROR);

    MockServerCluster testCluster(ConcreteClusterPath(endpointEntry1.id, 102), 1, {});
    static ServerClusterRegistration registration(testCluster);
    ASSERT_EQ(localProvider.AddCluster(registration), CHIP_NO_ERROR);

    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back(), endpointEntry1));
    ASSERT_EQ(localProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);

    ASSERT_EQ(localProvider.RemoveEndpoint(endpointEntry1.id), CHIP_NO_ERROR);
    EXPECT_EQ(localProvider.RemoveCluster(&testCluster), CHIP_NO_ERROR);
    EXPECT_EQ(testCluster.shutdownCallCount, 1);

    EXPECT_SUCCESS(localProvider.Shutdown());
}

TEST_F(TestCodeDrivenDataModelProvider, ClusterShutdownNotCalledWhenRemovingFromNonStartedProvider)
{
    CodeDrivenDataModelProvider localProvider(mServerClusterTestContext.StorageDelegate(),
                                              mServerClusterTestContext.AttributePersistenceProvider());

    MockServerCluster testCluster(ConcreteClusterPath(endpointEntry1.id, 103), 1, {});
    static ServerClusterRegistration registration(testCluster);
    ASSERT_EQ(localProvider.AddCluster(registration), CHIP_NO_ERROR);
    ASSERT_EQ(testCluster.startupCallCount, 0);
    ASSERT_EQ(testCluster.shutdownCallCount, 0);

    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back(), endpointEntry1));
    ASSERT_EQ(localProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);

    ASSERT_EQ(localProvider.RemoveEndpoint(endpointEntry1.id), CHIP_NO_ERROR);
    ASSERT_EQ(localProvider.RemoveCluster(&testCluster), CHIP_NO_ERROR);
    EXPECT_EQ(testCluster.shutdownCallCount, 0);
}

TEST_F(TestCodeDrivenDataModelProvider, ServerClustersMultiPath)
{
    static MockServerCluster mockServerClusterMultiPath({ { 3, 40 }, { 3, 50 } }, 1, {});
    static ServerClusterRegistration registration(mockServerClusterMultiPath);
    ASSERT_EQ(mProvider.AddCluster(registration), CHIP_NO_ERROR);

    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back(), endpointEntry3));
    ASSERT_EQ(mProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::ServerClusterEntry> builder;
    ASSERT_EQ(mProvider.ServerClusters(3, builder), CHIP_NO_ERROR);

    auto serverClusters = builder.TakeBuffer();
    ASSERT_EQ(serverClusters.size(), 2u);
}

TEST_F(TestCodeDrivenDataModelProvider, ReadAttribute)
{
    static MockServerCluster testCluster({ 1, 10 }, 1, {});
    static ServerClusterRegistration registration(testCluster);
    EXPECT_SUCCESS(mProvider.AddCluster(registration));

    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(
        *mEndpointStorage.back(), DataModel::EndpointEntry{ .id = 1, .compositionPattern = EndpointCompositionPattern(0) }));
    EXPECT_SUCCESS(mProvider.AddEndpoint(*mOwnedRegistrations.back()));

    uint32_t expectedValue = testCluster.mAttributeValue;
    uint32_t readValue;
    EXPECT_SUCCESS(ReadU32Attribute(mProvider, ConcreteDataAttributePath(1, 10, 1), readValue));
    EXPECT_EQ(expectedValue, readValue);
}

TEST_F(TestCodeDrivenDataModelProvider, WriteAttribute)
{
    static MockServerCluster testCluster({ 1, 10 }, 1, {});
    static ServerClusterRegistration registration(testCluster);
    EXPECT_SUCCESS(mProvider.AddCluster(registration));

    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(
        *mEndpointStorage.back(), DataModel::EndpointEntry{ .id = 1, .compositionPattern = EndpointCompositionPattern(0) }));
    EXPECT_SUCCESS(mProvider.AddEndpoint(*mOwnedRegistrations.back()));

    auto path             = ConcreteDataAttributePath(1, 10, 1);
    uint32_t valueToWrite = 123;
    EXPECT_EQ(WriteU32Attribute(mProvider, path, valueToWrite), CHIP_NO_ERROR);
    EXPECT_EQ(testCluster.mLastWriteRequest.path, path);

    uint32_t readValue;
    EXPECT_SUCCESS(ReadU32Attribute(mProvider, path, readValue));
    EXPECT_EQ(readValue, valueToWrite);
}

TEST_F(TestCodeDrivenDataModelProvider, InvokeCommand)
{
    static MockServerCluster testCluster({ 1, 10 }, 1, {});
    static ServerClusterRegistration registration(testCluster);
    EXPECT_SUCCESS(mProvider.AddCluster(registration));

    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(
        *mEndpointStorage.back(), DataModel::EndpointEntry{ .id = 1, .compositionPattern = EndpointCompositionPattern(0) }));
    EXPECT_SUCCESS(mProvider.AddEndpoint(*mOwnedRegistrations.back()));

    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(128);
    TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());
    DataModel::InvokeRequest request = { .path = ConcreteCommandPath(1, 10, 1) };
    auto result                      = mProvider.InvokeCommand(request, reader, nullptr);
    EXPECT_TRUE(result.has_value());
    if (result.has_value())
    {
        EXPECT_EQ(result.value().GetUnderlyingError(), CHIP_NO_ERROR);
    }
    EXPECT_EQ(testCluster.mLastInvokeRequest.path, request.path);
}

TEST_F(TestCodeDrivenDataModelProvider, IterateOverAttributes)
{
    static MockServerCluster testCluster({ 1, 10 }, 1, {});
    static ServerClusterRegistration registration(testCluster);
    EXPECT_SUCCESS(mProvider.AddCluster(registration));

    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(
        *mEndpointStorage.back(), DataModel::EndpointEntry{ .id = 1, .compositionPattern = EndpointCompositionPattern(0) }));
    EXPECT_SUCCESS(mProvider.AddEndpoint(*mOwnedRegistrations.back()));

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> builder;
    EXPECT_EQ(mProvider.Attributes(ConcreteClusterPath(1, 10), builder), CHIP_NO_ERROR);
    auto attributes = builder.TakeBuffer();
    EXPECT_EQ(attributes.size(), 1u);
    EXPECT_EQ(attributes[0].attributeId, testCluster.mAttributeEntry.attributeId);
}

TEST_F(TestCodeDrivenDataModelProvider, IterateOverAcceptedCommands)
{
    static MockServerCluster testCluster({ 1, 10 }, 1, {});
    static ServerClusterRegistration registration(testCluster);
    EXPECT_SUCCESS(mProvider.AddCluster(registration));

    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(
        *mEndpointStorage.back(), DataModel::EndpointEntry{ .id = 1, .compositionPattern = EndpointCompositionPattern(0) }));
    EXPECT_SUCCESS(mProvider.AddEndpoint(*mOwnedRegistrations.back()));

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> builder;
    EXPECT_EQ(mProvider.AcceptedCommands(ConcreteClusterPath(1, 10), builder), CHIP_NO_ERROR);
    auto commands = builder.TakeBuffer();
    EXPECT_EQ(commands.size(), 1u);
    EXPECT_EQ(commands[0].commandId, testCluster.mAcceptedCommandEntry.commandId);
}

TEST_F(TestCodeDrivenDataModelProvider, IterateOverGeneratedCommands)
{
    static MockServerCluster testCluster({ 1, 10 }, 1, {});
    static ServerClusterRegistration registration(testCluster);
    EXPECT_SUCCESS(mProvider.AddCluster(registration));

    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(
        *mEndpointStorage.back(), DataModel::EndpointEntry{ .id = 1, .compositionPattern = EndpointCompositionPattern(0) }));
    EXPECT_SUCCESS(mProvider.AddEndpoint(*mOwnedRegistrations.back()));

    ReadOnlyBufferBuilder<CommandId> builder;
    EXPECT_EQ(mProvider.GeneratedCommands(ConcreteClusterPath(1, 10), builder), CHIP_NO_ERROR);
    auto commands = builder.TakeBuffer();
    EXPECT_EQ(commands.size(), 1u);
    EXPECT_EQ(commands[0], testCluster.mGeneratedCommandId);
}

TEST_F(TestCodeDrivenDataModelProvider, GetEventInfo)
{
    static MockServerCluster testCluster({ 1, 10 }, 1, {});
    static ServerClusterRegistration registration(testCluster);
    EXPECT_SUCCESS(mProvider.AddCluster(registration));

    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(
        *mEndpointStorage.back(), DataModel::EndpointEntry{ .id = 1, .compositionPattern = EndpointCompositionPattern(0) }));
    EXPECT_SUCCESS(mProvider.AddEndpoint(*mOwnedRegistrations.back()));

    DataModel::EventEntry eventInfo;
    testCluster.mEventEntry = { .readPrivilege = Access::Privilege::kView };
    EXPECT_EQ(mProvider.EventInfo(ConcreteEventPath(1, 10, 1), eventInfo), CHIP_NO_ERROR);
    EXPECT_EQ(eventInfo.readPrivilege, testCluster.mEventEntry.readPrivilege);
}

TEST_F(TestCodeDrivenDataModelProvider, ListAttributeWriteNotification)
{
    static MockServerCluster testCluster({ 1, 10 }, 1, {});
    static ServerClusterRegistration registration(testCluster);
    EXPECT_SUCCESS(mProvider.AddCluster(registration));

    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(
        *mEndpointStorage.back(), DataModel::EndpointEntry{ .id = 1, .compositionPattern = EndpointCompositionPattern(0) }));
    EXPECT_SUCCESS(mProvider.AddEndpoint(*mOwnedRegistrations.back()));

    ConcreteAttributePath path(1, 10, 1);
    mProvider.ListAttributeWriteNotification(path, DataModel::ListWriteOperation::kListWriteSuccess, 1);
    ASSERT_TRUE(testCluster.mLastListWriteOpPath.has_value());
    if (testCluster.mLastListWriteOpPath)
    {
        EXPECT_EQ(testCluster.mLastListWriteOpPath.value(), path);
    }
    ASSERT_TRUE(testCluster.mLastListWriteOpType.has_value());
    if (testCluster.mLastListWriteOpType)
    {
        EXPECT_EQ(testCluster.mLastListWriteOpType.value(), DataModel::ListWriteOperation::kListWriteSuccess);
    }
    ASSERT_TRUE(testCluster.mLastListWriteOpAccessingFabric.has_value());
    if (testCluster.mLastListWriteOpAccessingFabric)
    {
        EXPECT_EQ(testCluster.mLastListWriteOpAccessingFabric.value(), 1);
    }
}

TEST_F(TestCodeDrivenDataModelProvider, Temporary_ReportAttributeChanged)
{
    static MockServerCluster testCluster({ 1, 10 }, 1, {});
    static ServerClusterRegistration registration(testCluster);
    EXPECT_SUCCESS(mProvider.AddCluster(registration));

    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(
        *mEndpointStorage.back(), DataModel::EndpointEntry{ .id = 1, .compositionPattern = EndpointCompositionPattern(0) }));
    EXPECT_SUCCESS(mProvider.AddEndpoint(*mOwnedRegistrations.back()));

    AttributePathParams path(1, 10, 1);
    mProvider.Temporary_ReportAttributeChanged(path);
    ASSERT_EQ(mChangeListener.mDirtyList.size(), 1u);
    EXPECT_EQ(mChangeListener.mDirtyList[0].mEndpointId, path.mEndpointId);
    EXPECT_EQ(mChangeListener.mDirtyList[0].mClusterId, path.mClusterId);
    EXPECT_EQ(mChangeListener.mDirtyList[0].mAttributeId, path.mAttributeId);
}

TEST_F(TestCodeDrivenDataModelProvider, Shutdown)
{
    static MockServerCluster testCluster(ConcreteClusterPath(endpointEntry1.id, 102), 1, {});
    static ServerClusterRegistration cluster_registration(testCluster);
    ASSERT_EQ(mProvider.AddCluster(cluster_registration), CHIP_NO_ERROR);

    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint));
    auto registration = std::make_unique<EndpointInterfaceRegistration>(
        *mEndpointStorage.back(), DataModel::EndpointEntry{ .id = 1, .compositionPattern = EndpointCompositionPattern(0) });
    ASSERT_EQ(mProvider.AddEndpoint(*registration), CHIP_NO_ERROR);
    mOwnedRegistrations.push_back(std::move(registration));

    ASSERT_EQ(mProvider.RemoveEndpoint(endpointEntry1.id), CHIP_NO_ERROR);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
    EXPECT_EQ(testCluster.shutdownCallCount, 1);
}

TEST_F(TestCodeDrivenDataModelProvider, ReadAttributeOnInvalidPath)
{
    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(
        *mEndpointStorage.back(), DataModel::EndpointEntry{ .id = 1, .compositionPattern = EndpointCompositionPattern(0) }));
    EXPECT_SUCCESS(mProvider.AddEndpoint(*mOwnedRegistrations.back()));

    uint32_t readValue;
    EXPECT_EQ(ReadU32Attribute(mProvider, ConcreteDataAttributePath(1, 99, 1), readValue), CHIP_ERROR_KEY_NOT_FOUND);
    EXPECT_EQ(ReadU32Attribute(mProvider, ConcreteDataAttributePath(99, 1, 1), readValue), CHIP_ERROR_KEY_NOT_FOUND);
}

TEST_F(TestCodeDrivenDataModelProvider, WriteAttributeOnInvalidPath)
{
    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(
        *mEndpointStorage.back(), DataModel::EndpointEntry{ .id = 1, .compositionPattern = EndpointCompositionPattern(0) }));
    EXPECT_SUCCESS(mProvider.AddEndpoint(*mOwnedRegistrations.back()));

    uint32_t valueToWrite = 123;
    EXPECT_EQ(WriteU32Attribute(mProvider, ConcreteDataAttributePath(1, 99, 1), valueToWrite), CHIP_ERROR_KEY_NOT_FOUND);
    EXPECT_EQ(WriteU32Attribute(mProvider, ConcreteDataAttributePath(99, 1, 1), valueToWrite), CHIP_ERROR_KEY_NOT_FOUND);
}

TEST_F(TestCodeDrivenDataModelProvider, RemoveNonExistentEndpoint)
{
    EXPECT_EQ(mProvider.RemoveEndpoint(999), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestCodeDrivenDataModelProvider, InvokeCommandOnInvalidEndpoint)
{
    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(
        *mEndpointStorage.back(), DataModel::EndpointEntry{ .id = 1, .compositionPattern = EndpointCompositionPattern(0) }));
    EXPECT_SUCCESS(mProvider.AddEndpoint(*mOwnedRegistrations.back()));

    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(128);
    TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    DataModel::InvokeRequest requestUnsupportedEndpoint = { .path = ConcreteCommandPath(5, 10, 1) };
    auto result                                         = mProvider.InvokeCommand(requestUnsupportedEndpoint, reader, nullptr);
    ASSERT_TRUE(result.has_value());
    if (result)
    {
        EXPECT_EQ(result.value().GetUnderlyingError(), CHIP_ERROR_KEY_NOT_FOUND);
    }
}

TEST_F(TestCodeDrivenDataModelProvider, InvokeCommandOnInvalidCluster)
{
    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(
        *mEndpointStorage.back(), DataModel::EndpointEntry{ .id = 1, .compositionPattern = EndpointCompositionPattern(0) }));
    EXPECT_SUCCESS(mProvider.AddEndpoint(*mOwnedRegistrations.back()));

    System::PacketBufferHandle buffer = System::PacketBufferHandle::New(128);
    TLV::TLVReader reader;
    reader.Init(buffer->Start(), buffer->DataLength());

    DataModel::InvokeRequest requestUnsupportedCluster = { .path = ConcreteCommandPath(1, 99, 1) };
    auto result                                        = mProvider.InvokeCommand(requestUnsupportedCluster, reader, nullptr);
    ASSERT_TRUE(result.has_value());
    if (result)
    {
        EXPECT_EQ(result.value().GetUnderlyingError(), CHIP_ERROR_KEY_NOT_FOUND);
    }
}

TEST_F(TestCodeDrivenDataModelProvider, SingleServerClusterInterfaceWithMultipleClustersAndMultipleEndpoints)
{
    static MockServerCluster multiPathCluster({ { 1, 10 }, { 2, 30 } }, 1, {});
    static ServerClusterRegistration registration(multiPathCluster);
    ASSERT_EQ(mProvider.AddCluster(registration), CHIP_NO_ERROR);

    auto endpoint1 = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint1));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(
        *mEndpointStorage.back(), DataModel::EndpointEntry{ .id = 1, .compositionPattern = EndpointCompositionPattern(0) }));
    ASSERT_EQ(mProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);

    auto endpoint2 = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint2));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(
        *mEndpointStorage.back(), DataModel::EndpointEntry{ .id = 2, .compositionPattern = EndpointCompositionPattern(0) }));
    ASSERT_EQ(mProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);

    {
        ReadOnlyBufferBuilder<DataModel::ServerClusterEntry> builder;
        ASSERT_EQ(mProvider.ServerClusters(1, builder), CHIP_NO_ERROR);
        auto serverClusters = builder.TakeBuffer();
        ASSERT_EQ(serverClusters.size(), 1u);
        EXPECT_EQ(serverClusters[0].clusterId, 10u);
    }

    {
        ReadOnlyBufferBuilder<DataModel::ServerClusterEntry> builder;
        ASSERT_EQ(mProvider.ServerClusters(2, builder), CHIP_NO_ERROR);
        auto serverClusters = builder.TakeBuffer();
        ASSERT_EQ(serverClusters.size(), 1u);
        EXPECT_EQ(serverClusters[0].clusterId, 30u);
    }

    {
        ReadOnlyBufferBuilder<DataModel::ServerClusterEntry> builder;
        EXPECT_EQ(mProvider.ServerClusters(99, builder), CHIP_IM_GLOBAL_STATUS(UnsupportedEndpoint));
    }
}

TEST_F(TestCodeDrivenDataModelProvider, AddEndpointWithKInvalidId)
{
    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());

    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(
        *endpoint, DataModel::EndpointEntry{ .id = kInvalidEndpointId, .compositionPattern = EndpointCompositionPattern(0) }));
    EndpointInterfaceRegistration * registration = mOwnedRegistrations.back().get();

    EXPECT_EQ(mProvider.AddEndpoint(*registration), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestCodeDrivenDataModelProvider, AddDuplicateEndpointId)
{
    mEndpointStorage.push_back(std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build()));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(
        *mEndpointStorage.back(), DataModel::EndpointEntry{ .id = 1, .compositionPattern = EndpointCompositionPattern(0) }));
    EndpointInterfaceRegistration * registration1 = mOwnedRegistrations.back().get();

    mEndpointStorage.push_back(std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build()));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(
        *mEndpointStorage.back(), DataModel::EndpointEntry{ .id = 1, .compositionPattern = EndpointCompositionPattern(0) }));
    EndpointInterfaceRegistration * registration2 = mOwnedRegistrations.back().get();

    EXPECT_EQ(mProvider.AddEndpoint(*registration1), CHIP_NO_ERROR);
    EXPECT_EQ(mProvider.AddEndpoint(*registration2), CHIP_ERROR_DUPLICATE_KEY_ID);
}

TEST_F(TestCodeDrivenDataModelProvider, AddEndpointStartsClusterWithMultipleEndpoints)
{
    CodeDrivenDataModelProvider localProvider(mServerClusterTestContext.StorageDelegate(),
                                              mServerClusterTestContext.AttributePersistenceProvider());
    ASSERT_EQ(localProvider.Startup(mContext), CHIP_NO_ERROR);

    MockServerCluster testCluster({ { 1, 100 }, { 2, 100 } }, 1, {});
    static ServerClusterRegistration registration(testCluster);
    ASSERT_EQ(localProvider.AddCluster(registration), CHIP_NO_ERROR);
    ASSERT_EQ(testCluster.startupCallCount, 0);

    auto endpoint1 = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint1));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back(), endpointEntry1));
    ASSERT_EQ(localProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);
    ASSERT_EQ(testCluster.startupCallCount, 1); // Cluster should start on first endpoint

    auto endpoint2 = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint2));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back(), endpointEntry2));
    ASSERT_EQ(localProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);
    ASSERT_EQ(testCluster.startupCallCount, 1); // Should not start again

    EXPECT_SUCCESS(localProvider.Shutdown());
}

TEST_F(TestCodeDrivenDataModelProvider, RemoveEndpointShutsDownClusterWithMultipleEndpoints)
{
    CodeDrivenDataModelProvider localProvider(mServerClusterTestContext.StorageDelegate(),
                                              mServerClusterTestContext.AttributePersistenceProvider());
    ASSERT_EQ(localProvider.Startup(mContext), CHIP_NO_ERROR);

    MockServerCluster testCluster({ { 1, 100 }, { 2, 100 } }, 1, {});
    static ServerClusterRegistration registration(testCluster);
    ASSERT_EQ(localProvider.AddCluster(registration), CHIP_NO_ERROR);

    auto endpoint1 = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint1));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back(), endpointEntry1));
    ASSERT_EQ(localProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);

    auto endpoint2 = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint2));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back(), endpointEntry2));
    ASSERT_EQ(localProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);
    ASSERT_EQ(testCluster.startupCallCount, 1);

    ASSERT_EQ(localProvider.RemoveEndpoint(endpointEntry1.id), CHIP_NO_ERROR);
    ASSERT_EQ(testCluster.shutdownCallCount, 0);

    ASSERT_EQ(localProvider.RemoveEndpoint(endpointEntry2.id), CHIP_NO_ERROR);
    ASSERT_EQ(testCluster.shutdownCallCount, 1);

    EXPECT_SUCCESS(localProvider.Shutdown());
}

TEST_F(TestCodeDrivenDataModelProvider, RemoveEndpointThenRemoveClusterCallsShutdownOnce)
{
    CodeDrivenDataModelProvider localProvider(mServerClusterTestContext.StorageDelegate(),
                                              mServerClusterTestContext.AttributePersistenceProvider());
    ASSERT_EQ(localProvider.Startup(mContext), CHIP_NO_ERROR);

    MockServerCluster testCluster(ConcreteClusterPath(endpointEntry1.id, 102), 1, {});
    static ServerClusterRegistration registration(testCluster);
    ASSERT_EQ(localProvider.AddCluster(registration), CHIP_NO_ERROR);

    auto endpoint = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back(), endpointEntry1));
    ASSERT_EQ(localProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);
    ASSERT_EQ(testCluster.startupCallCount, 1);

    ASSERT_EQ(localProvider.RemoveEndpoint(endpointEntry1.id), CHIP_NO_ERROR);
    ASSERT_EQ(localProvider.RemoveCluster(&testCluster), CHIP_NO_ERROR);
    EXPECT_EQ(testCluster.shutdownCallCount, 1);

    EXPECT_SUCCESS(localProvider.Shutdown());
}

TEST_F(TestCodeDrivenDataModelProvider, StartupOnlyStartsClustersWithRegisteredEndpoints)
{
    CodeDrivenDataModelProvider localProvider(mServerClusterTestContext.StorageDelegate(),
                                              mServerClusterTestContext.AttributePersistenceProvider());

    MockServerCluster clusterWithRegisteredEndpoint(ConcreteClusterPath(endpointEntry1.id, 100), 1, {});
    static ServerClusterRegistration registration1(clusterWithRegisteredEndpoint);
    ASSERT_EQ(localProvider.AddCluster(registration1), CHIP_NO_ERROR);

    MockServerCluster clusterWithoutRegisteredEndpoint(ConcreteClusterPath(endpointEntry2.id, 200), 1, {});
    static ServerClusterRegistration registration2(clusterWithoutRegisteredEndpoint);
    ASSERT_EQ(localProvider.AddCluster(registration2), CHIP_NO_ERROR);

    auto endpoint1 = std::make_unique<SpanEndpoint>(SpanEndpoint::Builder().Build());
    mEndpointStorage.push_back(std::move(endpoint1));
    mOwnedRegistrations.push_back(std::make_unique<EndpointInterfaceRegistration>(*mEndpointStorage.back(), endpointEntry1));
    ASSERT_EQ(localProvider.AddEndpoint(*mOwnedRegistrations.back()), CHIP_NO_ERROR);

    ASSERT_EQ(localProvider.Startup(mContext), CHIP_NO_ERROR);

    EXPECT_EQ(clusterWithRegisteredEndpoint.startupCallCount, 1);
    EXPECT_EQ(clusterWithoutRegisteredEndpoint.startupCallCount, 0);

    EXPECT_SUCCESS(localProvider.Shutdown());
}
