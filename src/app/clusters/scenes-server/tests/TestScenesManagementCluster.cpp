/**
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/scenes-server/SceneTableImpl.h>
#include <app/clusters/scenes-server/ScenesManagementCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/ScenesManagement/Commands.h>
#include <clusters/ScenesManagement/Metadata.h>
#include <clusters/ScenesManagement/Structs.h>
#include <credentials/GroupDataProvider.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <credentials/tests/CHIPCert_test_vectors.h>
#include <credentials/tests/CHIPCert_unit_test_vectors.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/TypeTraits.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ScenesManagement;
using namespace chip::Credentials;
using namespace chip::Protocols::InteractionModel;
using namespace chip::Testing;
using namespace chip::scenes;
using namespace chip::app::Clusters::ScenesManagement::Attributes;
using namespace chip::app::Clusters::ScenesManagement::Commands;
using namespace chip::app::Clusters::ScenesManagement::Structs;
using namespace chip::app::DataModel;

constexpr EndpointId kTestEndpointId = 123;
constexpr FabricIndex kFabricIndex   = 1;
constexpr FabricIndex kFabricIndex2  = 2;
constexpr FabricIndex kFabricIndex3  = 3;
constexpr GroupId kTestGroupId       = 123;
constexpr SceneId kTestSceneId       = 111;
constexpr GroupId kTestOtherGroupId  = 124;
constexpr SceneId kTestOtherSceneId  = 112;
constexpr ClusterId kMockClusterId   = 0xDEAD;

class MockSceneHandler : public scenes::SceneHandler
{
public:
    // Test EFS
    struct MockClusterEFS
    {
        bool on = false;

        CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag) const
        {
            TLV::TLVType outer;
            ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));
            ReturnErrorOnFailure(writer.PutBoolean(TLV::ContextTag(1), on));
            return writer.EndContainer(outer);
        }

        CHIP_ERROR Decode(TLV::TLVReader & reader)
        {
            TLV::TLVType outer;
            ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
            ReturnErrorOnFailure(reader.EnterContainer(outer));
            ReturnErrorOnFailure(reader.Next(TLV::ContextTag(1)));
            ReturnErrorOnFailure(reader.Get(on));
            return reader.ExitContainer(outer);
        }
    };

    MockClusterEFS mState;
    AttributeValuePairStruct::Type mAttributeRead[1]; // Persistent storage for Deserialize

    // SceneHandler Implementation
    bool SupportsCluster(EndpointId endpoint, ClusterId cluster) override { return cluster == kMockClusterId; }

    CHIP_ERROR
    SerializeAdd(EndpointId endpoint,
                 const app::Clusters::ScenesManagement::Structs::ExtensionFieldSetStruct::DecodableType & extensionFieldSet,
                 MutableByteSpan & serialisedBytes) override
    {
        VerifyOrReturnError(extensionFieldSet.clusterID == kMockClusterId, CHIP_ERROR_INVALID_ARGUMENT);

        MockClusterEFS efs_data;

        auto iter = extensionFieldSet.attributeValueList.begin();
        while (iter.Next())
        {
            auto & attr = iter.GetValue();
            if (attr.attributeID == 0 && attr.valueUnsigned8.HasValue()) // Assuming Attribute ID 0 is the 'on' state
            {
                efs_data.on = attr.valueUnsigned8.Value();
            }
        }
        ReturnErrorOnFailure(iter.GetStatus());

        TLV::TLVWriter writer;
        writer.Init(serialisedBytes);
        ReturnErrorOnFailure(efs_data.Encode(writer, TLV::AnonymousTag()));
        serialisedBytes.reduce_size(writer.GetLengthWritten());
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SerializeSave(EndpointId endpoint, ClusterId cluster, MutableByteSpan & serializedBytes) override
    {
        VerifyOrReturnError(cluster == kMockClusterId, CHIP_ERROR_INVALID_ARGUMENT);

        TLV::TLVWriter writer;
        writer.Init(serializedBytes);
        ReturnErrorOnFailure(mState.Encode(writer, TLV::AnonymousTag()));
        serializedBytes.reduce_size(writer.GetLengthWritten());
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Deserialize(EndpointId endpoint, ClusterId cluster, const ByteSpan & serializedBytes,
                           app::Clusters::ScenesManagement::Structs::ExtensionFieldSetStruct::Type & extensionFieldSet) override
    {
        VerifyOrReturnError(cluster == kMockClusterId, CHIP_ERROR_INVALID_ARGUMENT);

        MockClusterEFS deserializedEFS;
        TLV::TLVReader reader;
        reader.Init(serializedBytes);
        ReturnErrorOnFailure(deserializedEFS.Decode(reader));

        extensionFieldSet.clusterID = kMockClusterId;

        mAttributeRead[0].attributeID = 0; // Assuming Attribute ID 0 is the 'on' state
        mAttributeRead[0].valueUnsigned8.SetValue(deserializedEFS.on);

        extensionFieldSet.attributeValueList = DataModel::List<AttributeValuePairStruct::Type>(mAttributeRead);

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ApplyScene(EndpointId endpoint, ClusterId cluster, const ByteSpan & serializedBytes,
                          TransitionTimeMs timeMs) override
    {
        VerifyOrReturnError(cluster == kMockClusterId, CHIP_ERROR_INVALID_ARGUMENT);
        MockClusterEFS deserializedEFS;
        TLV::TLVReader reader;
        reader.Init(serializedBytes);
        ReturnErrorOnFailure(deserializedEFS.Decode(reader));
        mState.on = deserializedEFS.on;
        return CHIP_NO_ERROR;
    }
};

class MockGroupDataProvider : public GroupDataProvider
{
public:
    CHIP_ERROR Init() override { return CHIP_NO_ERROR; }
    void Finish() override {}
    CHIP_ERROR SetGroupInfo(FabricIndex, const GroupInfo &) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetGroupInfo(FabricIndex, GroupId, GroupInfo &) override { return CHIP_ERROR_NOT_FOUND; }
    CHIP_ERROR RemoveGroupInfo(FabricIndex, GroupId) override { return CHIP_NO_ERROR; }
    CHIP_ERROR SetGroupInfoAt(FabricIndex, size_t, const GroupInfo &) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetGroupInfoAt(FabricIndex, size_t, GroupInfo &) override { return CHIP_ERROR_NOT_FOUND; }
    CHIP_ERROR RemoveGroupInfoAt(FabricIndex, size_t) override { return CHIP_NO_ERROR; }
    bool HasEndpoint(FabricIndex fabric, GroupId group, EndpointId endpoint) override { return mHasEndpoint; }
    CHIP_ERROR AddEndpoint(FabricIndex, GroupId, EndpointId) override { return CHIP_NO_ERROR; }
    CHIP_ERROR RemoveEndpoint(FabricIndex, GroupId, EndpointId) override { return CHIP_NO_ERROR; }
    CHIP_ERROR RemoveEndpoint(FabricIndex, EndpointId) override { return CHIP_NO_ERROR; }
    GroupInfoIterator * IterateGroupInfo(FabricIndex) override { return nullptr; }
    EndpointIterator * IterateEndpoints(FabricIndex, std::optional<GroupId>) override { return nullptr; }
    CHIP_ERROR SetGroupKeyAt(FabricIndex, size_t, const GroupKey &) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetGroupKeyAt(FabricIndex, size_t, GroupKey &) override { return CHIP_ERROR_NOT_FOUND; }
    CHIP_ERROR RemoveGroupKeyAt(FabricIndex, size_t) override { return CHIP_NO_ERROR; }
    CHIP_ERROR RemoveGroupKeys(FabricIndex) override { return CHIP_NO_ERROR; }
    GroupKeyIterator * IterateGroupKeys(FabricIndex) override { return nullptr; }
    CHIP_ERROR SetKeySet(FabricIndex, const ByteSpan &, const KeySet &) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetKeySet(FabricIndex, KeysetId, KeySet &) override { return CHIP_ERROR_NOT_FOUND; }
    CHIP_ERROR RemoveKeySet(FabricIndex, KeysetId) override { return CHIP_NO_ERROR; }
    CHIP_ERROR GetIpkKeySet(FabricIndex, KeySet &) override { return CHIP_ERROR_NOT_FOUND; }
    KeySetIterator * IterateKeySets(FabricIndex) override { return nullptr; }
    CHIP_ERROR RemoveFabric(FabricIndex) override { return CHIP_NO_ERROR; }
    GroupSessionIterator * IterateGroupSessions(uint16_t) override { return nullptr; }
    Crypto::SymmetricKeyContext * GetKeyContext(FabricIndex, GroupId) override { return nullptr; }

    bool mHasEndpoint = true;
};

class TestSceneTable : public DefaultSceneTableImpl
{
public:
    CHIP_ERROR SceneSaveEFS(SceneTableEntry & scene) override
    {
        if (HandlerListEmpty())
        {
            return CHIP_NO_ERROR;
        }

        ExtensionFieldSet EFS;
        MutableByteSpan EFSSpan = MutableByteSpan(EFS.mBytesBuffer, kMaxFieldBytesPerCluster);
        EFS.mID                 = kMockClusterId;

        for (auto & handler : mHandlerList)
        {
            if (handler.SupportsCluster(mEndpointId, kMockClusterId))
            {
                ReturnErrorOnFailure(handler.SerializeSave(mEndpointId, EFS.mID, EFSSpan));
                EFS.mUsedBytes = static_cast<uint8_t>(EFSSpan.size());
                ReturnErrorOnFailure(scene.mStorageData.mExtensionFieldSets.InsertFieldSet(EFS));
                break;
            }
        }

        return CHIP_NO_ERROR;
    }
};

class TestScenesManagementTableProvider : public ScenesManagementTableProvider
{
public:
    TestScenesManagementTableProvider()           = default;
    ~TestScenesManagementTableProvider() override = default;

    void Init(PersistentStorageDelegate * storage, Provider * provider)
    {
        mSceneTable = Platform::MakeUnique<TestSceneTable>();
        mSceneTable->SetEndpoint(kTestEndpointId);
        ASSERT_EQ(mSceneTable->Init(*storage, *provider), CHIP_NO_ERROR);
    }

    ScenesManagementSceneTable * Take() override { return mSceneTable.get(); }
    void Release(ScenesManagementSceneTable *) override {}

    Platform::UniquePtr<TestSceneTable> mSceneTable;
};

class CustomDataModel : public EmptyProvider
{
public:
    CHIP_ERROR Endpoints(ReadOnlyBufferBuilder<DataModel::EndpointEntry> & builder) override
    {
        static constexpr DataModel::EndpointEntry kEndpoints[] = { {
            .id                 = kTestEndpointId,
            .parentId           = kInvalidEndpointId,
            .compositionPattern = EndpointCompositionPattern::kTree,
        }

        };
        return builder.ReferenceExisting(Span(kEndpoints));
    }
};

CHIP_ERROR AddFakeFabric(FabricTable & fabricTable, FabricIndex expectedFabricIndex)
{
    Crypto::P256SerializedKeypair opKeysSerialized;
    static Crypto::P256Keypair opKey;

    struct NocData
    {
        const ByteSpan & publicKey;
        const ByteSpan & privateKey;
        const ByteSpan & rcac;
        const ByteSpan & icac;
        const ByteSpan & noc;
    };

    static const NocData kNocItems[] = {
        {
            .publicKey  = TestCerts::sTestCert_Node01_01_PublicKey,
            .privateKey = TestCerts::sTestCert_Node01_01_PrivateKey,
            .rcac       = TestCerts::sTestCert_Root01_Chip,
            .icac       = TestCerts::sTestCert_ICA01_Chip,
            .noc        = TestCerts::sTestCert_Node01_01_Chip,
        },
        {
            .publicKey  = TestCerts::sTestCert_Node02_02_PublicKey,
            .privateKey = TestCerts::sTestCert_Node02_02_PrivateKey,
            .rcac       = TestCerts::sTestCert_Root02_Chip,
            .icac       = TestCerts::sTestCert_ICA02_Chip,
            .noc        = TestCerts::sTestCert_Node02_02_Chip,
        },
    };

    ssize_t nocDataIndex = -1;

    switch (expectedFabricIndex)
    {
    case kFabricIndex:
        nocDataIndex = 0;
        break;
    case kFabricIndex2:
        nocDataIndex = 1;
        break;
    }

    VerifyOrReturnError(nocDataIndex >= 0 && static_cast<size_t>(nocDataIndex) < MATTER_ARRAY_SIZE(kNocItems), CHIP_ERROR_INTERNAL);
    const NocData & nocData = kNocItems[nocDataIndex];

    FabricIndex fabricIndex;
    memcpy(opKeysSerialized.Bytes(), nocData.publicKey.data(), nocData.publicKey.size());
    memcpy(opKeysSerialized.Bytes() + nocData.publicKey.size(), nocData.privateKey.data(), nocData.privateKey.size());

    ReturnErrorOnFailure(opKeysSerialized.SetLength(TestCerts::sTestCert_Node01_01_PublicKey.size() +
                                                    TestCerts::sTestCert_Node01_01_PrivateKey.size()));
    ReturnErrorOnFailure(opKey.Deserialize(opKeysSerialized));
    ReturnErrorOnFailure(fabricTable.AddNewPendingTrustedRootCert(nocData.rcac));
    ReturnErrorOnFailure(fabricTable.AddNewPendingFabricWithProvidedOpKey(nocData.noc, nocData.icac, VendorId::TestVendor1, &opKey,
                                                                          /*isExistingOpKeyExternallyOwned =*/true, &fabricIndex));

    VerifyOrReturnError(fabricIndex == expectedFabricIndex, CHIP_ERROR_INTERNAL);

    return fabricTable.CommitPendingFabricData();
}

