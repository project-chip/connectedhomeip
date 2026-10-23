/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/AttributeValueEncoder.h>
#include <app/InteractionModelEngine.h>
#include <app/ReadClient.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/testing/EmptyProvider.h>
#include <app/tests/AppTestContext.h>
#include <clusters/OperationalCredentials/Structs.h>
#include <clusters/TlsCertificateManagement/Metadata.h>
#include <clusters/TlsClientManagement/Metadata.h>
#include <clusters/WebRTCTransportProvider/Metadata.h>
#include <clusters/WebRTCTransportRequestor/Metadata.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/tests/ExtraPwTestMacros.h>

#include <map>
#include <optional>

namespace {

using namespace chip;
using namespace chip::app;

constexpr EndpointId kEndpoint             = 1;
constexpr ClusterId kCluster               = 0xFFF1FC01;
constexpr AttributeId kFabricSensitiveList = 1;
constexpr AttributeId kFabricScopedList    = 2;

// Three rows, one on the accessing fabric and two on other fabrics.
constexpr size_t kRowCount = 3;

using ListEntryType = Clusters::OperationalCredentials::Structs::FabricDescriptorStruct::Type;

/// Exposes two list attributes of fabric-scoped entries. Both read the same rows;
/// only kFabricSensitiveList carries the fabric-sensitive quality flag.
class FabricScopedListProvider : public chip::Testing::EmptyProvider
{
public:
    CHIP_ERROR Endpoints(ReadOnlyBufferBuilder<DataModel::EndpointEntry> & builder) override
    {
        ReturnErrorOnFailure(builder.EnsureAppendCapacity(1));
        return builder.Append({ kEndpoint, kInvalidEndpointId, DataModel::EndpointCompositionPattern::kFullFamily });
    }

    CHIP_ERROR ServerClusters(EndpointId endpointId, ReadOnlyBufferBuilder<DataModel::ServerClusterEntry> & builder) override
    {
        VerifyOrReturnError(endpointId == kEndpoint, CHIP_IM_GLOBAL_STATUS(UnsupportedEndpoint));
        ReturnErrorOnFailure(builder.EnsureAppendCapacity(1));
        return builder.Append({ kCluster, 1 /* dataVersion */, {} });
    }

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override
    {
        VerifyOrReturnError(path.mEndpointId == kEndpoint, CHIP_IM_GLOBAL_STATUS(UnsupportedEndpoint));
        VerifyOrReturnError(path.mClusterId == kCluster, CHIP_IM_GLOBAL_STATUS(UnsupportedCluster));

        using Flags = DataModel::AttributeQualityFlags;
        ReturnErrorOnFailure(builder.EnsureAppendCapacity(2));
        ReturnErrorOnFailure(builder.Append(
            DataModel::AttributeEntry(kFabricSensitiveList, BitFlags<Flags>(Flags::kListAttribute, Flags::kFabricSensitive),
                                      Access::Privilege::kView, std::nullopt)));
        return builder.Append(DataModel::AttributeEntry(kFabricScopedList, BitFlags<Flags>(Flags::kListAttribute),
                                                        Access::Privilege::kView, std::nullopt));
    }

    ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request, AttributeValueEncoder & encoder) override
    {
        VerifyOrReturnError(request.path.mEndpointId == kEndpoint, Protocols::InteractionModel::Status::UnsupportedEndpoint);
        VerifyOrReturnError(request.path.mClusterId == kCluster, Protocols::InteractionModel::Status::UnsupportedCluster);
        VerifyOrReturnError(request.path.mAttributeId == kFabricSensitiveList || request.path.mAttributeId == kFabricScopedList,
                            Protocols::InteractionModel::Status::UnsupportedAttribute);

        const FabricIndex accessing = request.subjectDescriptor.fabricIndex;
        VerifyOrReturnError(accessing != kUndefinedFabricIndex, Protocols::InteractionModel::Status::Failure);

        return encoder.EncodeList([accessing](const auto & listEncoder) -> CHIP_ERROR {
            for (size_t i = 0; i < kRowCount; i++)
            {
                ListEntryType row;
                // Row 0 belongs to the accessing fabric; the others sit on fabrics that
                // are guaranteed distinct from it and valid (1..254).
                row.fabricIndex = static_cast<FabricIndex>(((accessing - 1 + i) % 254) + 1);
                ReturnErrorOnFailure(listEncoder.Encode(row));
            }
            return CHIP_NO_ERROR;
        });
    }
};