struct TestScenesManagementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestScenesManagementCluster() :
        cluster(kTestEndpointId,
                ScenesManagementCluster::Context{ .groupDataProvider = &mockGroupDataProvider,
                                                  .fabricTable       = &fabricTable,
                                                  .features =
                                                      BitMask<ScenesManagement::Feature>(ScenesManagement::Feature::kSceneNames),
                                                  .sceneTableProvider = sceneTableProvider,
                                                  .supportsCopyScene  = true })
    {}

    void SetUp() override
    {
        testContext.StorageDelegate().ClearStorage(); // Clear storage before each test
        ASSERT_EQ(mOpCertStore.Init(&testContext.StorageDelegate()), CHIP_NO_ERROR);

        FabricTable::InitParams initParams;
        initParams.storage     = &testContext.StorageDelegate();
        initParams.opCertStore = &mOpCertStore;
        ASSERT_EQ(fabricTable.Init(initParams), CHIP_NO_ERROR);

        sceneTableProvider.Init(&testContext.StorageDelegate(), &testContext.Get().provider);
        sceneTableProvider.mSceneTable->RegisterHandler(&mMockSceneHandler);

        ServerClusterContext context = testContext.Get();
        clusterContext               = std::make_unique<ServerClusterContext>(ServerClusterContext{
                          .provider           = customDataModel,
                          .storage            = context.storage,
                          .attributeStorage   = context.attributeStorage,
                          .interactionContext = context.interactionContext,
        });

        ASSERT_EQ(cluster.Startup(*clusterContext), CHIP_NO_ERROR);
    }

    void TearDown() override
    {
        sceneTableProvider.mSceneTable->UnregisterHandler(&mMockSceneHandler);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
        clusterContext.reset();
        fabricTable.Shutdown();
    }

    void AddSceneToTable(ClusterTester & tester, GroupId groupID, SceneId sceneID, const char * name = "TestScene",
                         uint16_t transitionTime = 100, const DataModel::List<ExtensionFieldSetStruct::Type> * efs = nullptr)
    {
        AddScene::Type request_data;
        request_data.groupID        = groupID;
        request_data.sceneID        = sceneID;
        request_data.transitionTime = transitionTime;
        request_data.sceneName      = CharSpan::fromCharString(name);
        if (efs)
        {
            request_data.extensionFieldSetStructs = *efs;
        }
        else
        {
            request_data.extensionFieldSetStructs = List<ExtensionFieldSetStruct::Type>();
        }

        auto response = tester.Invoke<AddScene::Type, AddSceneResponse::DecodableType>(AddScene::Id, request_data);
        ASSERT_TRUE(response.IsSuccess());
        ASSERT_TRUE(response.response.has_value());

        // NOLINTBEGIN(bugprone-unchecked-optional-access)
        ASSERT_EQ(response.response->status, to_underlying(Status::Success));
        EXPECT_EQ(response.response->groupID, groupID);
        EXPECT_EQ(response.response->sceneID, sceneID);
        // NOLINTEND(bugprone-unchecked-optional-access)
    }

    void VerifySceneInfoCount(ClusterTester & tester, FabricIndex fabricIndex, uint16_t expectedCount)
    {
        Attributes::FabricSceneInfo::TypeInfo::DecodableType sceneInfoList;
        ASSERT_EQ(tester.ReadAttribute(Attributes::FabricSceneInfo::Id, sceneInfoList), CHIP_NO_ERROR);

        auto it    = sceneInfoList.begin();
        bool found = false;
        while (it.Next())
        {
            if (it.GetValue().fabricIndex == fabricIndex)
            {
                EXPECT_EQ(it.GetValue().sceneCount, expectedCount);
                found = true;
                break;
            }
        }
        EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
        EXPECT_TRUE(found);
    }

    template <typename DecodableType>
    void ExpectCommandStatus(const ClusterTester::InvokeResult<DecodableType> & response, Status expectedStatus)
    {
        ASSERT_TRUE(response.IsSuccess());
        ASSERT_TRUE(response.response.has_value());
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        EXPECT_EQ(response.response->status, to_underlying(expectedStatus));
    }

    CustomDataModel customDataModel;
    TestServerClusterContext testContext;
    MockGroupDataProvider mockGroupDataProvider;
    PersistentStorageOpCertStore mOpCertStore;
    FabricTable fabricTable;
    TestScenesManagementTableProvider sceneTableProvider;
    ScenesManagementCluster cluster;
    MockSceneHandler mMockSceneHandler;

    // Test context uses an empty data model provider, however we scenes relies on
    // endpoint iteration. Create a DMP that returns some endpoints.
    std::unique_ptr<ServerClusterContext> clusterContext;
};

TEST_F(TestScenesManagementCluster, AttributesTest)
{
    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            SceneTableSize::kMetadataEntry,
                                            FabricSceneInfo::kMetadataEntry,
                                        }));
}

TEST_F(TestScenesManagementCluster, AddSceneCommandSuccess)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    AddSceneToTable(tester, kTestGroupId, kTestSceneId, "TestScene", 100);

    SceneTable<ExtensionFieldSetsImpl>::SceneStorageId sceneStorageId(kTestSceneId, kTestGroupId);
    SceneTable<ExtensionFieldSetsImpl>::SceneTableEntry sceneEntry(sceneStorageId);
    ASSERT_EQ(sceneTableProvider.mSceneTable->GetSceneTableEntry(kFabricIndex, sceneStorageId, sceneEntry), CHIP_NO_ERROR);

    SceneTable<ExtensionFieldSetsImpl>::SceneData expectedSceneData("TestScene"_span, 100);
    EXPECT_EQ(sceneEntry.mStorageData, expectedSceneData);

    VerifySceneInfoCount(tester, kFabricIndex, 1);
}

TEST_F(TestScenesManagementCluster, AddSceneCommandModifyExistingScene)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    // 1. Add an initial scene
    AddSceneToTable(tester, kTestGroupId, kTestSceneId, "InitialSceneName", 50);

    // 2. Modify the scene using AddScene with the same ID but different data
    AddSceneToTable(tester, kTestGroupId, kTestSceneId, "ModifiedName", 150);

    // 3. Verify the modification using ViewScene
    ViewScene::Type view_request;
    view_request.groupID = kTestGroupId;
    view_request.sceneID = kTestSceneId;
    auto view_response   = tester.Invoke<ViewScene::Type, ViewSceneResponse::DecodableType>(ViewScene::Id, view_request);
    ASSERT_TRUE(view_response.IsSuccess());
    ASSERT_TRUE(view_response.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    auto & data = *view_response.response;
    EXPECT_EQ(data.groupID, kTestGroupId);
    EXPECT_EQ(data.sceneID, kTestSceneId);
    ASSERT_TRUE(data.transitionTime.HasValue());
    EXPECT_EQ(data.transitionTime.Value(), 150u);
    ASSERT_TRUE(data.sceneName.HasValue());
    EXPECT_TRUE(data.sceneName.Value().data_equal("ModifiedName"_span));

    VerifySceneInfoCount(tester, kFabricIndex, 1);
}

TEST_F(TestScenesManagementCluster, AddSceneCommandInvalidGroupID)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    // Configure the mock to return false for HasEndpoint to simulate the group not existing for this endpoint.
    mockGroupDataProvider.mHasEndpoint = false;

    AddScene::Type request_data;
    request_data.groupID                  = kTestOtherGroupId; // A group ID that doesn't "exist" for the endpoint.
    request_data.sceneID                  = kTestSceneId;
    request_data.transitionTime           = 100;
    request_data.sceneName                = "InvalidGScene"_span;
    request_data.extensionFieldSetStructs = List<ExtensionFieldSetStruct::Type>();

    ClusterTester::InvokeResult<AddSceneResponse::DecodableType> response =
        tester.Invoke<AddScene::Type, AddSceneResponse::DecodableType>(AddScene::Id, request_data);

    ExpectCommandStatus(response, Status::InvalidCommand);

    // Restore the mock's default behavior
    mockGroupDataProvider.mHasEndpoint = true;
}

TEST_F(TestScenesManagementCluster, ViewSceneCommandSuccess)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    // Prepare EFS data
    AttributeValuePairStruct::Type attributeValue;
    attributeValue.attributeID = 0; // Assuming Attribute ID 0 is the 'on' state
    attributeValue.valueUnsigned8.SetValue(true);

    AttributeValuePairStruct::Type attributeValueList[1];
    attributeValueList[0] = attributeValue;

    ExtensionFieldSetStruct::Type efsStruct;
    efsStruct.clusterID          = kMockClusterId;
    efsStruct.attributeValueList = DataModel::List<AttributeValuePairStruct::Type>(attributeValueList);

    ExtensionFieldSetStruct::Type efsList[1];
    efsList[0]                                         = efsStruct;
    DataModel::List<ExtensionFieldSetStruct::Type> efs = DataModel::List<ExtensionFieldSetStruct::Type>(efsList);

    AddSceneToTable(tester, kTestGroupId, kTestSceneId, "ViewSceneWithEFS", 100, &efs);

    ViewScene::Type view_request;
    view_request.groupID = kTestGroupId;
    view_request.sceneID = kTestSceneId;

    ClusterTester::InvokeResult<ViewSceneResponse::DecodableType> view_response =
        tester.Invoke<ViewScene::Type, ViewSceneResponse::DecodableType>(ViewScene::Id, view_request);

    ExpectCommandStatus(view_response, Status::Success);
    ASSERT_TRUE(view_response.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    auto & data = *view_response.response;

    EXPECT_EQ(data.groupID, kTestGroupId);
    EXPECT_EQ(data.sceneID, kTestSceneId);
    ASSERT_TRUE(data.transitionTime.HasValue());
    EXPECT_EQ(data.transitionTime.Value(), 100u);
    ASSERT_TRUE(data.sceneName.HasValue());
    EXPECT_TRUE(data.sceneName.Value().data_equal("ViewSceneWithEFS"_span));

    // Verify EFS content
    ASSERT_TRUE(data.extensionFieldSetStructs.HasValue());
    auto responseEfsList = data.extensionFieldSetStructs.Value();
    auto iterator        = responseEfsList.begin();
    size_t efs_count     = 0;
    bool found_mock_efs  = false;

    while (iterator.Next())
    {
        efs_count++;
        auto const & received_efs = iterator.GetValue();
        if (received_efs.clusterID == kMockClusterId)
        {
            found_mock_efs = true;
            auto attr_iter = received_efs.attributeValueList.begin();
            ASSERT_TRUE(attr_iter.Next());
            auto const & attr = attr_iter.GetValue();
            EXPECT_EQ(attr.attributeID, 0u);
            ASSERT_TRUE(attr.valueUnsigned8.HasValue());
            EXPECT_EQ(attr.valueUnsigned8.Value(), true);
            ASSERT_FALSE(attr_iter.Next()); // Should only be one attribute
            EXPECT_EQ(attr_iter.GetStatus(), CHIP_NO_ERROR);
        }
    }
    EXPECT_EQ(iterator.GetStatus(), CHIP_NO_ERROR);
    EXPECT_EQ(efs_count, 1u);
    EXPECT_TRUE(found_mock_efs);
}

TEST_F(TestScenesManagementCluster, RemoveSceneCommandSuccess)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    AddSceneToTable(tester, kTestGroupId, kTestSceneId);

    RemoveScene::Type remove_request;
    remove_request.groupID = kTestGroupId;
    remove_request.sceneID = kTestSceneId;

    ClusterTester::InvokeResult<RemoveSceneResponse::DecodableType> remove_response =
        tester.Invoke<RemoveScene::Type, RemoveSceneResponse::DecodableType>(RemoveScene::Id, remove_request);

    ExpectCommandStatus(remove_response, Status::Success);
    ASSERT_TRUE(remove_response.response.has_value());
    // NOLINTBEGIN(bugprone-unchecked-optional-access)
    EXPECT_EQ(remove_response.response->groupID, remove_request.groupID);
    EXPECT_EQ(remove_response.response->sceneID, remove_request.sceneID);
    // NOLINTEND(bugprone-unchecked-optional-access)

    SceneTable<ExtensionFieldSetsImpl>::SceneStorageId sceneStorageId(kTestSceneId, kTestGroupId);
    SceneTable<ExtensionFieldSetsImpl>::SceneTableEntry sceneEntry(sceneStorageId);
    EXPECT_EQ(sceneTableProvider.mSceneTable->GetSceneTableEntry(kFabricIndex, sceneStorageId, sceneEntry), CHIP_ERROR_NOT_FOUND);

    VerifySceneInfoCount(tester, kFabricIndex, 0);
}

TEST_F(TestScenesManagementCluster, RemoveAllScenesCommandSuccess)
{
    ClusterTester tester(cluster);

    tester.SetFabricIndex(kFabricIndex);

    struct SceneDataForTest
    {
        GroupId groupID;
        SceneId sceneID;
        bool toBeRemoved;
    };

    SceneDataForTest scenesToAdd[] = {
        { kTestGroupId, kTestSceneId, true },
        { kTestGroupId, kTestOtherSceneId, true },
        { kTestOtherGroupId, kTestSceneId, false },
    };

    for (const auto & scene : scenesToAdd)
    {
        AddSceneToTable(tester, scene.groupID, scene.sceneID);
    }

    // Invoke RemoveAllScenes for kTestGroupId
    RemoveAllScenes::Type remove_all_request;
    remove_all_request.groupID = kTestGroupId;

    ClusterTester::InvokeResult<RemoveAllScenesResponse::DecodableType> remove_all_response =
        tester.Invoke<RemoveAllScenes::Type, RemoveAllScenesResponse::DecodableType>(RemoveAllScenes::Id, remove_all_request);

    ExpectCommandStatus(remove_all_response, Status::Success);
    ASSERT_TRUE(remove_all_response.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(remove_all_response.response->groupID, kTestGroupId);

    // Verify scenes based on toBeRemoved flag
    for (const auto & scene : scenesToAdd)
    {
        SceneTable<ExtensionFieldSetsImpl>::SceneStorageId sceneStorageId(scene.sceneID, scene.groupID);
        SceneTable<ExtensionFieldSetsImpl>::SceneTableEntry sceneEntry(sceneStorageId);
        if (scene.toBeRemoved)
        {
            EXPECT_EQ(sceneTableProvider.mSceneTable->GetSceneTableEntry(kFabricIndex, sceneStorageId, sceneEntry),
                      CHIP_ERROR_NOT_FOUND);
        }
        else
        {
            EXPECT_EQ(sceneTableProvider.mSceneTable->GetSceneTableEntry(kFabricIndex, sceneStorageId, sceneEntry), CHIP_NO_ERROR);
        }
    }

    VerifySceneInfoCount(tester, kFabricIndex, 1);
}

TEST_F(TestScenesManagementCluster, RemoveAllScenesInvalidGroupID)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    // Configure the mock to return false for HasEndpoint to simulate the group not existing for this endpoint.
    mockGroupDataProvider.mHasEndpoint = false;

    RemoveAllScenes::Type remove_all_request;
    remove_all_request.groupID = kTestOtherGroupId; // A group ID that doesn't "exist" for the endpoint.

    ClusterTester::InvokeResult<RemoveAllScenesResponse::DecodableType> remove_all_response =
        tester.Invoke<RemoveAllScenes::Type, RemoveAllScenesResponse::DecodableType>(RemoveAllScenes::Id, remove_all_request);

    ExpectCommandStatus(remove_all_response, Status::InvalidCommand);

    // Restore the mock's default behavior
    mockGroupDataProvider.mHasEndpoint = true;
}