class ListCountingCallback : public ReadClient::Callback
{
public:
    void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) override
    {
        if (!aStatus.IsSuccess())
        {
            mFailureStatus = aStatus.mStatus;
            ChipLogError(Test, "Attribute " ChipLogFormatMEI " status 0x%02x", ChipLogValueMEI(aPath.mAttributeId),
                         to_underlying(aStatus.mStatus));
            return;
        }
        if (apData == nullptr)
        {
            mError = CHIP_ERROR_INVALID_ARGUMENT;
            return;
        }
        VerifyOrReturn(aPath.IsListOperation() && !aPath.IsListItemOperation());

        TLV::TLVType containerType;
        VerifyOrReturn(apData->EnterContainer(containerType) == CHIP_NO_ERROR);
        size_t count = 0;
        VerifyOrReturn(TLV::Utilities::Count(*apData, count, /* aRecurse = */ false) == CHIP_NO_ERROR);
        mListSizes[aPath.mAttributeId] = count;
    }

    void OnError(CHIP_ERROR aError) override { mError = aError; }
    void OnDone(ReadClient *) override { mDone = true; }

    std::optional<size_t> ListSize(AttributeId id) const
    {
        auto it = mListSizes.find(id);
        return it == mListSizes.end() ? std::nullopt : std::optional<size_t>(it->second);
    }

    std::map<AttributeId, size_t> mListSizes;
    std::optional<Protocols::InteractionModel::Status> mFailureStatus;
    CHIP_ERROR mError = CHIP_NO_ERROR;
    bool mDone        = false;
};

class TestFabricSensitiveAttributeRead : public chip::Testing::AppContext
{
public:
    void SetUp() override
    {
        AppContext::SetUp();
        mOldProvider = InteractionModelEngine::GetInstance()->SetDataModelProvider(&mProvider);
    }

    void TearDown() override
    {
        InteractionModelEngine::GetInstance()->SetDataModelProvider(mOldProvider);
        AppContext::TearDown();
    }

    void ReadBothLists(bool fabricFiltered, ListCountingCallback & callback)
    {
        AttributePathParams paths[2];
        paths[0] = AttributePathParams(kEndpoint, kCluster, kFabricSensitiveList);
        paths[1] = AttributePathParams(kEndpoint, kCluster, kFabricScopedList);

        ReadPrepareParams params(GetSessionBobToAlice());
        params.mpAttributePathParamsList    = paths;
        params.mAttributePathParamsListSize = 2;
        params.mIsFabricFiltered            = fabricFiltered;

        ReadClient readClient(InteractionModelEngine::GetInstance(), &GetExchangeManager(), callback,
                              ReadClient::InteractionType::Read);
        ASSERT_SUCCESS(readClient.SendRequest(params));
        DrainAndServiceIO();

        ASSERT_SUCCESS(callback.mError);
        ASSERT_TRUE(callback.mDone);
        ASSERT_FALSE(callback.mFailureStatus.has_value());
    }

private:
    FabricScopedListProvider mProvider;
    DataModel::Provider * mOldProvider = nullptr;
};

TEST_F(TestFabricSensitiveAttributeRead, UnfilteredReadIsStillFilteredForFabricSensitiveAttribute)
{
    ListCountingCallback callback;
    ReadBothLists(/* fabricFiltered = */ false, callback);

    // The plain fabric-scoped list honors the request flag and returns every row.
    ASSERT_TRUE(callback.ListSize(kFabricScopedList).has_value());
    EXPECT_EQ(callback.ListSize(kFabricScopedList).value_or(0), kRowCount);

    // The fabric-sensitive list is filtered to the accessing fabric regardless of the flag.
    ASSERT_TRUE(callback.ListSize(kFabricSensitiveList).has_value());
    EXPECT_EQ(callback.ListSize(kFabricSensitiveList).value_or(0), 1u);
}

TEST_F(TestFabricSensitiveAttributeRead, FilteredReadFiltersBothLists)
{
    ListCountingCallback callback;
    ReadBothLists(/* fabricFiltered = */ true, callback);

    ASSERT_TRUE(callback.ListSize(kFabricScopedList).has_value());
    EXPECT_EQ(callback.ListSize(kFabricScopedList).value_or(0), 1u);
    ASSERT_TRUE(callback.ListSize(kFabricSensitiveList).has_value());
    EXPECT_EQ(callback.ListSize(kFabricSensitiveList).value_or(0), 1u);
}

// Every attribute the data model marks fabric-sensitive must carry the quality in
// its generated metadata, otherwise the reporting engine cannot enforce it.
// PushAvStreamTransport::CurrentConnections is not listed yet: its XML has not been
// regenerated with an Alchemy release that emits the marker.
static_assert(Clusters::WebRTCTransportProvider::Attributes::CurrentSessions::kMetadataEntry.HasFlags(
    DataModel::AttributeQualityFlags::kFabricSensitive));
static_assert(Clusters::WebRTCTransportRequestor::Attributes::CurrentSessions::kMetadataEntry.HasFlags(
    DataModel::AttributeQualityFlags::kFabricSensitive));
static_assert(Clusters::TlsCertificateManagement::Attributes::ProvisionedRootCertificates::kMetadataEntry.HasFlags(
    DataModel::AttributeQualityFlags::kFabricSensitive));
static_assert(Clusters::TlsCertificateManagement::Attributes::ProvisionedClientCertificates::kMetadataEntry.HasFlags(
    DataModel::AttributeQualityFlags::kFabricSensitive));
static_assert(Clusters::TlsClientManagement::Attributes::ProvisionedEndpoints::kMetadataEntry.HasFlags(
    DataModel::AttributeQualityFlags::kFabricSensitive));

} // namespace