TEST_F(TestScenesManagementCluster, GetSceneMembershipCommandSuccess)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    struct SceneDataForTest
    {
        GroupId groupID;
        SceneId sceneID;
    };

    SceneDataForTest scenesToAdd[] = {
        { kTestGroupId, kTestSceneId },
        { kTestGroupId, kTestOtherSceneId },
        { kTestOtherGroupId, kTestSceneId },
    };

    for (const auto & scene : scenesToAdd)
    {
        AddSceneToTable(tester, scene.groupID, scene.sceneID);
    }

    GetSceneMembership::Type request;
    request.groupID = kTestGroupId;

    ClusterTester::InvokeResult<GetSceneMembershipResponse::DecodableType> response =
        tester.Invoke<GetSceneMembership::Type, GetSceneMembershipResponse::DecodableType>(GetSceneMembership::Id, request);

    ExpectCommandStatus(response, Status::Success);
    ASSERT_TRUE(response.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    auto & data = *response.response;
    EXPECT_EQ(data.groupID, kTestGroupId);
    ASSERT_FALSE(data.capacity.IsNull());
    // We added 3 scenes total to the fabric, so capacity should be kMaxScenesPerFabric - 3
    EXPECT_EQ(data.capacity.Value(), kMaxScenesPerFabric - (sizeof(scenesToAdd) / sizeof(scenesToAdd[0])));

    // Verify the scene list
    ASSERT_TRUE(data.sceneList.HasValue());
    auto sceneList   = data.sceneList.Value();
    auto iterator    = sceneList.begin();
    size_t list_size = 0;
    bool foundScene1 = false;
    bool foundScene2 = false;

    while (iterator.Next())
    {
        list_size++;
        auto sceneId = iterator.GetValue();
        if (sceneId == kTestSceneId)
        {
            foundScene1 = true;
        }
        else if (sceneId == kTestOtherSceneId)
        {
            foundScene2 = true;
        }
    }
    EXPECT_EQ(iterator.GetStatus(), CHIP_NO_ERROR);
    EXPECT_EQ(list_size, 2u);
    EXPECT_TRUE(foundScene1);
    EXPECT_TRUE(foundScene2);
}

TEST_F(TestScenesManagementCluster, GetSceneMembershipInvalidGroupID)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    // Configure the mock to return false for HasEndpoint to simulate the group not existing for this endpoint.
    mockGroupDataProvider.mHasEndpoint = false;

    GetSceneMembership::Type request;
    request.groupID = kTestOtherGroupId;

    ClusterTester::InvokeResult<GetSceneMembershipResponse::DecodableType> response =
        tester.Invoke<GetSceneMembership::Type, GetSceneMembershipResponse::DecodableType>(GetSceneMembership::Id, request);

    ExpectCommandStatus(response, Status::InvalidCommand);

    // Restore the mock's default behavior
    mockGroupDataProvider.mHasEndpoint = true;
}

TEST_F(TestScenesManagementCluster, StoreSceneCommandSuccess)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    // A scene must be added via AddScene before StoreScene can be used to modify it.
    AddSceneToTable(tester, kTestGroupId, kTestSceneId);

    mMockSceneHandler.mState.on = true;

    StoreScene::Type request;
    request.groupID = kTestGroupId;
    request.sceneID = kTestSceneId;

    ClusterTester::InvokeResult<StoreSceneResponse::DecodableType> response =
        tester.Invoke<StoreScene::Type, StoreSceneResponse::DecodableType>(StoreScene::Id, request);

    ExpectCommandStatus(response, Status::Success);
    ASSERT_TRUE(response.response.has_value());
    // NOLINTBEGIN(bugprone-unchecked-optional-access)
    EXPECT_EQ(response.response->groupID, kTestGroupId);
    EXPECT_EQ(response.response->sceneID, kTestSceneId);
    // NOLINTEND(bugprone-unchecked-optional-access)

    SceneTable<ExtensionFieldSetsImpl>::SceneStorageId sceneStorageId(kTestSceneId, kTestGroupId);
    SceneTable<ExtensionFieldSetsImpl>::SceneTableEntry sceneEntry(sceneStorageId);
    ASSERT_EQ(sceneTableProvider.mSceneTable->GetSceneTableEntry(kFabricIndex, sceneStorageId, sceneEntry), CHIP_NO_ERROR);

    EXPECT_EQ(sceneEntry.mStorageData.mExtensionFieldSets.GetFieldSetCount(), 1u);

    ExtensionFieldSet efs;
    ASSERT_EQ(sceneEntry.mStorageData.mExtensionFieldSets.GetFieldSetAtPosition(efs, 0), CHIP_NO_ERROR);
    EXPECT_EQ(efs.mID, kMockClusterId);

    MockSceneHandler::MockClusterEFS deserializedEFS;
    TLV::TLVReader reader;
    reader.Init(efs.mBytesBuffer, efs.mUsedBytes);
    CHIP_ERROR err = deserializedEFS.Decode(reader);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(deserializedEFS.on, mMockSceneHandler.mState.on);
}

TEST_F(TestScenesManagementCluster, StoreSceneCommandCreatesSceneIfNotFound)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    // Do not add the scene first. Per Matter Spec 1.4 - 1.10.6.9, StoreScene should create the scene if it does not exist.
    StoreScene::Type request;
    request.groupID = kTestGroupId;
    request.sceneID = kTestSceneId;

    ClusterTester::InvokeResult<StoreSceneResponse::DecodableType> response =
        tester.Invoke<StoreScene::Type, StoreSceneResponse::DecodableType>(StoreScene::Id, request);

    ASSERT_TRUE(response.IsSuccess());
    ASSERT_TRUE(response.response.has_value());
    // NOLINTBEGIN(bugprone-unchecked-optional-access)
    EXPECT_EQ(response.response->status, to_underlying(Status::Success));
    EXPECT_EQ(response.response->groupID, kTestGroupId);
    EXPECT_EQ(response.response->sceneID, kTestSceneId);
    // NOLINTEND(bugprone-unchecked-optional-access)

    ViewScene::Type view_request;
    view_request.groupID = kTestGroupId;
    view_request.sceneID = kTestSceneId;

    auto view_response = tester.Invoke<ViewScene::Type, ViewSceneResponse::DecodableType>(ViewScene::Id, view_request);
    ASSERT_TRUE(view_response.IsSuccess());
    ASSERT_TRUE(view_response.response.has_value());
    // NOLINTBEGIN(bugprone-unchecked-optional-access)
    EXPECT_EQ(view_response.response->status, to_underlying(Status::Success));
    auto & data = view_response.response.value();
    // NOLINTEND(bugprone-unchecked-optional-access)

    // Per Matter Spec 1.4 - 1.10.6.9, when creating a scene via StoreScene,
    // the transition time is 0 and the scene name is an empty string.
    ASSERT_TRUE(data.transitionTime.HasValue());
    EXPECT_EQ(data.transitionTime.Value(), 0u);
    ASSERT_TRUE(data.sceneName.HasValue());
    EXPECT_TRUE(data.sceneName.Value().empty());
}

TEST_F(TestScenesManagementCluster, StoreSceneResourceExhausted)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    // Fill the scene table to its maximum capacity.
    for (SceneId sceneId = 0; sceneId < kMaxScenesPerFabric; ++sceneId)
    {
        AddSceneToTable(tester, kTestGroupId, sceneId);
    }

    // Attempt to store a new scene, which should fail due to lack of space in the fabric scene table.
    StoreScene::Type store_request_fail;
    store_request_fail.groupID = kTestOtherGroupId;
    store_request_fail.sceneID = kTestOtherSceneId;

    ClusterTester::InvokeResult<StoreSceneResponse::DecodableType> store_response_fail =
        tester.Invoke<StoreScene::Type, StoreSceneResponse::DecodableType>(StoreScene::Id, store_request_fail);

    ExpectCommandStatus(store_response_fail, Status::ResourceExhausted);
}

TEST_F(TestScenesManagementCluster, RecallSceneCommandSuccess)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    // Add and store a scene with the mock handler's state as "on".
    AddSceneToTable(tester, kTestGroupId, kTestSceneId);

    mMockSceneHandler.mState.on = true;

    StoreScene::Type store_request;
    store_request.groupID = kTestGroupId;
    store_request.sceneID = kTestSceneId;

    auto store_response = tester.Invoke<StoreScene::Type, StoreSceneResponse::DecodableType>(StoreScene::Id, store_request);
    ExpectCommandStatus(store_response, Status::Success);

    // Change the mock handler's state to "off" to ensure RecallScene changes it.
    mMockSceneHandler.mState.on = false;
    EXPECT_FALSE(mMockSceneHandler.mState.on);

    RecallScene::Type recall_request;
    recall_request.groupID = kTestGroupId;
    recall_request.sceneID = kTestSceneId;

    auto recall_response = tester.Invoke<RecallScene::Type, NullObjectType>(RecallScene::Id, recall_request);
    ASSERT_TRUE(recall_response.IsSuccess());

    // Verify the mock handler's state was restored to "on" by ApplyScene.
    EXPECT_TRUE(mMockSceneHandler.mState.on);
}

TEST_F(TestScenesManagementCluster, RecallSceneNotFound)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    RecallScene::Type recall_request;
    recall_request.groupID = kTestGroupId;
    recall_request.sceneID = kTestSceneId;

    auto recall_response = tester.Invoke<RecallScene::Type, NullObjectType>(RecallScene::Id, recall_request);

    ASSERT_FALSE(recall_response.IsSuccess());
    ASSERT_TRUE(recall_response.status.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(recall_response.status.value(), Status::NotFound);
}

TEST_F(TestScenesManagementCluster, CopySceneCommandSuccess)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    // 1. Add and Store a scene in the source location.
    AddSceneToTable(tester, kTestGroupId, kTestSceneId, "SourceScene", 1234);

    mMockSceneHandler.mState.on = true;
    StoreScene::Type store_request;
    store_request.groupID = kTestGroupId;
    store_request.sceneID = kTestSceneId;
    auto store_response   = tester.Invoke<StoreScene::Type, StoreSceneResponse::DecodableType>(StoreScene::Id, store_request);
    ExpectCommandStatus(store_response, Status::Success);

    // 2. Invoke CopyScene to copy from {kTestGroupId, kTestSceneId} to {kTestOtherGroupId, kTestOtherSceneId}.
    CopyScene::Type copy_request;
    copy_request.mode.Set(CopyModeBitmap::kCopyAllScenes, false);
    copy_request.groupIdentifierFrom = kTestGroupId;
    copy_request.sceneIdentifierFrom = kTestSceneId;
    copy_request.groupIdentifierTo   = kTestOtherGroupId;
    copy_request.sceneIdentifierTo   = kTestOtherSceneId;

    auto copy_response = tester.Invoke<CopyScene::Type, CopySceneResponse::DecodableType>(CopyScene::Id, copy_request);
    ExpectCommandStatus(copy_response, Status::Success);
    ASSERT_TRUE(copy_response.response.has_value());
    // NOLINTBEGIN(bugprone-unchecked-optional-access)
    EXPECT_EQ(copy_response.response->groupIdentifierFrom, kTestGroupId);
    EXPECT_EQ(copy_response.response->sceneIdentifierFrom, kTestSceneId);
    // NOLINTEND(bugprone-unchecked-optional-access)

    // 3. Verify the copied scene exists and is identical to the source.
    SceneTable<ExtensionFieldSetsImpl>::SceneStorageId destSceneId(kTestOtherSceneId, kTestOtherGroupId);
    SceneTable<ExtensionFieldSetsImpl>::SceneTableEntry destSceneEntry(destSceneId);
    ASSERT_EQ(sceneTableProvider.mSceneTable->GetSceneTableEntry(kFabricIndex, destSceneId, destSceneEntry), CHIP_NO_ERROR);

    SceneTable<ExtensionFieldSetsImpl>::SceneStorageId srcSceneId(kTestSceneId, kTestGroupId);
    SceneTable<ExtensionFieldSetsImpl>::SceneTableEntry srcSceneEntry(srcSceneId);
    ASSERT_EQ(sceneTableProvider.mSceneTable->GetSceneTableEntry(kFabricIndex, srcSceneId, srcSceneEntry), CHIP_NO_ERROR);

    EXPECT_EQ(destSceneEntry.mStorageData, srcSceneEntry.mStorageData);
}

TEST_F(TestScenesManagementCluster, CopySceneInvalidGroupID)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    // Configure the mock to return false for HasEndpoint to simulate the group not existing for this endpoint.
    mockGroupDataProvider.mHasEndpoint = false;

    CopyScene::Type copy_request;
    copy_request.mode.Set(CopyModeBitmap::kCopyAllScenes, false);
    copy_request.groupIdentifierFrom = kTestOtherGroupId; // Non-existent group
    copy_request.sceneIdentifierFrom = kTestOtherSceneId;
    copy_request.groupIdentifierTo   = kTestGroupId;
    copy_request.sceneIdentifierTo   = kTestSceneId;

    auto copy_response = tester.Invoke<CopyScene::Type, CopySceneResponse::DecodableType>(CopyScene::Id, copy_request);

    ExpectCommandStatus(copy_response, Status::InvalidCommand);

    // Restore the mock's default behavior
    mockGroupDataProvider.mHasEndpoint = true;
}

TEST_F(TestScenesManagementCluster, AcceptedCommandsWithCopySceneTest)
{
    // Test with supportsCopyScene = true
    ScenesManagementCluster clusterWithCopy(
        kTestEndpointId,
        ScenesManagementCluster::Context{ .groupDataProvider = &mockGroupDataProvider,
                                          .fabricTable       = &fabricTable,
                                          .features = BitMask<ScenesManagement::Feature>(ScenesManagement::Feature::kSceneNames),
                                          .sceneTableProvider = sceneTableProvider,
                                          .supportsCopyScene  = true });

    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(clusterWithCopy,
                                              {
                                                  AddScene::kMetadataEntry,
                                                  ViewScene::kMetadataEntry,
                                                  RemoveScene::kMetadataEntry,
                                                  RemoveAllScenes::kMetadataEntry,
                                                  StoreScene::kMetadataEntry,
                                                  RecallScene::kMetadataEntry,
                                                  GetSceneMembership::kMetadataEntry,
                                                  CopyScene::kMetadataEntry,
                                              }));
}

TEST_F(TestScenesManagementCluster, AcceptedCommandsWithoutCopySceneTest)
{
    // Test with supportsCopyScene = false
    ScenesManagementCluster clusterWithoutCopy(
        kTestEndpointId,
        ScenesManagementCluster::Context{ .groupDataProvider = &mockGroupDataProvider,
                                          .fabricTable       = &fabricTable,
                                          .features = BitMask<ScenesManagement::Feature>(ScenesManagement::Feature::kSceneNames),
                                          .sceneTableProvider = sceneTableProvider,
                                          .supportsCopyScene  = false });

    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(clusterWithoutCopy,
                                              {
                                                  AddScene::kMetadataEntry,
                                                  ViewScene::kMetadataEntry,
                                                  RemoveScene::kMetadataEntry,
                                                  RemoveAllScenes::kMetadataEntry,
                                                  StoreScene::kMetadataEntry,
                                                  RecallScene::kMetadataEntry,
                                                  GetSceneMembership::kMetadataEntry,
                                              }));
}

TEST_F(TestScenesManagementCluster, ViewSceneCommandNotFound)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    ViewScene::Type view_request;
    view_request.groupID = kTestOtherGroupId; // Use a group that has no scenes
    view_request.sceneID = kTestOtherSceneId; // Use a scene that does not exist

    ClusterTester::InvokeResult<ViewSceneResponse::DecodableType> view_response =
        tester.Invoke<ViewScene::Type, ViewSceneResponse::DecodableType>(ViewScene::Id, view_request);

    ExpectCommandStatus(view_response, Status::NotFound);
}

TEST_F(TestScenesManagementCluster, RemoveSceneCommandNotFound)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    RemoveScene::Type remove_request;
    remove_request.groupID = kTestOtherGroupId; // Use a group that has no scenes
    remove_request.sceneID = kTestOtherSceneId; // Use a scene that does not exist

    ClusterTester::InvokeResult<RemoveSceneResponse::DecodableType> remove_response =
        tester.Invoke<RemoveScene::Type, RemoveSceneResponse::DecodableType>(RemoveScene::Id, remove_request);

    ExpectCommandStatus(remove_response, Status::NotFound);
}

TEST_F(TestScenesManagementCluster, AddSceneCommandResourceExhausted)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    // Fill the scene table to its maximum capacity.
    for (SceneId sceneId = 0; sceneId < kMaxScenesPerFabric; ++sceneId)
    {
        AddSceneToTable(tester, kTestGroupId, sceneId);
    }

    // Attempt to add one more scene, exceeding the fabric's scene capacity.
    AddScene::Type add_request_fail;
    add_request_fail.groupID                  = kTestGroupId;
    add_request_fail.sceneID                  = kMaxScenesPerFabric; // This sceneId will exceed the limit
    add_request_fail.transitionTime           = 100;
    add_request_fail.sceneName                = "OverflowScene"_span;
    add_request_fail.extensionFieldSetStructs = List<ExtensionFieldSetStruct::Type>();

    ClusterTester::InvokeResult<AddSceneResponse::DecodableType> add_response_fail =
        tester.Invoke<AddScene::Type, AddSceneResponse::DecodableType>(AddScene::Id, add_request_fail);

    ExpectCommandStatus(add_response_fail, Status::ResourceExhausted);
}

TEST_F(TestScenesManagementCluster, CopySceneCommandNotFound)
{
    ClusterTester tester(cluster);

    tester.SetFabricIndex(kFabricIndex);

    CopyScene::Type copy_request;
    copy_request.mode.Set(CopyModeBitmap::kCopyAllScenes, false);
    copy_request.groupIdentifierFrom = kTestOtherGroupId; // Non-existent group
    copy_request.sceneIdentifierFrom = kTestOtherSceneId; // Non-existent scene
    copy_request.groupIdentifierTo   = kTestGroupId;
    copy_request.sceneIdentifierTo   = kTestSceneId;

    auto copy_response = tester.Invoke<CopyScene::Type, CopySceneResponse::DecodableType>(CopyScene::Id, copy_request);

    ExpectCommandStatus(copy_response, Status::NotFound);
}

TEST_F(TestScenesManagementCluster, CopySceneCommandResourceExhausted)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    // 1. Fill the scene table to its maximum capacity for the fabric.
    for (SceneId sceneId = 0; sceneId < kMaxScenesPerFabric; ++sceneId)
    {
        AddSceneToTable(tester, kTestOtherGroupId, sceneId, "DestScene", 0);
    }

    // 2. Attempt to add a source scene to a different group. This will fail with RESOURCE_EXHAUSTED
    //    because the fabric's scene table is already full.
    AddScene::Type add_source_request;
    add_source_request.groupID                  = kTestGroupId; // Source group
    add_source_request.sceneID                  = kTestSceneId;
    add_source_request.transitionTime           = 0;
    add_source_request.sceneName                = "SourceScene"_span;
    add_source_request.extensionFieldSetStructs = List<ExtensionFieldSetStruct::Type>();
    auto add_source_response = tester.Invoke<AddScene::Type, AddSceneResponse::DecodableType>(AddScene::Id, add_source_request);
    ExpectCommandStatus(add_source_response, Status::ResourceExhausted);

    // 3. Attempt to copy the non-existent source scene to the destination group.
    //    This is expected to fail with NOT_FOUND first, but we are testing RESOURCE_EXHAUSTED scenarios.
    //    In a real scenario, this would return NOT_FOUND. However, since the table is full, if the scene DID exist,
    //    it would return RESOURCE_EXHAUSTED. To force this, we can try copying a scene that DOES exist.

    //    Let's use scene {kTestOtherGroupId, 0} as the source, to copy to {kTestGroupId, kTestSceneId}.
    CopyScene::Type copy_request;
    copy_request.mode.Set(CopyModeBitmap::kCopyAllScenes, false);
    copy_request.groupIdentifierFrom = kTestOtherGroupId;
    copy_request.sceneIdentifierFrom = 0;
    copy_request.groupIdentifierTo   = kTestGroupId; // Different group
    copy_request.sceneIdentifierTo   = kTestSceneId; // A new, unused scene ID

    auto copy_response = tester.Invoke<CopyScene::Type, CopySceneResponse::DecodableType>(CopyScene::Id, copy_request);

    // Even though the destination GroupID/SceneID doesn't exist, the table is full, so RESOURCE_EXHAUSTED is returned first.
    ExpectCommandStatus(copy_response, Status::ResourceExhausted);
}

TEST_F(TestScenesManagementCluster, SceneNamesFeatureEnabledTest)
{
    // Scenario: kSceneNames feature ENABLED (default for fixture).
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    AddSceneToTable(tester, kTestGroupId, kTestSceneId, "MySceneName");

    ViewScene::Type view_request;
    view_request.groupID = kTestGroupId;
    view_request.sceneID = kTestSceneId;
    auto view_response   = tester.Invoke<ViewScene::Type, ViewSceneResponse::DecodableType>(ViewScene::Id, view_request);

    ExpectCommandStatus(view_response, Status::Success);
    ASSERT_TRUE(view_response.response.has_value());
    // NOLINTBEGIN(bugprone-unchecked-optional-access)
    ASSERT_TRUE(view_response.response->sceneName.HasValue());
    EXPECT_TRUE(view_response.response->sceneName.Value().data_equal("MySceneName"_span));
    // NOLINTEND(bugprone-unchecked-optional-access)
}

TEST_F(TestScenesManagementCluster, SceneNamesFeatureDisabledTest)
{
    // Scenario: kSceneNames feature DISABLED.
    // Create a new cluster instance with kSceneNames feature disabled.
    TestScenesManagementTableProvider disabledSceneTableProvider;
    disabledSceneTableProvider.Init(&testContext.StorageDelegate(), &testContext.Get().provider);
    ScenesManagementCluster clusterWithoutSceneNames(
        kTestEndpointId,
        ScenesManagementCluster::Context{ .groupDataProvider  = &mockGroupDataProvider,
                                          .fabricTable        = &fabricTable,
                                          .features           = BitMask<ScenesManagement::Feature>(), // No features enabled
                                          .sceneTableProvider = disabledSceneTableProvider,
                                          .supportsCopyScene  = true }); // supportsCopyScene doesn't affect scene names

    ASSERT_EQ(clusterWithoutSceneNames.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(clusterWithoutSceneNames);
    tester.SetFabricIndex(kFabricIndex);

    AddSceneToTable(tester, kTestGroupId, kTestSceneId, "IgnoredSceneName");

    ViewScene::Type view_request;
    view_request.groupID = kTestGroupId;
    view_request.sceneID = kTestSceneId;
    auto view_response   = tester.Invoke<ViewScene::Type, ViewSceneResponse::DecodableType>(ViewScene::Id, view_request);

    ExpectCommandStatus(view_response, Status::Success);
    ASSERT_TRUE(view_response.response.has_value());
    // When kSceneNames is not supported, the Scene Name must be an empty string.
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_TRUE(!view_response.response.value().sceneName.HasValue() || view_response.response.value().sceneName.Value().empty());
    clusterWithoutSceneNames.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestScenesManagementCluster, CopyAllScenesCommandSuccess)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    // 1. Add multiple scenes to the source group kTestGroupId.
    const SceneId sourceSceneIds[] = { kTestSceneId, kTestOtherSceneId };
    for (const auto & sceneId : sourceSceneIds)
    {
        AddSceneToTable(tester, kTestGroupId, sceneId);
    }

    // 2. Invoke CopyScene with kCopyAllScenes mode to copy from kTestGroupId to kTestOtherGroupId.
    CopyScene::Type copy_request;
    copy_request.mode.Set(CopyModeBitmap::kCopyAllScenes, true);
    copy_request.groupIdentifierFrom = kTestGroupId;
    copy_request.sceneIdentifierFrom = 0; // Per Matter Spec 1.4 - 1.10.6.10, this field is ignored when copying all scenes.
    copy_request.groupIdentifierTo   = kTestOtherGroupId;
    copy_request.sceneIdentifierTo   = 0; // Per Matter Spec 1.4 - 1.10.6.10, this field is ignored when copying all scenes.

    auto copy_response = tester.Invoke<CopyScene::Type, CopySceneResponse::DecodableType>(CopyScene::Id, copy_request);
    ExpectCommandStatus(copy_response, Status::Success);
    ASSERT_TRUE(copy_response.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(copy_response.response->groupIdentifierFrom, kTestGroupId);

    // 3. Verify that all scenes were copied to the destination group.
    for (const auto & sceneId : sourceSceneIds)
    {
        SceneTable<ExtensionFieldSetsImpl>::SceneStorageId destSceneId(sceneId, kTestOtherGroupId);
        SceneTable<ExtensionFieldSetsImpl>::SceneTableEntry destSceneEntry(destSceneId);
        ASSERT_EQ(sceneTableProvider.mSceneTable->GetSceneTableEntry(kFabricIndex, destSceneId, destSceneEntry), CHIP_NO_ERROR);

        SceneTable<ExtensionFieldSetsImpl>::SceneStorageId srcSceneId(sceneId, kTestGroupId);
        SceneTable<ExtensionFieldSetsImpl>::SceneTableEntry srcSceneEntry(srcSceneId);
        ASSERT_EQ(sceneTableProvider.mSceneTable->GetSceneTableEntry(kFabricIndex, srcSceneId, srcSceneEntry), CHIP_NO_ERROR);

        EXPECT_EQ(destSceneEntry.mStorageData, srcSceneEntry.mStorageData);
    }
}

TEST_F(TestScenesManagementCluster, CopySceneOverwriteExisting)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    // 1. Add a source scene
    AddSceneToTable(tester, kTestGroupId, kTestSceneId, "SourceScene", 1234);

    // 2. Add a destination scene with the same ID, but different content
    AddSceneToTable(tester, kTestGroupId, kTestOtherSceneId, "OrigDestScene", 5678);

    // 3. Copy from {kTestGroupId, kTestSceneId} to {kTestGroupId, kTestOtherSceneId}, overwriting the existing scene.
    CopyScene::Type copy_request;
    copy_request.mode.Set(CopyModeBitmap::kCopyAllScenes, false);
    copy_request.groupIdentifierFrom = kTestGroupId;
    copy_request.sceneIdentifierFrom = kTestSceneId;
    copy_request.groupIdentifierTo   = kTestGroupId;
    copy_request.sceneIdentifierTo   = kTestOtherSceneId;

    auto copy_response = tester.Invoke<CopyScene::Type, CopySceneResponse::DecodableType>(CopyScene::Id, copy_request);
    ExpectCommandStatus(copy_response, Status::Success);

    // 4. Verify the destination scene is now a copy of the source scene.
    ViewScene::Type view_request;
    view_request.groupID = kTestGroupId;
    view_request.sceneID = kTestOtherSceneId;
    auto view_response   = tester.Invoke<ViewScene::Type, ViewSceneResponse::DecodableType>(ViewScene::Id, view_request);
    ExpectCommandStatus(view_response, Status::Success);
    ASSERT_TRUE(view_response.response.has_value());
    // NOLINTBEGIN(bugprone-unchecked-optional-access)
    EXPECT_TRUE(view_response.response->sceneName.Value().data_equal("SourceScene"_span));
    EXPECT_EQ(view_response.response->transitionTime.Value(), 1234u);
    // NOLINTEND(bugprone-unchecked-optional-access)
}

TEST_F(TestScenesManagementCluster, FabricScopingAddScene)
{
    ClusterTester tester(cluster);

    // Add scene to fabric 1
    tester.SetFabricIndex(kFabricIndex);
    AddSceneToTable(tester, kTestGroupId, kTestSceneId, "Fabric1Scene");

    // Add same scene ID to fabric 2 with a different name
    tester.SetFabricIndex(kFabricIndex2);
    AddSceneToTable(tester, kTestGroupId, kTestSceneId, "Fabric2Scene");

    // Verify fabric 1 scene
    tester.SetFabricIndex(kFabricIndex);
    ViewScene::Type view_request1;
    view_request1.groupID = kTestGroupId;
    view_request1.sceneID = kTestSceneId;
    auto view_response1   = tester.Invoke<ViewScene::Type, ViewSceneResponse::DecodableType>(ViewScene::Id, view_request1);
    ExpectCommandStatus(view_response1, Status::Success);
    ASSERT_TRUE(view_response1.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_TRUE(view_response1.response->sceneName.Value().data_equal("Fabric1Scene"_span));

    // Verify fabric 2 scene
    tester.SetFabricIndex(kFabricIndex2);
    ViewScene::Type view_request2;
    view_request2.groupID = kTestGroupId;
    view_request2.sceneID = kTestSceneId;
    auto view_response2   = tester.Invoke<ViewScene::Type, ViewSceneResponse::DecodableType>(ViewScene::Id, view_request2);
    ExpectCommandStatus(view_response2, Status::Success);
    ASSERT_TRUE(view_response2.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_TRUE(view_response2.response->sceneName.Value().data_equal("Fabric2Scene"_span));

    // Verify scene counts for both fabrics
    tester.SetFabricIndex(kFabricIndex); // Reading attributes can be done from any fabric context
    Attributes::FabricSceneInfo::TypeInfo::DecodableType sceneInfoList;
    ASSERT_EQ(tester.ReadAttribute(Attributes::FabricSceneInfo::Id, sceneInfoList), CHIP_NO_ERROR);

    auto it           = sceneInfoList.begin();
    int found_fabrics = 0;
    while (it.Next())
    {
        auto const & info = it.GetValue();
        if (info.fabricIndex == kFabricIndex)
        {
            EXPECT_EQ(info.sceneCount, 1u);
            found_fabrics++;
        }
        else if (info.fabricIndex == kFabricIndex2)
        {
            EXPECT_EQ(info.sceneCount, 1u);
            found_fabrics++;
        }
    }
    EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
    EXPECT_EQ(found_fabrics, 2);
}

TEST_F(TestScenesManagementCluster, FabricRemovalRemovesScenes)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    // 1. Add a scene for Fabric 1
    AddSceneToTable(tester, kTestGroupId, kTestSceneId, "Fabric1Scene");

    // 2. Add a scene for Fabric 2
    tester.SetFabricIndex(kFabricIndex2);
    AddSceneToTable(tester, kTestGroupId, kTestSceneId, "Fabric2Scene");

    // Verify scenes exist
    VerifySceneInfoCount(tester, kFabricIndex, 1);
    VerifySceneInfoCount(tester, kFabricIndex2, 1);

    // 3. Simulate Fabric 1 removal
    cluster.OnFabricRemoved(fabricTable, kFabricIndex);

    // 4. Verify Fabric 1 scenes are gone
    SceneTable<ExtensionFieldSetsImpl>::SceneStorageId sceneStorageId1(kTestSceneId, kTestGroupId);
    SceneTable<ExtensionFieldSetsImpl>::SceneTableEntry sceneEntry1(sceneStorageId1);
    EXPECT_EQ(sceneTableProvider.mSceneTable->GetSceneTableEntry(kFabricIndex, sceneStorageId1, sceneEntry1), CHIP_ERROR_NOT_FOUND);

    // 5. Verify Fabric 2 scenes are still there
    SceneTable<ExtensionFieldSetsImpl>::SceneStorageId sceneStorageId2(kTestSceneId, kTestGroupId);
    SceneTable<ExtensionFieldSetsImpl>::SceneTableEntry sceneEntry2(sceneStorageId2);
    ASSERT_EQ(sceneTableProvider.mSceneTable->GetSceneTableEntry(kFabricIndex2, sceneStorageId2, sceneEntry2), CHIP_NO_ERROR);

    // 6. Verify FabricSceneInfo list is updated
    Attributes::FabricSceneInfo::TypeInfo::DecodableType sceneInfoList;
    ASSERT_EQ(tester.ReadAttribute(Attributes::FabricSceneInfo::Id, sceneInfoList), CHIP_NO_ERROR);

    auto it           = sceneInfoList.begin();
    bool foundFabric1 = false;
    bool foundFabric2 = false;
    while (it.Next())
    {
        if (it.GetValue().fabricIndex == kFabricIndex)
        {
            foundFabric1 = true;
        }
        else if (it.GetValue().fabricIndex == kFabricIndex2)
        {
            foundFabric2 = true;
            EXPECT_EQ(it.GetValue().sceneCount, 1u);
        }
    }
    EXPECT_EQ(it.GetStatus(), CHIP_NO_ERROR);
    EXPECT_FALSE(foundFabric1); // Fabric 1 should not be in the list anymore
    EXPECT_TRUE(foundFabric2);
}

TEST_F(TestScenesManagementCluster, GroupRemovalRemovesScenes)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    // 1. Add scenes for kTestGroupId and kTestOtherGroupId
    AddSceneToTable(tester, kTestGroupId, kTestSceneId);
    AddSceneToTable(tester, kTestOtherGroupId, kTestSceneId);

    // Verify scenes exist
    SceneTable<ExtensionFieldSetsImpl>::SceneStorageId sceneId1(kTestSceneId, kTestGroupId);
    SceneTable<ExtensionFieldSetsImpl>::SceneTableEntry entry1(sceneId1);
    ASSERT_EQ(sceneTableProvider.mSceneTable->GetSceneTableEntry(kFabricIndex, sceneId1, entry1), CHIP_NO_ERROR);

    SceneTable<ExtensionFieldSetsImpl>::SceneStorageId sceneId2(kTestSceneId, kTestOtherGroupId);
    SceneTable<ExtensionFieldSetsImpl>::SceneTableEntry entry2(sceneId2);
    ASSERT_EQ(sceneTableProvider.mSceneTable->GetSceneTableEntry(kFabricIndex, sceneId2, entry2), CHIP_NO_ERROR);

    // 2. Call GroupWillBeRemoved for kTestGroupId
    ASSERT_EQ(cluster.GroupWillBeRemoved(kFabricIndex, kTestGroupId), CHIP_NO_ERROR);

    // 3. Verify scenes for kTestGroupId are gone
    EXPECT_EQ(sceneTableProvider.mSceneTable->GetSceneTableEntry(kFabricIndex, sceneId1, entry1), CHIP_ERROR_NOT_FOUND);

    // 4. Verify scenes for kTestOtherGroupId are still there
    ASSERT_EQ(sceneTableProvider.mSceneTable->GetSceneTableEntry(kFabricIndex, sceneId2, entry2), CHIP_NO_ERROR);
}

TEST_F(TestScenesManagementCluster, ShutdownPermanentRemoveWipesData)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    // 1. Add a scene
    AddSceneToTable(tester, kTestGroupId, kTestSceneId);

    // Verify it exists in storage
    SceneTable<ExtensionFieldSetsImpl>::SceneStorageId sceneStorageId(kTestSceneId, kTestGroupId);
    SceneTable<ExtensionFieldSetsImpl>::SceneTableEntry sceneEntry(sceneStorageId);
    ASSERT_EQ(sceneTableProvider.mSceneTable->GetSceneTableEntry(kFabricIndex, sceneStorageId, sceneEntry), CHIP_NO_ERROR);

    // 2. Shutdown with kPermanentRemove
    cluster.Shutdown(ClusterShutdownType::kPermanentRemove);

    // 3. Verify data is gone from storage
    // Re-init provider and table
    sceneTableProvider.Init(&testContext.StorageDelegate(), &testContext.Get().provider);

    // Check entry
    EXPECT_EQ(sceneTableProvider.mSceneTable->GetSceneTableEntry(kFabricIndex, sceneStorageId, sceneEntry), CHIP_ERROR_NOT_FOUND);

    // Restore cluster for TearDown to be happy
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
}

TEST_F(TestScenesManagementCluster, PersistenceAfterPowerCycle)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    // 1. Add a scene
    AddSceneToTable(tester, kTestGroupId, kTestSceneId, "PersistentScene");

    // 2. Simulate Power Cycle: Shutdown cluster and provider, but KEEP storage.
    sceneTableProvider.mSceneTable->UnregisterHandler(&mMockSceneHandler);
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);

    // 3. Re-initialize
    sceneTableProvider.Init(&testContext.StorageDelegate(), &testContext.Get().provider);
    sceneTableProvider.mSceneTable->RegisterHandler(&mMockSceneHandler);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    // 4. Verify scene still exists
    SceneTable<ExtensionFieldSetsImpl>::SceneStorageId sceneStorageId(kTestSceneId, kTestGroupId);
    SceneTable<ExtensionFieldSetsImpl>::SceneTableEntry sceneEntry(sceneStorageId);
    ASSERT_EQ(sceneTableProvider.mSceneTable->GetSceneTableEntry(kFabricIndex, sceneStorageId, sceneEntry), CHIP_NO_ERROR);
    EXPECT_EQ(sceneEntry.mStorageData.mNameLength, 15u); // "PersistentScene" length

    CharSpan nameSpan(sceneEntry.mStorageData.mName, sceneEntry.mStorageData.mNameLength);
    EXPECT_TRUE(nameSpan.data_equal("PersistentScene"_span));
}

TEST_F(TestScenesManagementCluster, RecallSceneInvalidatesOtherFabrics)
{
    // Pretend that we have some fabrics since this is what our tests expect
    // Note I could only find mock data for 2 fabrics even though fabric index 3 is defined.
    // This is just sufficient here for our own tests (adding fabric entries is rough!)
    ASSERT_EQ(AddFakeFabric(fabricTable, kFabricIndex), CHIP_NO_ERROR);
    ASSERT_EQ(AddFakeFabric(fabricTable, kFabricIndex2), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    // 1. Add scenes for Fabric 1 and Fabric 2
    AddSceneToTable(tester, kTestGroupId, kTestSceneId, "Fabric1Scene");

    tester.SetFabricIndex(kFabricIndex2);
    AddSceneToTable(tester, kTestGroupId, kTestSceneId, "Fabric2Scene");

    // 2. Recall Scene on Fabric 1
    tester.SetFabricIndex(kFabricIndex);
    RecallScene::Type recall_request;
    recall_request.groupID = kTestGroupId;
    recall_request.sceneID = kTestSceneId;
    auto recall_response   = tester.Invoke<RecallScene::Type, NullObjectType>(RecallScene::Id, recall_request);
    ASSERT_TRUE(recall_response.IsSuccess());

    // 3. Verify SceneValid is TRUE for Fabric 1 and FALSE for Fabric 2
    //    Make sure read is using fabric 1 (so that we can read fabric 1 data) - that allows us to read fabric sensitive fields
    //    like currentScene and currentGroup
    tester.SetFabricIndex(kFabricIndex);
    Attributes::FabricSceneInfo::TypeInfo::DecodableType sceneInfoList;
    ASSERT_EQ(tester.ReadAttribute(Attributes::FabricSceneInfo::Id, sceneInfoList), CHIP_NO_ERROR);

    auto it = sceneInfoList.begin();
    while (it.Next())
    {
        auto val = it.GetValue();
        if (val.fabricIndex == kFabricIndex)
        {
            EXPECT_EQ(val.currentScene, kTestSceneId);
            EXPECT_EQ(val.currentGroup, kTestGroupId);
            EXPECT_TRUE(val.sceneValid);
        }
        else if (val.fabricIndex == kFabricIndex2)
        {
            // For Fabric 2, if sceneValid is initialized, it should be false (or effectively treated as invalid)
            EXPECT_FALSE(val.sceneValid);
        }
    }

    // 4. Recall Scene on Fabric 2
    tester.SetFabricIndex(kFabricIndex2);
    auto recall_response2 = tester.Invoke<RecallScene::Type, NullObjectType>(RecallScene::Id, recall_request);
    ASSERT_TRUE(recall_response2.IsSuccess());

    // 5. Verify SceneValid is TRUE for Fabric 2 and FALSE for Fabric 1
    ASSERT_EQ(tester.ReadAttribute(Attributes::FabricSceneInfo::Id, sceneInfoList), CHIP_NO_ERROR);
    auto it2 = sceneInfoList.begin();
    while (it2.Next())
    {
        auto val = it2.GetValue();
        if (val.fabricIndex == kFabricIndex)
        {
            // Fabric 1 should now be invalid
            EXPECT_FALSE(val.sceneValid);
        }
        else if (val.fabricIndex == kFabricIndex2)
        {
            EXPECT_TRUE(val.sceneValid);
        }
    }
}

TEST_F(TestScenesManagementCluster, TransitionTimeLimit)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    // Try adding a scene with TransitionTime > 60,000,000 ms
    // 60,000,001
    AddScene::Type request_data;
    request_data.groupID                  = kTestGroupId;
    request_data.sceneID                  = kTestSceneId;
    request_data.transitionTime           = 60000001;
    request_data.sceneName                = "TooLong"_span;
    request_data.extensionFieldSetStructs = List<ExtensionFieldSetStruct::Type>();

    auto response = tester.Invoke<AddScene::Type, AddSceneResponse::DecodableType>(AddScene::Id, request_data);

    // Expect ConstraintError
    ExpectCommandStatus(response, Status::ConstraintError);
}

TEST_F(TestScenesManagementCluster, SceneNameLength)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    // Try adding a scene with Name length > 16
    // "12345678901234567" (17 chars)
    AddScene::Type request_data;
    request_data.groupID                  = kTestGroupId;
    request_data.sceneID                  = kTestSceneId;
    request_data.transitionTime           = 100;
    request_data.sceneName                = "12345678901234567"_span;
    request_data.extensionFieldSetStructs = List<ExtensionFieldSetStruct::Type>();

    auto response = tester.Invoke<AddScene::Type, AddSceneResponse::DecodableType>(AddScene::Id, request_data);

    // Expect ConstraintError
    ExpectCommandStatus(response, Status::ConstraintError);
}

TEST_F(TestScenesManagementCluster, DuplicateFieldSets)
{
    ClusterTester tester(cluster);
    tester.SetFabricIndex(kFabricIndex);

    // Create EFS with duplicate cluster IDs
    AttributeValuePairStruct::Type attributeValue1;
    attributeValue1.attributeID = 0;
    attributeValue1.valueUnsigned8.SetValue(true); // First value

    AttributeValuePairStruct::Type attributeValue2;
    attributeValue2.attributeID = 0;
    attributeValue2.valueUnsigned8.SetValue(false); // Second value (should overwrite)

    AttributeValuePairStruct::Type avList1[] = { attributeValue1 };
    AttributeValuePairStruct::Type avList2[] = { attributeValue2 };

    ExtensionFieldSetStruct::Type efs1;
    efs1.clusterID          = kMockClusterId;
    efs1.attributeValueList = DataModel::List<AttributeValuePairStruct::Type>(avList1);

    ExtensionFieldSetStruct::Type efs2;
    efs2.clusterID          = kMockClusterId;
    efs2.attributeValueList = DataModel::List<AttributeValuePairStruct::Type>(avList2);

    ExtensionFieldSetStruct::Type efsList[] = { efs1, efs2 };
    DataModel::List<ExtensionFieldSetStruct::Type> efs(efsList);

    AddSceneToTable(tester, kTestGroupId, kTestSceneId, "DuplicateEFS", 100, &efs);

    // Verify only the last one is recorded (value should be false)
    ViewScene::Type view_request;
    view_request.groupID = kTestGroupId;
    view_request.sceneID = kTestSceneId;

    auto view_response = tester.Invoke<ViewScene::Type, ViewSceneResponse::DecodableType>(ViewScene::Id, view_request);
    ExpectCommandStatus(view_response, Status::Success);
    ASSERT_TRUE(view_response.response.has_value());

    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    auto & data = *view_response.response;
    ASSERT_TRUE(data.extensionFieldSetStructs.HasValue());
    auto responseEfsList = data.extensionFieldSetStructs.Value();

    int count = 0;
    auto iter = responseEfsList.begin();
    while (iter.Next())
    {
        count++;
        auto const & received_efs = iter.GetValue();
        if (received_efs.clusterID == kMockClusterId)
        {
            auto attr_iter = received_efs.attributeValueList.begin();
            ASSERT_TRUE(attr_iter.Next());
            auto const & attr = attr_iter.GetValue();
            EXPECT_EQ(attr.valueUnsigned8.Value(), false); // Should be the second value
        }
    }
    EXPECT_EQ(count, 1); // Should only have 1 entry for kMockClusterId
}

TEST_F(TestScenesManagementCluster, RemainingCapacityUpdatesAcrossFabrics)
{
    ClusterTester tester(cluster);

    // 1. Add a scene on Fabric 1 to populate FabricSceneInfo and consume some capacity
    tester.SetFabricIndex(kFabricIndex);
    AddSceneToTable(tester, kTestGroupId, kTestSceneId, "Fabric1Scene");

    // 2. Get initial capacity on Fabric 1
    Attributes::FabricSceneInfo::TypeInfo::DecodableType sceneInfoList;
    ASSERT_EQ(tester.ReadAttribute(Attributes::FabricSceneInfo::Id, sceneInfoList), CHIP_NO_ERROR);

    uint8_t capacity1_initial = 0;
    bool found1               = false;
    auto it                   = sceneInfoList.begin();
    while (it.Next())
    {
        if (it.GetValue().fabricIndex == kFabricIndex)
        {
            capacity1_initial = it.GetValue().remainingCapacity;
            found1            = true;
        }
    }
    ASSERT_TRUE(found1);

    // 3. Add scenes on Fabric 2 and 3 until we hit global limit
    // We simply try to add more scenes than what a single fabric can hold,
    // and do this for multiple fabrics to exhaust the endpoint limit.

    // Fill Fabric 2
    tester.SetFabricIndex(kFabricIndex2);
    for (int i = 0; i < kMaxScenesPerFabric + 5; i++)
    {
        AddScene::Type request_data;
        request_data.groupID                  = kTestGroupId;
        request_data.sceneID                  = static_cast<SceneId>(kTestSceneId + i + 1);
        request_data.transitionTime           = 100;
        request_data.sceneName                = "Fabric2Scene"_span;
        request_data.extensionFieldSetStructs = List<ExtensionFieldSetStruct::Type>();

        // We ignore the result here because we expect failure eventually
        auto response = tester.Invoke<AddScene::Type, AddSceneResponse::DecodableType>(AddScene::Id, request_data);
        (void) response;
    }

    // Fill Fabric 3
    tester.SetFabricIndex(kFabricIndex3);
    for (int i = 0; i < kMaxScenesPerFabric + 5; i++)
    {
        AddScene::Type request_data;
        request_data.groupID                  = kTestGroupId;
        request_data.sceneID                  = static_cast<SceneId>(kTestSceneId + i + 1);
        request_data.transitionTime           = 100;
        request_data.sceneName                = "Fabric3Scene"_span;
        request_data.extensionFieldSetStructs = List<ExtensionFieldSetStruct::Type>();

        // We ignore the result here because we expect failure eventually
        auto response = tester.Invoke<AddScene::Type, AddSceneResponse::DecodableType>(AddScene::Id, request_data);
        (void) response;
    }

    // 4. Get capacity on Fabric 1 again
    tester.SetFabricIndex(kFabricIndex);
    ASSERT_EQ(tester.ReadAttribute(Attributes::FabricSceneInfo::Id, sceneInfoList), CHIP_NO_ERROR);

    uint8_t capacity1_new = 0;
    found1                = false;
    auto it2              = sceneInfoList.begin();
    while (it2.Next())
    {
        if (it2.GetValue().fabricIndex == kFabricIndex)
        {
            capacity1_new = it2.GetValue().remainingCapacity;
            found1        = true;
        }
    }
    ASSERT_TRUE(found1);

    // 5. Verify capacity decreased
    EXPECT_LT(capacity1_new, capacity1_initial);
}

} // namespace
