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
#include <pw_unit_test/framework.h>

#include <data-model-providers/codegen/tests/EmberInvokeOverride.h>
#include <data-model-providers/codegen/tests/EmberReadWriteOverride.h>

#include <access/AccessControl.h>
#include <access/SubjectDescriptor.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/AttributeEncodeState.h>
#include <app/AttributeValueDecoder.h>
#include <app/CommandHandlerInterface.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/GlobalAttributes.h>
#include <app/MessageDef/ReportDataMessage.h>
#include <app/data-model-provider/MetadataList.h>
#include <app/data-model-provider/MetadataLookup.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/OperationTypes.h>
#include <app/data-model-provider/StringBuilderAdapters.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/TestConstants.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>
#include <app/data-model/List.h>
#include <app/data-model/Nullable.h>
#include <app/util/attribute-metadata.h>
#include <app/util/attribute-storage-null-handling.h>
#include <app/util/ember-io-storage.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <app/util/mock/MockNodeConfig.h>
#include <app/util/odd-sized-integers.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVReader.h>
#include <lib/core/TLVTags.h>
#include <lib/core/TLVTypes.h>
#include <lib/core/TLVWriter.h>
#include <lib/support/Span.h>
#include <protocols/interaction_model/StatusCode.h>

#include <optional>
#include <vector>

using namespace chip;
using namespace chip::Test;
using namespace chip::app;
using namespace chip::app::Testing;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters::Globals::Attributes;

using chip::Protocols::InteractionModel::Status;

// Mock function for linking
void InitDataModelHandler() {}

namespace {

constexpr AttributeId kAttributeIdReadOnly   = 0x3001;
constexpr AttributeId kAttributeIdTimedWrite = 0x3002;

constexpr CommandId kMockCommandId1 = 0x1234;
constexpr CommandId kMockCommandId2 = 0x1122;

constexpr EndpointId kEndpointIdThatIsMissing = kMockEndpointMin - 1;

constexpr AttributeId kReadOnlyAttributeId = 0x5001;

constexpr DeviceTypeId kDeviceTypeId1   = 123;
constexpr uint8_t kDeviceTypeId1Version = 10;

constexpr DeviceTypeId kDeviceTypeId2   = 1122;
constexpr uint8_t kDeviceTypeId2Version = 11;

constexpr DeviceTypeId kDeviceTypeId3   = 3;
constexpr uint8_t kDeviceTypeId3Version = 33;

constexpr uint8_t kNamespaceID1 = 123;
constexpr uint8_t kTag1         = 10;
constexpr char kLabel1[]        = "Label1";

constexpr uint8_t kNamespaceID2 = 254;
constexpr uint8_t kTag2         = 22;
constexpr char kLabel2[]        = "Label2";

constexpr uint8_t kNamespaceID3 = 3;
constexpr uint8_t kTag3         = 32;

static_assert(kEndpointIdThatIsMissing != kInvalidEndpointId);
static_assert(kEndpointIdThatIsMissing != kMockEndpoint1);
static_assert(kEndpointIdThatIsMissing != kMockEndpoint2);
static_assert(kEndpointIdThatIsMissing != kMockEndpoint3);

bool operator==(const Access::SubjectDescriptor & a, const Access::SubjectDescriptor & b)
{
    if (a.fabricIndex != b.fabricIndex)
    {
        return false;
    }
    if (a.authMode != b.authMode)
    {
        return false;
    }
    if (a.subject != b.subject)
    {
        return false;
    }
    for (unsigned i = 0; i < a.cats.values.size(); i++)
    {
        if (a.cats.values[i] != b.cats.values[i])
        {
            return false;
        }
    }
    return true;
}

struct TestCodegenModelViaMocks : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

class TestProviderChangeListener : public ProviderChangeListener
{
public:
    void MarkDirty(const AttributePathParams & path) override { mDirtyList.push_back(path); }

    std::vector<AttributePathParams> & DirtyList() { return mDirtyList; }
    const std::vector<AttributePathParams> & DirtyList() const { return mDirtyList; }

private:
    std::vector<AttributePathParams> mDirtyList;
};

class TestEventGenerator : public EventsGenerator
{
    CHIP_ERROR GenerateEvent(EventLoggingDelegate * eventPayloadWriter, const EventOptions & options,
                             EventNumber & generatedEventNumber) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
};

class TestActionContext : public ActionContext
{
public:
    Messaging::ExchangeContext * CurrentExchange() override { return nullptr; }
};

class CodegenDataModelProviderWithContext : public CodegenDataModelProvider
{
public:
    CodegenDataModelProviderWithContext()
    {
        InteractionModelContext context{
            .eventsGenerator         = &mEventGenerator,
            .dataModelChangeListener = &mChangeListener,
            .actionContext           = &mActionContext,
        };

        Startup(context);
    }
    ~CodegenDataModelProviderWithContext() { Shutdown(); }

    TestProviderChangeListener & ChangeListener() { return mChangeListener; }
    const TestProviderChangeListener & ChangeListener() const { return mChangeListener; }

private:
    TestEventGenerator mEventGenerator;
    TestProviderChangeListener mChangeListener;
    TestActionContext mActionContext;
};

class MockAccessControl : public Access::AccessControl::Delegate, public Access::AccessControl::DeviceTypeResolver
{
public:
    CHIP_ERROR Check(const Access::SubjectDescriptor & subjectDescriptor, const Access::RequestPath & requestPath,
                     Access::Privilege requestPrivilege) override
    {
        if (subjectDescriptor == kAdminSubjectDescriptor)
        {
            return CHIP_NO_ERROR;
        }
        if ((subjectDescriptor == kViewSubjectDescriptor) && (requestPrivilege == Access::Privilege::kView))
        {
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_ACCESS_DENIED;
    }

    bool IsDeviceTypeOnEndpoint(DeviceTypeId deviceType, EndpointId endpoint) override { return true; }
};

class MockCommandHandler : public CommandHandler
{
public:
    CHIP_ERROR FallibleAddStatus(const ConcreteCommandPath & aRequestCommandPath,
                                 const Protocols::InteractionModel::ClusterStatusCode & aStatus,
                                 const char * context = nullptr) override
    {
        // MOCK: do not do anything here
        return CHIP_NO_ERROR;
    }

    void AddStatus(const ConcreteCommandPath & aRequestCommandPath, const Protocols::InteractionModel::ClusterStatusCode & aStatus,
                   const char * context = nullptr) override
    {
        // MOCK: do not do anything here
    }

    FabricIndex GetAccessingFabricIndex() const override { return 1; }

    CHIP_ERROR AddResponseData(const ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                               const DataModel::EncodableToTLV & aEncodable) override
    {
        return CHIP_NO_ERROR;
    }

    void AddResponse(const ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                     const DataModel::EncodableToTLV & aEncodable) override
    {}

    bool IsTimedInvoke() const override { return false; }

    void FlushAcksRightAwayOnSlowCommand() override {}

    Access::SubjectDescriptor GetSubjectDescriptor() const override { return kAdminSubjectDescriptor; }

    Messaging::ExchangeContext * GetExchangeContext() const override { return nullptr; }
};

/// Overrides Enumerate*Commands in the CommandHandlerInterface to allow
/// testing of behaviors when command enumeration is done in the interace.
class CustomListCommandHandler : public CommandHandlerInterface
{
public:
    CustomListCommandHandler(Optional<EndpointId> endpointId, ClusterId clusterId) : CommandHandlerInterface(endpointId, clusterId)
    {
        CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this);
    }
    ~CustomListCommandHandler() { CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this); }

    void InvokeCommand(HandlerContext & handlerContext) override
    {
        if (mHandleCommand)
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Protocols::InteractionModel::Status::Success);
            handlerContext.SetCommandHandled();
        }
        else
        {
            handlerContext.SetCommandNotHandled();
        }
    }

    void SetHandleCommands(bool handle) { mHandleCommand = handle; }

    CHIP_ERROR EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context) override
    {
        VerifyOrReturnError(mOverrideAccepted, CHIP_ERROR_NOT_IMPLEMENTED);

        for (auto id : mAccepted)
        {
            if (callback(id, context) != Loop::Continue)
            {
                break;
            }
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR EnumerateGeneratedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context) override
    {
        VerifyOrReturnError(mOverrideGenerated, CHIP_ERROR_NOT_IMPLEMENTED);

        for (auto id : mGenerated)
        {
            if (callback(id, context) != Loop::Continue)
            {
                break;
            }
        }
        return CHIP_NO_ERROR;
    }

    void SetOverrideAccepted(bool o) { mOverrideAccepted = o; }
    void SetOverrideGenerated(bool o) { mOverrideGenerated = o; }

    std::vector<CommandId> & AcceptedVec() { return mAccepted; }
    std::vector<CommandId> & GeneratedVec() { return mGenerated; }

private:
    bool mOverrideAccepted  = false;
    bool mOverrideGenerated = false;
    bool mHandleCommand     = false;

    std::vector<CommandId> mAccepted;
    std::vector<CommandId> mGenerated;
};

class ScopedMockAccessControl
{
public:
    ScopedMockAccessControl() { Access::GetAccessControl().Init(&mMock, mMock); }
    ~ScopedMockAccessControl() { Access::GetAccessControl().Finish(); }

private:
    MockAccessControl mMock;
};

#define MOCK_ATTRIBUTE_ID_FOR_NULLABLE_TYPE(zcl_type) MockAttributeId(zcl_type + 0x1000)
#define MOCK_ATTRIBUTE_CONFIG_NULLABLE(zcl_type)                                                                                   \
    MockAttributeConfig(MOCK_ATTRIBUTE_ID_FOR_NULLABLE_TYPE(zcl_type), zcl_type, ATTRIBUTE_MASK_WRITABLE | ATTRIBUTE_MASK_NULLABLE)

#define MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(zcl_type) MockAttributeId(zcl_type + 0x2000)
#define MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(zcl_type)                                                                               \
    MockAttributeConfig(MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(zcl_type), zcl_type, ATTRIBUTE_MASK_WRITABLE)

// clang-format off
const MockNodeConfig gTestNodeConfig({
    MockEndpointConfig(kMockEndpoint1, {
        MockClusterConfig(MockClusterId(1), {
            ClusterRevision::Id, FeatureMap::Id,
        }, {
            MockEventId(1), MockEventId(2),
        }),
        MockClusterConfig(MockClusterId(2), {
            ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1),
        }),
        MockClusterConfig(MockClusterId(3), {}, {}, {}, {}, BitMask<MockClusterSide>().Set(MockClusterSide::kClient)),
        MockClusterConfig(MockClusterId(4), {}, {}, {}, {}, BitMask<MockClusterSide>().Set(MockClusterSide::kClient)),
    }, {
        { kDeviceTypeId1, kDeviceTypeId1Version},
        { kDeviceTypeId2, kDeviceTypeId2Version},
        { kDeviceTypeId3, kDeviceTypeId3Version},
    },{
        { MakeNullable(VendorId::TestVendor1), kNamespaceID1, kTag1, MakeOptional(MakeNullable(CharSpan::fromCharString(kLabel1)))},
        { Nullable<VendorId>(), kNamespaceID2, kTag2, MakeOptional(MakeNullable(CharSpan::fromCharString(kLabel2)))},
        { MakeNullable(VendorId::TestVendor3), kNamespaceID3, kTag3, NullOptional},
    }),
    MockEndpointConfig(kMockEndpoint2, {
        MockClusterConfig(MockClusterId(1), {
            ClusterRevision::Id, FeatureMap::Id,
        }),
        MockClusterConfig(
            MockClusterId(2),
            {
               ClusterRevision::Id,
               FeatureMap::Id,
               MockAttributeId(1),
               MockAttributeConfig(MockAttributeId(2), ZCL_ARRAY_ATTRIBUTE_TYPE),
            },          /* attributes */
            {},         /* events */
            {1, 2, 23}, /* acceptedCommands */
            {2, 10}     /* generatedCommands */
        ),
        MockClusterConfig(
            MockClusterId(3),
            {
                ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1), MockAttributeId(2), MockAttributeId(3),
            },    /* attributes */
            {},   /* events */
            {11}, /* acceptedCommands */
            {4, 6},   /* generatedCommands */
            BitMask<MockClusterSide>().Set(MockClusterSide::kClient).Set(MockClusterSide::kServer)
        ),
        MockClusterConfig(MockClusterId(4), {}, {}, {}, {}, MockClusterSide::kClient),
    }, {
        { kDeviceTypeId2, kDeviceTypeId2Version},
    }, {},
    EndpointComposition::kTree),
    MockEndpointConfig(kMockEndpoint3, {
        MockClusterConfig(MockClusterId(1), {
            ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1),
        }),
        MockClusterConfig(MockClusterId(2), {
            ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1), MockAttributeId(2), MockAttributeId(3), MockAttributeId(4),
        }),
        MockClusterConfig(MockClusterId(3), {
            ClusterRevision::Id, FeatureMap::Id,
            MockAttributeConfig(
              kReadOnlyAttributeId,
              ZCL_INT32U_ATTRIBUTE_TYPE,
              ATTRIBUTE_MASK_NULLABLE    // NOTE: explicltly NOT ATTRIBUTE_MASK_WRITABLE
            )
        }),
        MockClusterConfig(MockClusterId(4), {
            ClusterRevision::Id,
            FeatureMap::Id,
            // several attributes of varying data types for testing.
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_BOOLEAN_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_BITMAP8_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_BITMAP16_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_BITMAP32_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_BITMAP64_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_INT8U_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_INT16U_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_INT24U_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_INT32U_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_INT40U_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_INT48U_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_INT56U_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_INT64U_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_INT8S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_INT16S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_INT24S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_INT32S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_INT40S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_INT48S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_INT56S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_INT64S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_ENUM8_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_ENUM16_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_PRIORITY_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_STATUS_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_SINGLE_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_DOUBLE_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_OCTET_STRING_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_CHAR_STRING_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_ARRAY_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_STRUCT_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_GROUP_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_ENDPOINT_NO_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_VENDOR_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_DEVTYPE_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_FABRIC_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_FABRIC_IDX_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_ENTRY_IDX_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_DATA_VER_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_EVENT_NO_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_SEMTAG_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_NAMESPACE_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_TAG_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_SYSTIME_US_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_SYSTIME_MS_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_ELAPSED_S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_TEMPERATURE_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_POWER_MW_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_AMPERAGE_MA_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_VOLTAGE_MV_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_ENERGY_MWH_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_TOD_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_DATE_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_EPOCH_US_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_EPOCH_S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_POSIX_MS_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_PERCENT_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_PERCENT100THS_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_CLUSTER_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_ATTRIB_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_FIELD_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_EVENT_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_COMMAND_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_ACTION_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_TRANS_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_NODE_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_IPADR_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_IPV4ADR_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_IPV6ADR_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_IPV6PRE_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(ZCL_HWADR_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_BOOLEAN_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_BITMAP8_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_BITMAP16_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_BITMAP32_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_BITMAP64_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_INT8U_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_INT16U_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_INT24U_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_INT32U_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_INT40U_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_INT48U_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_INT56U_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_INT64U_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_INT8S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_INT16S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_INT24S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_INT32S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_INT40S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_INT48S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_INT56S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_INT64S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_ENUM8_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_ENUM16_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_PRIORITY_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_STATUS_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_SINGLE_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_DOUBLE_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_OCTET_STRING_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_CHAR_STRING_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_ARRAY_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_STRUCT_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_GROUP_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_ENDPOINT_NO_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_VENDOR_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_DEVTYPE_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_FABRIC_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_FABRIC_IDX_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_ENTRY_IDX_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_DATA_VER_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_EVENT_NO_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_SEMTAG_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_NAMESPACE_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_TAG_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_SYSTIME_US_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_SYSTIME_MS_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_ELAPSED_S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_TEMPERATURE_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_POWER_MW_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_AMPERAGE_MA_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_VOLTAGE_MV_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_ENERGY_MWH_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_TOD_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_DATE_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_EPOCH_US_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_EPOCH_S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_POSIX_MS_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_PERCENT_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_PERCENT100THS_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_CLUSTER_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_ATTRIB_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_FIELD_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_EVENT_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_COMMAND_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_ACTION_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_TRANS_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_NODE_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_IPADR_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_IPV4ADR_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_IPV6ADR_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_IPV6PRE_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG_NULLABLE(ZCL_HWADR_ATTRIBUTE_TYPE),

            // Special case handling
            MockAttributeConfig(kAttributeIdReadOnly, ZCL_INT32S_ATTRIBUTE_TYPE, 0),
            MockAttributeConfig(kAttributeIdTimedWrite, ZCL_INT32S_ATTRIBUTE_TYPE, ATTRIBUTE_MASK_WRITABLE | ATTRIBUTE_MASK_MUST_USE_TIMED_WRITE),
        }),
    }),
});
// clang-format on

struct UseMockNodeConfig
{
    UseMockNodeConfig(const MockNodeConfig & config) { SetMockNodeConfig(config); }
    ~UseMockNodeConfig() { ResetMockNodeConfig(); }
};

template <typename T>
CHIP_ERROR DecodeList(TLV::TLVReader & reader, std::vector<T> & out)
{
    TLV::TLVType outer;
    ReturnErrorOnFailure(reader.EnterContainer(outer));
    while (true)
    {
        CHIP_ERROR err = reader.Next();

        if (err == CHIP_END_OF_TLV)
        {
            return CHIP_NO_ERROR;
        }
        ReturnErrorOnFailure(err);

        T value;
        ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, value));
        out.emplace_back(std::move(value));
    }
}

class UnsupportedReadAccessInterface : public AttributeAccessInterface
{
public:
    UnsupportedReadAccessInterface(ConcreteAttributePath path) :
        AttributeAccessInterface(MakeOptional(path.mEndpointId), path.mClusterId), mPath(path)
    {}
    ~UnsupportedReadAccessInterface() = default;

    CHIP_ERROR Read(const ConcreteReadAttributePath & path, AttributeValueEncoder & encoder) override
    {
        if (static_cast<const ConcreteAttributePath &>(path) != mPath)
        {
            // returning without trying to handle means "I do not handle this"
            return CHIP_NO_ERROR;
        }

        return CHIP_IM_GLOBAL_STATUS(UnsupportedRead);
    }

private:
    ConcreteAttributePath mPath;
};

class StructAttributeAccessInterface : public AttributeAccessInterface
{
public:
    StructAttributeAccessInterface(ConcreteAttributePath path) :
        AttributeAccessInterface(MakeOptional(path.mEndpointId), path.mClusterId), mPath(path)
    {}
    ~StructAttributeAccessInterface() = default;

    CHIP_ERROR Read(const ConcreteReadAttributePath & path, AttributeValueEncoder & encoder) override
    {
        if (static_cast<const ConcreteAttributePath &>(path) != mPath)
        {
            // returning without trying to handle means "I do not handle this"
            return CHIP_NO_ERROR;
        }

        return encoder.Encode(mData);
    }

    CHIP_ERROR Write(const ConcreteDataAttributePath & path, AttributeValueDecoder & decoder) override
    {
        if (static_cast<const ConcreteAttributePath &>(path) != mPath)
        {
            // returning without trying to handle means "I do not handle this"
            return CHIP_NO_ERROR;
        }

        return decoder.Decode(mData);
    }

    void SetReturnedData(const Clusters::UnitTesting::Structs::SimpleStruct::Type & data) { mData = data; }
    const Clusters::UnitTesting::Structs::SimpleStruct::Type & GetData() const { return mData; }

private:
    ConcreteAttributePath mPath;
    Clusters::UnitTesting::Structs::SimpleStruct::Type mData;
};

class ErrorAccessInterface : public AttributeAccessInterface
{
public:
    ErrorAccessInterface(ConcreteAttributePath path, CHIP_ERROR err) :
        AttributeAccessInterface(MakeOptional(path.mEndpointId), path.mClusterId), mPath(path), mError(err)
    {}
    ~ErrorAccessInterface() = default;

    CHIP_ERROR Read(const ConcreteReadAttributePath & path, AttributeValueEncoder & encoder) override
    {
        if (static_cast<const ConcreteAttributePath &>(path) != mPath)
        {
            // returning without trying to handle means "I do not handle this"
            return CHIP_NO_ERROR;
        }
        return mError;
    }

    CHIP_ERROR Write(const ConcreteDataAttributePath & path, AttributeValueDecoder & decoder) override
    {
        if (static_cast<const ConcreteAttributePath &>(path) != mPath)
        {
            // returning without trying to handle means "I do not handle this"
            return CHIP_NO_ERROR;
        }
        return mError;
    }

private:
    ConcreteAttributePath mPath;
    CHIP_ERROR mError;
};

class ListAttributeAcessInterface : public AttributeAccessInterface
{
public:
    ListAttributeAcessInterface(ConcreteAttributePath path) :
        AttributeAccessInterface(MakeOptional(path.mEndpointId), path.mClusterId), mPath(path)
    {}
    ~ListAttributeAcessInterface() = default;

    CHIP_ERROR Read(const ConcreteReadAttributePath & path, AttributeValueEncoder & encoder) override
    {
        if (static_cast<const ConcreteAttributePath &>(path) != mPath)
        {
            // returning without trying to handle means "I do not handle this"
            return CHIP_NO_ERROR;
        }

        return encoder.EncodeList([this](const auto & listEncoder) {
            for (unsigned i = 0; i < mCount; i++)
            {
                mData.a = static_cast<uint8_t>(i % 0xFF);
                ReturnErrorOnFailure(listEncoder.Encode(mData));
            }
            return CHIP_NO_ERROR;
        });
    }

    void SetReturnedData(const Clusters::UnitTesting::Structs::SimpleStruct::Type & data) { mData = data; }
    void SetReturnedDataCount(unsigned count) { mCount = count; }

private:
    ConcreteAttributePath mPath;
    Clusters::UnitTesting::Structs::SimpleStruct::Type mData;
    unsigned mCount = 0;
};

/// RAII registration of an attribute access interface
template <typename T>
class RegisteredAttributeAccessInterface
{
public:
    template <typename... Args>
    RegisteredAttributeAccessInterface(Args &&... args) : mData(std::forward<Args>(args)...)
    {
        VerifyOrDie(AttributeAccessInterfaceRegistry::Instance().Register(&mData));
    }
    ~RegisteredAttributeAccessInterface() { AttributeAccessInterfaceRegistry::Instance().Unregister(&mData); }

    T * operator->() { return &mData; }
    T & operator*() { return mData; }

private:
    T mData;
};

template <typename T, EmberAfAttributeType ZclType>
void TestEmberScalarTypeRead(typename NumericAttributeTraits<T>::WorkingType value)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    ReadOperation testRequest(kMockEndpoint3, MockClusterId(4), MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZclType));
    testRequest.SetSubjectDescriptor(kAdminSubjectDescriptor);

    // Ember encoding for integers is IDENTICAL to the in-memory representation for them
    typename NumericAttributeTraits<T>::StorageType storage;
    NumericAttributeTraits<T>::WorkingToStorage(value, storage);
    chip::Test::SetEmberReadOutput(ByteSpan(reinterpret_cast<const uint8_t *>(&storage), sizeof(storage)));

    // Data read via the encoder
    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding();
    ASSERT_EQ(model.ReadAttribute(testRequest.GetRequest(), *encoder), CHIP_NO_ERROR);
    ASSERT_EQ(testRequest.FinishEncoding(), CHIP_NO_ERROR);

    // Validate after read
    std::vector<DecodedAttributeData> attribute_data;
    ASSERT_EQ(testRequest.GetEncodedIBs().Decode(attribute_data), CHIP_NO_ERROR);
    ASSERT_EQ(attribute_data.size(), 1u);

    DecodedAttributeData & encodedData = attribute_data[0];
    ASSERT_EQ(encodedData.attributePath, testRequest.GetRequest().path);

    typename NumericAttributeTraits<T>::WorkingType actual;
    ASSERT_EQ(chip::app::DataModel::Decode<typename NumericAttributeTraits<T>::WorkingType>(encodedData.dataReader, actual),
              CHIP_NO_ERROR);
    ASSERT_EQ(actual, value);
}

template <typename T, EmberAfAttributeType ZclType>
void TestEmberScalarNullRead()
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    ReadOperation testRequest(kMockEndpoint3, MockClusterId(4), MOCK_ATTRIBUTE_ID_FOR_NULLABLE_TYPE(ZclType));
    testRequest.SetSubjectDescriptor(kAdminSubjectDescriptor);

    // Ember encoding for integers is IDENTICAL to the in-memory representation for them
    typename NumericAttributeTraits<T>::StorageType nullValue;
    NumericAttributeTraits<T>::SetNull(nullValue);
    chip::Test::SetEmberReadOutput(ByteSpan(reinterpret_cast<const uint8_t *>(&nullValue), sizeof(nullValue)));

    // Data read via the encoder
    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding();
    ASSERT_EQ(model.ReadAttribute(testRequest.GetRequest(), *encoder), CHIP_NO_ERROR);
    ASSERT_EQ(testRequest.FinishEncoding(), CHIP_NO_ERROR);

    // Validate after read
    std::vector<DecodedAttributeData> attribute_data;
    ASSERT_EQ(testRequest.GetEncodedIBs().Decode(attribute_data), CHIP_NO_ERROR);
    ASSERT_EQ(attribute_data.size(), 1u);

    DecodedAttributeData & encodedData = attribute_data[0];
    ASSERT_EQ(encodedData.attributePath, testRequest.GetRequest().path);
    chip::app::DataModel::Nullable<typename NumericAttributeTraits<T>::WorkingType> actual;
    ASSERT_EQ(chip::app::DataModel::Decode(encodedData.dataReader, actual), CHIP_NO_ERROR);
    ASSERT_TRUE(actual.IsNull());
}

template <typename T, EmberAfAttributeType ZclType>
void TestEmberScalarTypeWrite(const typename NumericAttributeTraits<T>::WorkingType value)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    // non-nullable test
    {
        WriteOperation test(kMockEndpoint3, MockClusterId(4), MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZclType));
        test.SetSubjectDescriptor(kAdminSubjectDescriptor);

        AttributeValueDecoder decoder = test.DecoderFor(value);

        // write should succeed
        ASSERT_TRUE(model.WriteAttribute(test.GetRequest(), decoder).IsSuccess());

        // Validate data after write
        chip::ByteSpan writtenData = Test::GetEmberBuffer();

        typename NumericAttributeTraits<T>::StorageType storage;
        ASSERT_GE(writtenData.size(), sizeof(storage));
        memcpy(&storage, writtenData.data(), sizeof(storage));
        typename NumericAttributeTraits<T>::WorkingType actual = NumericAttributeTraits<T>::StorageToWorking(storage);

        EXPECT_EQ(actual, value);
        ASSERT_EQ(model.ChangeListener().DirtyList().size(), 1u);
        EXPECT_EQ(model.ChangeListener().DirtyList()[0],
                  AttributePathParams(test.GetRequest().path.mEndpointId, test.GetRequest().path.mClusterId,
                                      test.GetRequest().path.mAttributeId));

        // reset for the next test
        model.ChangeListener().DirtyList().clear();
    }

    // nullable test: write null to make sure content of buffer changed (otherwise it will be a noop for dirty checking)
    {
        WriteOperation test(kMockEndpoint3, MockClusterId(4), MOCK_ATTRIBUTE_ID_FOR_NULLABLE_TYPE(ZclType));
        test.SetSubjectDescriptor(kAdminSubjectDescriptor);

        using NumericType             = NumericAttributeTraits<T>;
        using NullableType            = chip::app::DataModel::Nullable<typename NumericType::WorkingType>;
        AttributeValueDecoder decoder = test.DecoderFor<NullableType>(NullableType());

        // write should succeed
        ASSERT_EQ(model.WriteAttribute(test.GetRequest(), decoder), CHIP_NO_ERROR);

        // dirty: we changed the value to null
        ASSERT_EQ(model.ChangeListener().DirtyList().size(), 1u);
        EXPECT_EQ(model.ChangeListener().DirtyList()[0],
                  AttributePathParams(test.GetRequest().path.mEndpointId, test.GetRequest().path.mClusterId,
                                      test.GetRequest().path.mAttributeId));
    }

    // nullable test
    {
        WriteOperation test(kMockEndpoint3, MockClusterId(4), MOCK_ATTRIBUTE_ID_FOR_NULLABLE_TYPE(ZclType));
        test.SetSubjectDescriptor(kAdminSubjectDescriptor);

        AttributeValueDecoder decoder = test.DecoderFor(value);

        // write should succeed
        ASSERT_EQ(model.WriteAttribute(test.GetRequest(), decoder), CHIP_NO_ERROR);

        // Validate data after write
        chip::ByteSpan writtenData = Test::GetEmberBuffer();

        typename NumericAttributeTraits<T>::StorageType storage;
        ASSERT_GE(writtenData.size(), sizeof(storage));
        memcpy(&storage, writtenData.data(), sizeof(storage));
        typename NumericAttributeTraits<T>::WorkingType actual = NumericAttributeTraits<T>::StorageToWorking(storage);

        ASSERT_EQ(actual, value);
        // dirty a 2nd time when we moved from null to a real value
        ASSERT_EQ(model.ChangeListener().DirtyList().size(), 2u);
        EXPECT_EQ(model.ChangeListener().DirtyList()[1],
                  AttributePathParams(test.GetRequest().path.mEndpointId, test.GetRequest().path.mClusterId,
                                      test.GetRequest().path.mAttributeId));
    }
}

template <typename T, EmberAfAttributeType ZclType>
void TestEmberScalarNullWrite()
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    WriteOperation test(kMockEndpoint3, MockClusterId(4), MOCK_ATTRIBUTE_ID_FOR_NULLABLE_TYPE(ZclType));
    test.SetSubjectDescriptor(kAdminSubjectDescriptor);

    using NumericType             = NumericAttributeTraits<T>;
    using NullableType            = chip::app::DataModel::Nullable<typename NumericType::WorkingType>;
    AttributeValueDecoder decoder = test.DecoderFor<NullableType>(NullableType());

    // write should succeed
    ASSERT_TRUE(model.WriteAttribute(test.GetRequest(), decoder).IsSuccess());

    // Validate data after write
    chip::ByteSpan writtenData = Test::GetEmberBuffer();

    using Traits = NumericAttributeTraits<T>;

    typename Traits::StorageType storage;
    ASSERT_GE(writtenData.size(), sizeof(storage));
    memcpy(&storage, writtenData.data(), sizeof(storage));
    ASSERT_TRUE(Traits::IsNullValue(storage));
}

template <typename T, EmberAfAttributeType ZclType>
void TestEmberScalarTypeWriteNullValueToNullable()
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    WriteOperation test(kMockEndpoint3, MockClusterId(4), MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZclType));
    test.SetSubjectDescriptor(kAdminSubjectDescriptor);

    using NumericType             = NumericAttributeTraits<T>;
    using NullableType            = chip::app::DataModel::Nullable<typename NumericType::WorkingType>;
    AttributeValueDecoder decoder = test.DecoderFor<NullableType>(NullableType());

    // write should fail: we are trying to write null
    ASSERT_EQ(model.WriteAttribute(test.GetRequest(), decoder), CHIP_ERROR_WRONG_TLV_TYPE);
}

uint16_t ReadLe16(const void * buffer)
{
    const uint8_t * p = reinterpret_cast<const uint8_t *>(buffer);
    return chip::Encoding::LittleEndian::Read16(p);
}

void WriteLe16(void * buffer, uint16_t value)
{
    uint8_t * p = reinterpret_cast<uint8_t *>(buffer);
    chip::Encoding::LittleEndian::Write16(p, value);
}

} // namespace

TEST_F(TestCodegenModelViaMocks, IterateOverEndpoints)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;

    // This iteration relies on the hard-coding that occurs when mock_ember is used
    DataModel::ListBuilder<DataModel::EndpointEntry> endpointsBuilder;

    ASSERT_EQ(model.Endpoints(endpointsBuilder), CHIP_NO_ERROR);

    auto endpoints = endpointsBuilder.TakeBuffer();

    ASSERT_EQ(endpoints.size(), 3u);

    EXPECT_EQ(endpoints[0].id, kMockEndpoint1);
    EXPECT_EQ(endpoints[0].parentId, kInvalidEndpointId);
    EXPECT_EQ(endpoints[0].compositionPattern, EndpointCompositionPattern::kFullFamily);

    EXPECT_EQ(endpoints[1].id, kMockEndpoint2);
    EXPECT_EQ(endpoints[1].parentId, kInvalidEndpointId);
    EXPECT_EQ(endpoints[1].compositionPattern, EndpointCompositionPattern::kTree);

    EXPECT_EQ(endpoints[2].id, kMockEndpoint3);
    EXPECT_EQ(endpoints[2].parentId, kInvalidEndpointId);
    EXPECT_EQ(endpoints[2].compositionPattern, EndpointCompositionPattern::kFullFamily);
}

TEST_F(TestCodegenModelViaMocks, IterateOverServerClusters)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;

    chip::Test::ResetVersion();

    DataModel::ListBuilder<DataModel::ServerClusterEntry> builder;

    EXPECT_NE(model.ServerClusters(kEndpointIdThatIsMissing, builder), CHIP_NO_ERROR);
    EXPECT_TRUE(builder.IsEmpty());
    EXPECT_NE(model.ServerClusters(kInvalidEndpointId, builder), CHIP_NO_ERROR);
    EXPECT_TRUE(builder.IsEmpty());

    // mock endpoint 1 has 2 mock clusters: 1 and 2
    EXPECT_EQ(model.ServerClusters(kMockEndpoint1, builder), CHIP_NO_ERROR);
    auto serverClusters = builder.TakeBuffer();
    ASSERT_EQ(serverClusters.size(), 2u);

    EXPECT_EQ(serverClusters[0].clusterId, MockClusterId(1));
    EXPECT_EQ(serverClusters[0].dataVersion, 0u);
    EXPECT_EQ(serverClusters[0].flags.Raw(), 0u);

    EXPECT_EQ(serverClusters[1].clusterId, MockClusterId(2));
    EXPECT_EQ(serverClusters[1].dataVersion, 0u);
    EXPECT_EQ(serverClusters[1].flags.Raw(), 0u);

    chip::Test::BumpVersion();

    EXPECT_EQ(model.ServerClusters(kMockEndpoint1, builder), CHIP_NO_ERROR);
    serverClusters = builder.TakeBuffer();
    ASSERT_EQ(serverClusters.size(), 2u);
    EXPECT_EQ(serverClusters[0].dataVersion, 1u);
    EXPECT_EQ(serverClusters[1].dataVersion, 1u);

    // mock endpoint 3 has 4 mock clusters: 1 through 4
    EXPECT_EQ(model.ServerClusters(kMockEndpoint3, builder), CHIP_NO_ERROR);
    serverClusters = builder.TakeBuffer();
    ASSERT_EQ(serverClusters.size(), 4u);
    EXPECT_EQ(serverClusters[0].clusterId, MockClusterId(1));
    EXPECT_EQ(serverClusters[1].clusterId, MockClusterId(2));
    EXPECT_EQ(serverClusters[2].clusterId, MockClusterId(3));
    EXPECT_EQ(serverClusters[3].clusterId, MockClusterId(4));
}

TEST_F(TestCodegenModelViaMocks, IterateOverClientClusters)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;

    DataModel::ListBuilder<ClusterId> builder;

    EXPECT_EQ(model.ClientClusters(kEndpointIdThatIsMissing, builder), CHIP_ERROR_NOT_FOUND);
    EXPECT_TRUE(builder.IsEmpty());
    EXPECT_EQ(model.ClientClusters(kInvalidEndpointId, builder), CHIP_ERROR_NOT_FOUND);
    EXPECT_TRUE(builder.IsEmpty());

    // mock endpoint 1 has 2 mock client clusters: 3 and 4
    EXPECT_EQ(model.ClientClusters(kMockEndpoint1, builder), CHIP_NO_ERROR);
    auto clientClusters = builder.TakeBuffer();

    const ClusterId kExpectedClusters1[] = { MockClusterId(3), MockClusterId(4) };
    ASSERT_TRUE(clientClusters.data_equal(Span<const ClusterId>(kExpectedClusters1)));

    // mock endpoint 2 has 1 mock client clusters: 3(has server side at the same time) and 4
    EXPECT_EQ(model.ClientClusters(kMockEndpoint2, builder), CHIP_NO_ERROR);
    clientClusters = builder.TakeBuffer();

    const ClusterId kExpectedClusters2[] = { MockClusterId(3), MockClusterId(4) };
    ASSERT_TRUE(clientClusters.data_equal(Span<const ClusterId>(kExpectedClusters2)));
}

TEST_F(TestCodegenModelViaMocks, IterateOverAttributes)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;

    // invalid paths should return in "no more data"
    ASSERT_TRUE(model.AttributesIgnoreError(ConcreteClusterPath(kEndpointIdThatIsMissing, MockClusterId(1))).empty());
    ASSERT_TRUE(model.AttributesIgnoreError(ConcreteClusterPath(kInvalidEndpointId, MockClusterId(1))).empty());
    ASSERT_TRUE(model.AttributesIgnoreError(ConcreteClusterPath(kMockEndpoint1, MockClusterId(10))).empty());
    ASSERT_TRUE(model.AttributesIgnoreError(ConcreteClusterPath(kMockEndpoint1, kInvalidClusterId)).empty());

    // should be able to iterate over valid paths
    DataModel::ListBuilder<DataModel::AttributeEntry> builder;

    // invalid paths return errors
    ASSERT_EQ(model.Attributes(ConcreteClusterPath(kEndpointIdThatIsMissing, MockClusterId(1)), builder), CHIP_ERROR_NOT_FOUND);
    ASSERT_EQ(model.Attributes(ConcreteClusterPath(kInvalidEndpointId, MockClusterId(1)), builder), CHIP_ERROR_NOT_FOUND);
    ASSERT_EQ(model.Attributes(ConcreteClusterPath(kMockEndpoint1, MockClusterId(10)), builder), CHIP_ERROR_NOT_FOUND);
    ASSERT_EQ(model.Attributes(ConcreteClusterPath(kMockEndpoint1, kInvalidClusterId), builder), CHIP_ERROR_NOT_FOUND);

    EXPECT_EQ(model.Attributes(ConcreteClusterPath(kMockEndpoint2, MockClusterId(2)), builder), CHIP_NO_ERROR);
    auto attributes = builder.TakeBuffer();
    ASSERT_EQ(attributes.size(), 7u);

    ASSERT_EQ(attributes[0].attributeId, ClusterRevision::Id);
    ASSERT_FALSE(attributes[0].flags.Has(AttributeQualityFlags::kListAttribute));

    ASSERT_EQ(attributes[1].attributeId, FeatureMap::Id);
    ASSERT_FALSE(attributes[1].flags.Has(AttributeQualityFlags::kListAttribute));

    ASSERT_EQ(attributes[2].attributeId, MockAttributeId(1));
    ASSERT_FALSE(attributes[2].flags.Has(AttributeQualityFlags::kListAttribute));

    ASSERT_EQ(attributes[3].attributeId, MockAttributeId(2));
    ASSERT_TRUE(attributes[3].flags.Has(AttributeQualityFlags::kListAttribute));

    // Ends with global list attributes
    ASSERT_EQ(attributes[4].attributeId, GeneratedCommandList::Id);
    ASSERT_TRUE(attributes[4].flags.Has(AttributeQualityFlags::kListAttribute));

    ASSERT_EQ(attributes[5].attributeId, AcceptedCommandList::Id);
    ASSERT_TRUE(attributes[5].flags.Has(AttributeQualityFlags::kListAttribute));

    ASSERT_EQ(attributes[6].attributeId, AttributeList::Id);
    ASSERT_TRUE(attributes[6].flags.Has(AttributeQualityFlags::kListAttribute));
}

TEST_F(TestCodegenModelViaMocks, FindAttribute)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;

    AttributeFinder finder(&model);

    // various non-existent or invalid paths should return no info data
    ASSERT_FALSE(finder.Find(ConcreteAttributePath(kInvalidEndpointId, kInvalidClusterId, kInvalidAttributeId)).has_value());
    ASSERT_FALSE(finder.Find(ConcreteAttributePath(kInvalidEndpointId, kInvalidClusterId, FeatureMap::Id)).has_value());
    ASSERT_FALSE(finder.Find(ConcreteAttributePath(kInvalidEndpointId, MockClusterId(1), FeatureMap::Id)).has_value());
    ASSERT_FALSE(finder.Find(ConcreteAttributePath(kMockEndpoint1, kInvalidClusterId, FeatureMap::Id)).has_value());
    ASSERT_FALSE(finder.Find(ConcreteAttributePath(kMockEndpoint1, MockClusterId(10), FeatureMap::Id)).has_value());
    ASSERT_FALSE(finder.Find(ConcreteAttributePath(kMockEndpoint1, MockClusterId(10), kInvalidAttributeId)).has_value());
    ASSERT_FALSE(finder.Find(ConcreteAttributePath(kMockEndpoint1, MockClusterId(1), MockAttributeId(10))).has_value());

    // valid info
    std::optional<AttributeEntry> info = finder.Find(ConcreteAttributePath(kMockEndpoint1, MockClusterId(1), FeatureMap::Id));
    ASSERT_TRUE(info.has_value());
    EXPECT_FALSE(info->flags.Has(AttributeQualityFlags::kListAttribute)); // NOLINT(bugprone-unchecked-optional-access)

    // Mocks always set everything as R/W with administrative privileges
    EXPECT_EQ(info->readPrivilege, chip::Access::Privilege::kAdminister);  // NOLINT(bugprone-unchecked-optional-access)
    EXPECT_EQ(info->writePrivilege, chip::Access::Privilege::kAdminister); // NOLINT(bugprone-unchecked-optional-access)

    info = finder.Find(ConcreteAttributePath(kMockEndpoint2, MockClusterId(2), MockAttributeId(2)));
    ASSERT_TRUE(info.has_value());
    EXPECT_TRUE(info->flags.Has(AttributeQualityFlags::kListAttribute));   // NOLINT(bugprone-unchecked-optional-access)
    EXPECT_EQ(info->readPrivilege, chip::Access::Privilege::kAdminister);  // NOLINT(bugprone-unchecked-optional-access)
    EXPECT_EQ(info->writePrivilege, chip::Access::Privilege::kAdminister); // NOLINT(bugprone-unchecked-optional-access)

    // test a read-only attribute, which will not have a write privilege
    info = finder.Find(ConcreteAttributePath(kMockEndpoint3, MockClusterId(3), kReadOnlyAttributeId));
    ASSERT_TRUE(info.has_value());
    EXPECT_FALSE(info->flags.Has(AttributeQualityFlags::kListAttribute)); // NOLINT(bugprone-unchecked-optional-access)
    EXPECT_EQ(info->readPrivilege, chip::Access::Privilege::kAdminister); // NOLINT(bugprone-unchecked-optional-access)
    EXPECT_FALSE(info->writePrivilege.has_value());                       // NOLINT(bugprone-unchecked-optional-access)
}

// global attributes are EXPLICITLY supported
TEST_F(TestCodegenModelViaMocks, GlobalAttributeInfo)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;

    AttributeFinder finder(&model);

    std::optional<AttributeEntry> info = finder.Find(
        ConcreteAttributePath(kMockEndpoint1, MockClusterId(1), Clusters::Globals::Attributes::GeneratedCommandList::Id));

    ASSERT_TRUE(info.has_value());

    info = finder.Find(ConcreteAttributePath(kMockEndpoint1, MockClusterId(1), Clusters::Globals::Attributes::AttributeList::Id));
    ASSERT_TRUE(info.has_value());

    info = finder.Find(
        ConcreteAttributePath(kMockEndpoint1, MockClusterId(1), Clusters::Globals::Attributes::AcceptedCommandList::Id));
    ASSERT_TRUE(info.has_value());
}

TEST_F(TestCodegenModelViaMocks, IterateOverAcceptedCommands)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;

    DataModel::ListBuilder<DataModel::AcceptedCommandEntry> builder;

    // invalid paths should return in "no more data"
    ASSERT_EQ(model.AcceptedCommands(ConcreteClusterPath(kEndpointIdThatIsMissing, MockClusterId(1)), builder),
              CHIP_ERROR_NOT_FOUND);
    ASSERT_TRUE(builder.IsEmpty());
    ASSERT_EQ(model.AcceptedCommands(ConcreteClusterPath(kInvalidEndpointId, MockClusterId(1)), builder), CHIP_ERROR_NOT_FOUND);
    ASSERT_TRUE(builder.IsEmpty());
    ASSERT_EQ(model.AcceptedCommands(ConcreteClusterPath(kMockEndpoint1, MockClusterId(10)), builder), CHIP_ERROR_NOT_FOUND);
    ASSERT_TRUE(builder.IsEmpty());
    ASSERT_EQ(model.AcceptedCommands(ConcreteClusterPath(kMockEndpoint1, kInvalidClusterId), builder), CHIP_ERROR_NOT_FOUND);
    ASSERT_TRUE(builder.IsEmpty());

    ASSERT_EQ(model.AcceptedCommands(ConcreteClusterPath(kMockEndpoint2, MockClusterId(2)), builder), CHIP_NO_ERROR);
    ASSERT_EQ(builder.Size(), 3u);

    auto cmds = builder.TakeBuffer();

    // took ownership
    ASSERT_EQ(builder.Size(), 0u);
    ASSERT_TRUE(builder.IsEmpty());

    ASSERT_EQ(cmds.size(), 3u);
    ASSERT_EQ(cmds[0].commandId, 1u);
    ASSERT_EQ(cmds[1].commandId, 2u);
    ASSERT_EQ(cmds[2].commandId, 23u);
}

TEST_F(TestCodegenModelViaMocks, IterateOverGeneratedCommands)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;

    DataModel::ListBuilder<CommandId> builder;

    // invalid paths should return in "no more data"
    ASSERT_EQ(model.GeneratedCommands(ConcreteClusterPath(kEndpointIdThatIsMissing, MockClusterId(1)), builder),
              CHIP_ERROR_NOT_FOUND);
    ASSERT_TRUE(builder.IsEmpty());
    ASSERT_EQ(model.GeneratedCommands(ConcreteClusterPath(kInvalidEndpointId, MockClusterId(1)), builder), CHIP_ERROR_NOT_FOUND);
    ASSERT_TRUE(builder.IsEmpty());
    ASSERT_EQ(model.GeneratedCommands(ConcreteClusterPath(kMockEndpoint1, MockClusterId(10)), builder), CHIP_ERROR_NOT_FOUND);
    ASSERT_TRUE(builder.IsEmpty());
    ASSERT_EQ(model.GeneratedCommands(ConcreteClusterPath(kMockEndpoint1, kInvalidClusterId), builder), CHIP_ERROR_NOT_FOUND);
    ASSERT_TRUE(builder.IsEmpty());

    // should be able to iterate over valid paths
    ASSERT_EQ(model.GeneratedCommands(ConcreteClusterPath(kMockEndpoint2, MockClusterId(2)), builder), CHIP_NO_ERROR);
    auto cmds = builder.TakeBuffer();

    const CommandId expectedCommands2[] = { 2, 10 };
    ASSERT_TRUE(cmds.data_equal(Span<const CommandId>(expectedCommands2)));

    ASSERT_EQ(model.GeneratedCommands(ConcreteClusterPath(kMockEndpoint2, MockClusterId(3)), builder), CHIP_NO_ERROR);
    cmds                                = builder.TakeBuffer();
    const CommandId expectedCommands3[] = { 4, 6 };
    ASSERT_TRUE(cmds.data_equal(Span<const CommandId>(expectedCommands3)));
}

TEST_F(TestCodegenModelViaMocks, AcceptedGeneratedCommandsOnInvalidEndpoints)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;

    // register a CHI on ALL endpoints
    CustomListCommandHandler handler(chip::NullOptional, MockClusterId(1));
    handler.SetHandleCommands(true);

    DataModel::ListBuilder<CommandId> generatedBuilder;
    DataModel::ListBuilder<DataModel::AcceptedCommandEntry> acceptedBuilder;

    // valid endpoint will result in valid data (even though list is empty)
    ASSERT_EQ(model.GeneratedCommands(ConcreteClusterPath(kMockEndpoint1, MockClusterId(1)), generatedBuilder), CHIP_NO_ERROR);
    ASSERT_TRUE(generatedBuilder.IsEmpty());
    ASSERT_EQ(model.AcceptedCommands(ConcreteClusterPath(kMockEndpoint1, MockClusterId(1)), acceptedBuilder), CHIP_NO_ERROR);
    ASSERT_TRUE(acceptedBuilder.IsEmpty());

    // Invalid endpoint fails - we will get no commands there (even though CHI is registered)
    ASSERT_EQ(model.GeneratedCommands(ConcreteClusterPath(kEndpointIdThatIsMissing, MockClusterId(1)), generatedBuilder),
              CHIP_ERROR_NOT_FOUND);
    ASSERT_EQ(model.AcceptedCommands(ConcreteClusterPath(kEndpointIdThatIsMissing, MockClusterId(1)), acceptedBuilder),
              CHIP_ERROR_NOT_FOUND);

    // same for invalid cluster ID
    ASSERT_EQ(model.GeneratedCommands(ConcreteClusterPath(kMockEndpoint1, MockClusterId(0x1123)), generatedBuilder),
              CHIP_ERROR_NOT_FOUND);
    ASSERT_EQ(model.AcceptedCommands(ConcreteClusterPath(kMockEndpoint1, MockClusterId(0x1123)), acceptedBuilder),
              CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestCodegenModelViaMocks, CommandHandlerInterfaceCommandHandling)
{

    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;

    // Command handler interface is capable to override accepted and generated commands.
    // Validate that these work
    CustomListCommandHandler handler(MakeOptional(kMockEndpoint1), MockClusterId(1));

    DataModel::ListBuilder<CommandId> generatedBuilder;
    DataModel::ListBuilder<DataModel::AcceptedCommandEntry> acceptedBuilder;

    // At this point, without overrides, there should be no accepted/generated commands
    ASSERT_EQ(model.GeneratedCommands(ConcreteClusterPath(kMockEndpoint1, MockClusterId(1)), generatedBuilder), CHIP_NO_ERROR);
    ASSERT_TRUE(generatedBuilder.IsEmpty());
    ASSERT_EQ(model.AcceptedCommands(ConcreteClusterPath(kMockEndpoint1, MockClusterId(1)), acceptedBuilder), CHIP_NO_ERROR);
    ASSERT_TRUE(acceptedBuilder.IsEmpty());

    handler.SetOverrideAccepted(true);
    handler.SetOverrideGenerated(true);

    // with overrides, the list is still empty ...
    ASSERT_EQ(model.GeneratedCommands(ConcreteClusterPath(kMockEndpoint1, MockClusterId(1)), generatedBuilder), CHIP_NO_ERROR);
    ASSERT_TRUE(generatedBuilder.IsEmpty());
    ASSERT_EQ(model.AcceptedCommands(ConcreteClusterPath(kMockEndpoint1, MockClusterId(1)), acceptedBuilder), CHIP_NO_ERROR);
    ASSERT_TRUE(acceptedBuilder.IsEmpty());

    // set some overrides
    handler.AcceptedVec().push_back(1234);
    handler.AcceptedVec().push_back(999);

    handler.GeneratedVec().push_back(33);

    ASSERT_EQ(model.AcceptedCommands(ConcreteClusterPath(kMockEndpoint1, MockClusterId(1)), acceptedBuilder), CHIP_NO_ERROR);
    auto acceptedCommands = acceptedBuilder.TakeBuffer();

    ASSERT_EQ(acceptedCommands.size(), 2u);
    ASSERT_EQ(acceptedCommands[0].commandId, 1234u);
    ASSERT_EQ(acceptedCommands[1].commandId, 999u);

    ASSERT_EQ(model.GeneratedCommands(ConcreteClusterPath(kMockEndpoint1, MockClusterId(1)), generatedBuilder), CHIP_NO_ERROR);
    auto generatedCommands                      = generatedBuilder.TakeBuffer();
    const CommandId expectedGeneratedCommands[] = { 33 };
    ASSERT_TRUE(generatedCommands.data_equal(Span<const CommandId>(expectedGeneratedCommands)));
}

TEST_F(TestCodegenModelViaMocks, ReadForInvalidGlobalAttributePath)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    {
        ReadOperation testRequest(kEndpointIdThatIsMissing, MockClusterId(1), AttributeList::Id);
        testRequest.SetSubjectDescriptor(kAdminSubjectDescriptor);

        std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding();
        ASSERT_EQ(model.ReadAttribute(testRequest.GetRequest(), *encoder), Status::UnsupportedEndpoint);
    }

    {
        ReadOperation testRequest(kMockEndpoint1, kInvalidClusterId, AttributeList::Id);
        testRequest.SetSubjectDescriptor(kAdminSubjectDescriptor);

        std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding();
        ASSERT_EQ(model.ReadAttribute(testRequest.GetRequest(), *encoder), Status::UnsupportedCluster);
    }
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeInvalidRead)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    // Invalid attribute
    {
        ReadOperation testRequest(kMockEndpoint1, MockClusterId(1), MockAttributeId(10));
        testRequest.SetSubjectDescriptor(kAdminSubjectDescriptor);

        std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding();
        ASSERT_EQ(model.ReadAttribute(testRequest.GetRequest(), *encoder), Status::UnsupportedAttribute);
    }

    // Invalid cluster
    {
        ReadOperation testRequest(kMockEndpoint1, MockClusterId(100), MockAttributeId(1));
        testRequest.SetSubjectDescriptor(kAdminSubjectDescriptor);
        std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding();

        ASSERT_EQ(model.ReadAttribute(testRequest.GetRequest(), *encoder), Status::UnsupportedCluster);
    }

    // Invalid endpoint
    {
        ReadOperation testRequest(kEndpointIdThatIsMissing, MockClusterId(1), MockAttributeId(1));
        testRequest.SetSubjectDescriptor(kAdminSubjectDescriptor);
        std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding();

        ASSERT_EQ(model.ReadAttribute(testRequest.GetRequest(), *encoder), Status::UnsupportedEndpoint);
    }
}

TEST_F(TestCodegenModelViaMocks, AccessInterfaceUnsupportedRead)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    const ConcreteAttributePath kTestPath(kMockEndpoint3, MockClusterId(4),
                                          MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_STRUCT_ATTRIBUTE_TYPE));

    ReadOperation testRequest(kTestPath);
    testRequest.SetSubjectDescriptor(kAdminSubjectDescriptor);
    testRequest.SetPathExpanded(true);

    RegisteredAttributeAccessInterface<UnsupportedReadAccessInterface> aai(kTestPath);

    // For expanded paths, unsupported read from AAI (i.e. reading write-only data)
    // succeed without attempting to encode.
    // This is temporary until ACL checks are moved inside the IM/ReportEngine
    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding();
    ASSERT_EQ(model.ReadAttribute(testRequest.GetRequest(), *encoder), CHIP_NO_ERROR);
    ASSERT_FALSE(encoder->TriedEncode());
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeReadInt32S)
{
    TestEmberScalarTypeRead<int32_t, ZCL_INT32S_ATTRIBUTE_TYPE>(-1234);
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeReadEnum16)
{
    TestEmberScalarTypeRead<uint16_t, ZCL_ENUM16_ATTRIBUTE_TYPE>(0x1234);
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeReadFloat)
{
    TestEmberScalarTypeRead<float, ZCL_SINGLE_ATTRIBUTE_TYPE>(0.625);
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeReadDouble)
{
    TestEmberScalarTypeRead<double, ZCL_DOUBLE_ATTRIBUTE_TYPE>(0.625);
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeReadInt24U)
{
    TestEmberScalarTypeRead<OddSizedInteger<3, false>, ZCL_INT24U_ATTRIBUTE_TYPE>(0x1234AB);
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeReadInt32U)
{
    TestEmberScalarTypeRead<uint32_t, ZCL_INT32U_ATTRIBUTE_TYPE>(0x1234ABCD);
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeReadInt40U)
{
    TestEmberScalarTypeRead<OddSizedInteger<5, false>, ZCL_INT40U_ATTRIBUTE_TYPE>(0x1122334455);
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeReadInt48U)
{
    TestEmberScalarTypeRead<OddSizedInteger<6, false>, ZCL_INT48U_ATTRIBUTE_TYPE>(0xAABB11223344);
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeReadInt56U)
{
    TestEmberScalarTypeRead<OddSizedInteger<7, false>, ZCL_INT56U_ATTRIBUTE_TYPE>(0xAABB11223344);
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeReadBool)
{
    TestEmberScalarTypeRead<bool, ZCL_BOOLEAN_ATTRIBUTE_TYPE>(true);
    TestEmberScalarTypeRead<bool, ZCL_BOOLEAN_ATTRIBUTE_TYPE>(false);
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeReadInt8U)
{
    TestEmberScalarTypeRead<uint8_t, ZCL_INT8U_ATTRIBUTE_TYPE>(0x12);
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeReadNulls)
{
    TestEmberScalarNullRead<uint8_t, ZCL_INT8U_ATTRIBUTE_TYPE>();
    TestEmberScalarNullRead<uint16_t, ZCL_INT16U_ATTRIBUTE_TYPE>();
    TestEmberScalarNullRead<OddSizedInteger<3, false>, ZCL_INT24U_ATTRIBUTE_TYPE>();
    TestEmberScalarNullRead<uint32_t, ZCL_INT32U_ATTRIBUTE_TYPE>();
    TestEmberScalarNullRead<OddSizedInteger<5, false>, ZCL_INT40U_ATTRIBUTE_TYPE>();
    TestEmberScalarNullRead<OddSizedInteger<6, false>, ZCL_INT48U_ATTRIBUTE_TYPE>();
    TestEmberScalarNullRead<OddSizedInteger<7, false>, ZCL_INT56U_ATTRIBUTE_TYPE>();
    TestEmberScalarNullRead<uint64_t, ZCL_INT64U_ATTRIBUTE_TYPE>();

    TestEmberScalarNullRead<int8_t, ZCL_INT8S_ATTRIBUTE_TYPE>();
    TestEmberScalarNullRead<int16_t, ZCL_INT16S_ATTRIBUTE_TYPE>();
    TestEmberScalarNullRead<OddSizedInteger<3, true>, ZCL_INT24S_ATTRIBUTE_TYPE>();
    TestEmberScalarNullRead<int32_t, ZCL_INT32S_ATTRIBUTE_TYPE>();
    TestEmberScalarNullRead<OddSizedInteger<5, true>, ZCL_INT40S_ATTRIBUTE_TYPE>();
    TestEmberScalarNullRead<OddSizedInteger<6, true>, ZCL_INT48S_ATTRIBUTE_TYPE>();
    TestEmberScalarNullRead<OddSizedInteger<7, true>, ZCL_INT56S_ATTRIBUTE_TYPE>();
    TestEmberScalarNullRead<int64_t, ZCL_INT64S_ATTRIBUTE_TYPE>();

    TestEmberScalarNullRead<bool, ZCL_BOOLEAN_ATTRIBUTE_TYPE>();

    TestEmberScalarNullRead<float, ZCL_SINGLE_ATTRIBUTE_TYPE>();
    TestEmberScalarNullRead<double, ZCL_DOUBLE_ATTRIBUTE_TYPE>();
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeReadErrorReading)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    {
        ReadOperation testRequest(kMockEndpoint3, MockClusterId(4),
                                  MOCK_ATTRIBUTE_ID_FOR_NULLABLE_TYPE(ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE));
        testRequest.SetSubjectDescriptor(kAdminSubjectDescriptor);

        chip::Test::SetEmberReadOutput(Protocols::InteractionModel::Status::Failure);

        // Actual read via an encoder
        std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding();
        ASSERT_EQ(model.ReadAttribute(testRequest.GetRequest(), *encoder), Status::Failure);
    }

    {
        ReadOperation testRequest(kMockEndpoint3, MockClusterId(4),
                                  MOCK_ATTRIBUTE_ID_FOR_NULLABLE_TYPE(ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE));
        testRequest.SetSubjectDescriptor(kAdminSubjectDescriptor);

        chip::Test::SetEmberReadOutput(Protocols::InteractionModel::Status::Busy);

        // Actual read via an encoder
        std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding();
        ASSERT_EQ(model.ReadAttribute(testRequest.GetRequest(), *encoder), Status::Busy);
    }

    // reset things to success to not affect other tests
    chip::Test::SetEmberReadOutput(ByteSpan());
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeReadNullOctetString)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    ReadOperation testRequest(kMockEndpoint3, MockClusterId(4),
                              MOCK_ATTRIBUTE_ID_FOR_NULLABLE_TYPE(ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE));
    testRequest.SetSubjectDescriptor(kAdminSubjectDescriptor);

    // NOTE: This is a pascal string of size 0xFFFF which for null strings is a null marker
    char data[] = "\xFF\xFFInvalid length string is null";
    chip::Test::SetEmberReadOutput(ByteSpan(reinterpret_cast<const uint8_t *>(data), sizeof(data)));

    // Actual read via an encoder
    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding();
    ASSERT_EQ(model.ReadAttribute(testRequest.GetRequest(), *encoder), CHIP_NO_ERROR);
    ASSERT_EQ(testRequest.FinishEncoding(), CHIP_NO_ERROR);

    // Validate after read
    std::vector<DecodedAttributeData> attribute_data;
    ASSERT_EQ(testRequest.GetEncodedIBs().Decode(attribute_data), CHIP_NO_ERROR);
    ASSERT_EQ(attribute_data.size(), 1u);

    DecodedAttributeData & encodedData = attribute_data[0];
    ASSERT_EQ(encodedData.attributePath, testRequest.GetRequest().path);

    // data element should be null for the given 0xFFFF length
    ASSERT_EQ(encodedData.dataReader.GetType(), TLV::kTLVType_Null);

    chip::app::DataModel::Nullable<ByteSpan> actual;
    ASSERT_EQ(chip::app::DataModel::Decode(encodedData.dataReader, actual), CHIP_NO_ERROR);
    ASSERT_TRUE(actual.IsNull());
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeReadOctetString)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    ReadOperation testRequest(kMockEndpoint3, MockClusterId(4),
                              MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE));
    testRequest.SetSubjectDescriptor(kAdminSubjectDescriptor);

    // NOTE: This is a pascal string, so actual data is "test"
    //       the longer encoding is to make it clear we do not encode the overflow
    char data[] = "\0\0testing here with overflow";
    WriteLe16(data, 4);
    chip::Test::SetEmberReadOutput(ByteSpan(reinterpret_cast<const uint8_t *>(data), sizeof(data)));

    // Actual read via an encoder
    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding();
    ASSERT_EQ(model.ReadAttribute(testRequest.GetRequest(), *encoder), CHIP_NO_ERROR);
    ASSERT_EQ(testRequest.FinishEncoding(), CHIP_NO_ERROR);

    // Validate after read
    std::vector<DecodedAttributeData> attribute_data;
    ASSERT_EQ(testRequest.GetEncodedIBs().Decode(attribute_data), CHIP_NO_ERROR);
    ASSERT_EQ(attribute_data.size(), 1u);

    const DecodedAttributeData & encodedData = attribute_data[0];
    ASSERT_EQ(encodedData.attributePath, testRequest.GetRequest().path);

    // data element should be a encoded byte string as this is what the attribute type is
    ASSERT_EQ(encodedData.dataReader.GetType(), TLV::kTLVType_ByteString);
    ByteSpan actual;
    ASSERT_EQ(encodedData.dataReader.Get(actual), CHIP_NO_ERROR);

    ByteSpan expected(reinterpret_cast<const uint8_t *>(data + 2), 4);
    ASSERT_TRUE(actual.data_equal(expected));
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeReadLongOctetString)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    ReadOperation testRequest(kMockEndpoint3, MockClusterId(4),
                              MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_OCTET_STRING_ATTRIBUTE_TYPE));
    testRequest.SetSubjectDescriptor(kAdminSubjectDescriptor);

    // NOTE: This is a pascal string, so actual data is "test"
    //       the longer encoding is to make it clear we do not encode the overflow
    const char data[] = "\x04testing here with overflow";
    chip::Test::SetEmberReadOutput(ByteSpan(reinterpret_cast<const uint8_t *>(data), sizeof(data)));

    // Actual read via an encoder
    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding();
    ASSERT_EQ(model.ReadAttribute(testRequest.GetRequest(), *encoder), CHIP_NO_ERROR);
    ASSERT_EQ(testRequest.FinishEncoding(), CHIP_NO_ERROR);

    // Validate after read
    std::vector<DecodedAttributeData> attribute_data;
    ASSERT_EQ(testRequest.GetEncodedIBs().Decode(attribute_data), CHIP_NO_ERROR);
    ASSERT_EQ(attribute_data.size(), 1u);

    const DecodedAttributeData & encodedData = attribute_data[0];
    ASSERT_EQ(encodedData.attributePath, testRequest.GetRequest().path);

    // data element should be a encoded byte string as this is what the attribute type is
    ASSERT_EQ(encodedData.dataReader.GetType(), TLV::kTLVType_ByteString);
    ByteSpan actual;
    ASSERT_EQ(encodedData.dataReader.Get(actual), CHIP_NO_ERROR);

    ByteSpan expected(reinterpret_cast<const uint8_t *>(data + 1), 4);
    ASSERT_TRUE(actual.data_equal(expected));
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeReadShortString)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    ReadOperation testRequest(kMockEndpoint3, MockClusterId(4),
                              MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_CHAR_STRING_ATTRIBUTE_TYPE));
    testRequest.SetSubjectDescriptor(kAdminSubjectDescriptor);

    // NOTE: This is a pascal string, so actual data is "abcde"
    //       the longer encoding is to make it clear we do not encode the overflow
    char data[] = "\0abcdef...this is the alphabet";
    *data       = 5;
    chip::Test::SetEmberReadOutput(ByteSpan(reinterpret_cast<const uint8_t *>(data), sizeof(data)));

    // Actual read via an encoder
    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding();
    ASSERT_EQ(model.ReadAttribute(testRequest.GetRequest(), *encoder), CHIP_NO_ERROR);
    ASSERT_EQ(testRequest.FinishEncoding(), CHIP_NO_ERROR);

    // Validate after reading
    std::vector<DecodedAttributeData> attribute_data;
    ASSERT_EQ(testRequest.GetEncodedIBs().Decode(attribute_data), CHIP_NO_ERROR);
    ASSERT_EQ(attribute_data.size(), 1u);

    const DecodedAttributeData & encodedData = attribute_data[0];
    ASSERT_EQ(encodedData.attributePath, testRequest.GetRequest().path);

    // data element should be a encoded byte string as this is what the attribute type is
    ASSERT_EQ(encodedData.dataReader.GetType(), TLV::kTLVType_UTF8String);
    CharSpan actual;
    ASSERT_EQ(encodedData.dataReader.Get(actual), CHIP_NO_ERROR);
    ASSERT_TRUE(actual.data_equal("abcde"_span));
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeReadLongString)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    ReadOperation testRequest(kMockEndpoint3, MockClusterId(4),
                              MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE));
    testRequest.SetSubjectDescriptor(kAdminSubjectDescriptor);

    // NOTE: This is a pascal string, so actual data is "abcde"
    //       the longer encoding is to make it clear we do not encode the overflow
    char data[] = "\0\0abcdef...this is the alphabet";
    WriteLe16(data, 5);
    chip::Test::SetEmberReadOutput(ByteSpan(reinterpret_cast<const uint8_t *>(data), sizeof(data)));

    // Actual read via an encoder
    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding();
    ASSERT_EQ(model.ReadAttribute(testRequest.GetRequest(), *encoder), CHIP_NO_ERROR);
    ASSERT_EQ(testRequest.FinishEncoding(), CHIP_NO_ERROR);

    // Validate after reading
    std::vector<DecodedAttributeData> attribute_data;
    ASSERT_EQ(testRequest.GetEncodedIBs().Decode(attribute_data), CHIP_NO_ERROR);
    ASSERT_EQ(attribute_data.size(), 1u);

    const DecodedAttributeData & encodedData = attribute_data[0];
    ASSERT_EQ(encodedData.attributePath, testRequest.GetRequest().path);

    // data element should be a encoded byte string as this is what the attribute type is
    ASSERT_EQ(encodedData.dataReader.GetType(), TLV::kTLVType_UTF8String);
    CharSpan actual;
    ASSERT_EQ(encodedData.dataReader.Get(actual), CHIP_NO_ERROR);
    ASSERT_TRUE(actual.data_equal("abcde"_span));
}

TEST_F(TestCodegenModelViaMocks, AttributeAccessInterfaceStructRead)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    const ConcreteAttributePath kStructPath(kMockEndpoint3, MockClusterId(4),
                                            MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_STRUCT_ATTRIBUTE_TYPE));

    ReadOperation testRequest(kStructPath);
    testRequest.SetSubjectDescriptor(kAdminSubjectDescriptor);

    RegisteredAttributeAccessInterface<StructAttributeAccessInterface> aai(kStructPath);

    aai->SetReturnedData(Clusters::UnitTesting::Structs::SimpleStruct::Type{
        .a = 123,
        .b = true,
        .e = "foo"_span,
        .g = 0.5,
        .h = 0.125,
    });

    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding();
    ASSERT_EQ(model.ReadAttribute(testRequest.GetRequest(), *encoder), CHIP_NO_ERROR);
    ASSERT_EQ(testRequest.FinishEncoding(), CHIP_NO_ERROR);

    // Validate after read
    std::vector<DecodedAttributeData> attribute_data;
    ASSERT_EQ(testRequest.GetEncodedIBs().Decode(attribute_data), CHIP_NO_ERROR);
    ASSERT_EQ(attribute_data.size(), 1u);

    DecodedAttributeData & encodedData = attribute_data[0];
    ASSERT_EQ(encodedData.attributePath, testRequest.GetRequest().path);

    ASSERT_EQ(encodedData.dataReader.GetType(), TLV::kTLVType_Structure);
    Clusters::UnitTesting::Structs::SimpleStruct::DecodableType actual;
    ASSERT_EQ(chip::app::DataModel::Decode(encodedData.dataReader, actual), CHIP_NO_ERROR);

    ASSERT_EQ(actual.a, 123);
    ASSERT_EQ(actual.b, true);
    ASSERT_EQ(actual.g, 0.5);
    ASSERT_EQ(actual.h, 0.125);
    ASSERT_TRUE(actual.e.data_equal("foo"_span));
}

TEST_F(TestCodegenModelViaMocks, AttributeAccessInterfaceReadError)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    const ConcreteAttributePath kStructPath(kMockEndpoint3, MockClusterId(4),
                                            MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_STRUCT_ATTRIBUTE_TYPE));

    ReadOperation testRequest(kStructPath);
    testRequest.SetSubjectDescriptor(kAdminSubjectDescriptor);

    RegisteredAttributeAccessInterface<ErrorAccessInterface> aai(kStructPath, CHIP_ERROR_KEY_NOT_FOUND);
    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding();
    ASSERT_EQ(model.ReadAttribute(testRequest.GetRequest(), *encoder), CHIP_ERROR_KEY_NOT_FOUND);
}

TEST_F(TestCodegenModelViaMocks, AttributeAccessInterfaceListRead)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    const ConcreteAttributePath kStructPath(kMockEndpoint3, MockClusterId(4),
                                            MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_ARRAY_ATTRIBUTE_TYPE));

    ReadOperation testRequest(kStructPath);
    testRequest.SetSubjectDescriptor(kAdminSubjectDescriptor);

    RegisteredAttributeAccessInterface<ListAttributeAcessInterface> aai(kStructPath);

    constexpr unsigned kDataCount = 5;
    aai->SetReturnedData(Clusters::UnitTesting::Structs::SimpleStruct::Type{
        .b = true,
        .e = "xyz"_span,
        .g = 0.25,
        .h = 0.5,
    });
    aai->SetReturnedDataCount(kDataCount);

    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding();
    ASSERT_EQ(model.ReadAttribute(testRequest.GetRequest(), *encoder), CHIP_NO_ERROR);
    ASSERT_EQ(testRequest.FinishEncoding(), CHIP_NO_ERROR);

    // Validate after read
    std::vector<DecodedAttributeData> attribute_data;
    ASSERT_EQ(testRequest.GetEncodedIBs().Decode(attribute_data), CHIP_NO_ERROR);
    ASSERT_EQ(attribute_data.size(), 1u);

    DecodedAttributeData & encodedData = attribute_data[0];
    ASSERT_EQ(encodedData.attributePath, testRequest.GetRequest().path);

    ASSERT_EQ(encodedData.dataReader.GetType(), TLV::kTLVType_Array);

    std::vector<Clusters::UnitTesting::Structs::SimpleStruct::DecodableType> items;
    ASSERT_EQ(DecodeList(encodedData.dataReader, items), CHIP_NO_ERROR);

    ASSERT_EQ(items.size(), kDataCount);

    for (unsigned i = 0; i < kDataCount; i++)
    {
        Clusters::UnitTesting::Structs::SimpleStruct::DecodableType & actual = items[i];

        ASSERT_EQ(actual.a, static_cast<uint8_t>(i & 0xFF));
        ASSERT_EQ(actual.b, true);
        ASSERT_EQ(actual.g, 0.25);
        ASSERT_EQ(actual.h, 0.5);
        ASSERT_TRUE(actual.e.data_equal("xyz"_span));
    }
}

TEST_F(TestCodegenModelViaMocks, AttributeAccessInterfaceListOverflowRead)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    const ConcreteAttributePath kStructPath(kMockEndpoint3, MockClusterId(4),
                                            MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_ARRAY_ATTRIBUTE_TYPE));

    ReadOperation testRequest(kStructPath);
    testRequest.SetSubjectDescriptor(kAdminSubjectDescriptor);
    RegisteredAttributeAccessInterface<ListAttributeAcessInterface> aai(kStructPath);

    constexpr unsigned kDataCount = 1024;
    aai->SetReturnedData(Clusters::UnitTesting::Structs::SimpleStruct::Type{
        .b = true,
        .e = "thisislongertofillupfaster"_span,
        .g = 0.25,
        .h = 0.5,
    });
    aai->SetReturnedDataCount(kDataCount);

    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding();
    // NOTE: overflow, however data should be valid. Technically both NO_MEMORY and BUFFER_TOO_SMALL
    // should be ok here, however we know buffer-too-small is the error in this case hence
    // the compare (easier to write the test and read the output)
    ASSERT_EQ(model.ReadAttribute(testRequest.GetRequest(), *encoder), CHIP_ERROR_BUFFER_TOO_SMALL);
    ASSERT_EQ(testRequest.FinishEncoding(), CHIP_NO_ERROR);

    // Validate after read
    std::vector<DecodedAttributeData> attribute_data;
    ASSERT_EQ(testRequest.GetEncodedIBs().Decode(attribute_data), CHIP_NO_ERROR);
    ASSERT_EQ(attribute_data.size(), 1u);

    DecodedAttributeData & encodedData = attribute_data[0];
    ASSERT_EQ(encodedData.attributePath, testRequest.GetRequest().path);

    ASSERT_EQ(encodedData.dataReader.GetType(), TLV::kTLVType_Array);

    std::vector<Clusters::UnitTesting::Structs::SimpleStruct::DecodableType> items;
    ASSERT_EQ(DecodeList(encodedData.dataReader, items), CHIP_NO_ERROR);

    // On last check, 16 items can be encoded. Set some non-zero range to be enforced here that
    // SOME list items are actually encoded. Actual lower bound here IS ARBITRARY and was picked
    // to just ensure non-zero item count for checks.
    ASSERT_GT(items.size(), 5u);
    ASSERT_LT(items.size(), kDataCount);

    for (unsigned i = 0; i < items.size(); i++)
    {
        Clusters::UnitTesting::Structs::SimpleStruct::DecodableType & actual = items[i];

        ASSERT_EQ(actual.a, static_cast<uint8_t>(i & 0xFF));
        ASSERT_EQ(actual.b, true);
        ASSERT_EQ(actual.g, 0.25);
        ASSERT_EQ(actual.h, 0.5);
        ASSERT_TRUE(actual.e.data_equal("thisislongertofillupfaster"_span));
    }
}

TEST_F(TestCodegenModelViaMocks, AttributeAccessInterfaceListIncrementalRead)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    const ConcreteAttributePath kStructPath(kMockEndpoint3, MockClusterId(4),
                                            MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_ARRAY_ATTRIBUTE_TYPE));

    ReadOperation testRequest(kStructPath);
    testRequest.SetSubjectDescriptor(kAdminSubjectDescriptor);
    RegisteredAttributeAccessInterface<ListAttributeAcessInterface> aai(kStructPath);

    constexpr unsigned kDataCount        = 1024;
    constexpr unsigned kEncodeIndexStart = 101;
    aai->SetReturnedData(Clusters::UnitTesting::Structs::SimpleStruct::Type{
        .b = true,
        .e = "thisislongertofillupfaster"_span,
        .g = 0.25,
        .h = 0.5,
    });
    aai->SetReturnedDataCount(kDataCount);

    AttributeEncodeState encodeState;
    encodeState.SetCurrentEncodingListIndex(kEncodeIndexStart);

    std::unique_ptr<AttributeValueEncoder> encoder =
        testRequest.StartEncoding(ReadOperation::EncodingParams().SetEncodingState(encodeState));

    // NOTE: overflow, however data should be valid. Technically both NO_MEMORY and BUFFER_TOO_SMALL
    // should be ok here, however we know buffer-too-small is the error in this case hence
    // the compare (easier to write the test and read the output)
    ASSERT_EQ(model.ReadAttribute(testRequest.GetRequest(), *encoder), CHIP_ERROR_BUFFER_TOO_SMALL);
    ASSERT_EQ(testRequest.FinishEncoding(), CHIP_NO_ERROR);

    // Validate after read
    std::vector<DecodedAttributeData> attribute_data;
    ASSERT_EQ(testRequest.GetEncodedIBs().Decode(attribute_data), CHIP_NO_ERROR);

    // Incremental encodes are separate list items, repeated
    // actual size IS ARBITRARY (current test sets it at 11)
    ASSERT_GT(attribute_data.size(), 3u);

    for (unsigned i = 0; i < attribute_data.size(); i++)
    {
        DecodedAttributeData & encodedData = attribute_data[i];
        ASSERT_EQ(encodedData.attributePath.mEndpointId, testRequest.GetRequest().path.mEndpointId);
        ASSERT_EQ(encodedData.attributePath.mClusterId, testRequest.GetRequest().path.mClusterId);
        ASSERT_EQ(encodedData.attributePath.mAttributeId, testRequest.GetRequest().path.mAttributeId);
        ASSERT_EQ(encodedData.attributePath.mListOp, ConcreteDataAttributePath::ListOperation::AppendItem);

        // individual structures encoded in each item
        ASSERT_EQ(encodedData.dataReader.GetType(), TLV::kTLVType_Structure);

        Clusters::UnitTesting::Structs::SimpleStruct::DecodableType actual;
        ASSERT_EQ(chip::app::DataModel::Decode(encodedData.dataReader, actual), CHIP_NO_ERROR);

        ASSERT_EQ(actual.a, static_cast<uint8_t>((i + kEncodeIndexStart) & 0xFF));
        ASSERT_EQ(actual.b, true);
        ASSERT_EQ(actual.g, 0.25);
        ASSERT_EQ(actual.h, 0.5);
        ASSERT_TRUE(actual.e.data_equal("thisislongertofillupfaster"_span));
    }
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeWriteBasicTypes)
{
    TestEmberScalarTypeWrite<uint8_t, ZCL_INT8U_ATTRIBUTE_TYPE>(0x12);
    TestEmberScalarTypeWrite<uint16_t, ZCL_ENUM16_ATTRIBUTE_TYPE>(0x1234);
    TestEmberScalarTypeWrite<OddSizedInteger<3, false>, ZCL_INT24U_ATTRIBUTE_TYPE>(0x112233);
    TestEmberScalarTypeWrite<uint32_t, ZCL_INT32U_ATTRIBUTE_TYPE>(0x11223344);
    TestEmberScalarTypeWrite<OddSizedInteger<5, false>, ZCL_INT40U_ATTRIBUTE_TYPE>(0x1122334455ULL);
    TestEmberScalarTypeWrite<OddSizedInteger<6, false>, ZCL_INT48U_ATTRIBUTE_TYPE>(0x112233445566ULL);
    TestEmberScalarTypeWrite<OddSizedInteger<7, false>, ZCL_INT56U_ATTRIBUTE_TYPE>(0x11223344556677ULL);
    TestEmberScalarTypeWrite<uint64_t, ZCL_INT64U_ATTRIBUTE_TYPE>(0x1122334455667788ULL);

    TestEmberScalarTypeWrite<int8_t, ZCL_INT8S_ATTRIBUTE_TYPE>(-10);
    TestEmberScalarTypeWrite<int16_t, ZCL_INT16S_ATTRIBUTE_TYPE>(-123);
    TestEmberScalarTypeWrite<OddSizedInteger<3, true>, ZCL_INT24S_ATTRIBUTE_TYPE>(-1234);
    TestEmberScalarTypeWrite<int32_t, ZCL_INT32S_ATTRIBUTE_TYPE>(-12345);
    TestEmberScalarTypeWrite<OddSizedInteger<5, true>, ZCL_INT40S_ATTRIBUTE_TYPE>(-123456);
    TestEmberScalarTypeWrite<OddSizedInteger<6, true>, ZCL_INT48S_ATTRIBUTE_TYPE>(-1234567);
    TestEmberScalarTypeWrite<OddSizedInteger<7, true>, ZCL_INT56S_ATTRIBUTE_TYPE>(-12345678);
    TestEmberScalarTypeWrite<int64_t, ZCL_INT64S_ATTRIBUTE_TYPE>(-123456789);

    TestEmberScalarTypeWrite<bool, ZCL_BOOLEAN_ATTRIBUTE_TYPE>(true);
    TestEmberScalarTypeWrite<bool, ZCL_BOOLEAN_ATTRIBUTE_TYPE>(false);
    TestEmberScalarTypeWrite<float, ZCL_SINGLE_ATTRIBUTE_TYPE>(0.625);
    TestEmberScalarTypeWrite<double, ZCL_DOUBLE_ATTRIBUTE_TYPE>(0.625);
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeWriteInvalidValueToNullable)
{
    TestEmberScalarTypeWriteNullValueToNullable<uint8_t, ZCL_INT8U_ATTRIBUTE_TYPE>();
    TestEmberScalarTypeWriteNullValueToNullable<uint16_t, ZCL_ENUM16_ATTRIBUTE_TYPE>();
    TestEmberScalarTypeWriteNullValueToNullable<OddSizedInteger<3, false>, ZCL_INT24U_ATTRIBUTE_TYPE>();
    TestEmberScalarTypeWriteNullValueToNullable<uint32_t, ZCL_INT32U_ATTRIBUTE_TYPE>();
    TestEmberScalarTypeWriteNullValueToNullable<OddSizedInteger<5, false>, ZCL_INT40U_ATTRIBUTE_TYPE>();
    TestEmberScalarTypeWriteNullValueToNullable<OddSizedInteger<6, false>, ZCL_INT48U_ATTRIBUTE_TYPE>();
    TestEmberScalarTypeWriteNullValueToNullable<OddSizedInteger<7, false>, ZCL_INT56U_ATTRIBUTE_TYPE>();
    TestEmberScalarTypeWriteNullValueToNullable<uint64_t, ZCL_INT64U_ATTRIBUTE_TYPE>();

    TestEmberScalarTypeWriteNullValueToNullable<int8_t, ZCL_INT8S_ATTRIBUTE_TYPE>();
    TestEmberScalarTypeWriteNullValueToNullable<int16_t, ZCL_INT16S_ATTRIBUTE_TYPE>();
    TestEmberScalarTypeWriteNullValueToNullable<OddSizedInteger<3, true>, ZCL_INT24S_ATTRIBUTE_TYPE>();
    TestEmberScalarTypeWriteNullValueToNullable<int32_t, ZCL_INT32S_ATTRIBUTE_TYPE>();
    TestEmberScalarTypeWriteNullValueToNullable<OddSizedInteger<5, true>, ZCL_INT40S_ATTRIBUTE_TYPE>();
    TestEmberScalarTypeWriteNullValueToNullable<OddSizedInteger<6, true>, ZCL_INT48S_ATTRIBUTE_TYPE>();
    TestEmberScalarTypeWriteNullValueToNullable<OddSizedInteger<7, true>, ZCL_INT56S_ATTRIBUTE_TYPE>();
    TestEmberScalarTypeWriteNullValueToNullable<int64_t, ZCL_INT64S_ATTRIBUTE_TYPE>();

    TestEmberScalarTypeWriteNullValueToNullable<bool, ZCL_BOOLEAN_ATTRIBUTE_TYPE>();
    TestEmberScalarTypeWriteNullValueToNullable<bool, ZCL_BOOLEAN_ATTRIBUTE_TYPE>();
    TestEmberScalarTypeWriteNullValueToNullable<float, ZCL_SINGLE_ATTRIBUTE_TYPE>();
    TestEmberScalarTypeWriteNullValueToNullable<double, ZCL_DOUBLE_ATTRIBUTE_TYPE>();
}

TEST_F(TestCodegenModelViaMocks, EmberTestWriteReservedNullPlaceholderToNullable)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    WriteOperation test(kMockEndpoint3, MockClusterId(4), MOCK_ATTRIBUTE_ID_FOR_NULLABLE_TYPE(ZCL_INT32U_ATTRIBUTE_TYPE));
    test.SetSubjectDescriptor(kAdminSubjectDescriptor);

    using NumericType             = NumericAttributeTraits<uint32_t>;
    using NullableType            = chip::app::DataModel::Nullable<typename NumericType::WorkingType>;
    AttributeValueDecoder decoder = test.DecoderFor<NullableType>(0xFFFFFFFF);

    // write should fail: we are trying to write null which is out of range
    ASSERT_EQ(model.WriteAttribute(test.GetRequest(), decoder), Status::ConstraintError);
}

TEST_F(TestCodegenModelViaMocks, EmberTestWriteOutOfRepresentableRangeOddIntegerNonNullable)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    WriteOperation test(kMockEndpoint3, MockClusterId(4), MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_INT24U_ATTRIBUTE_TYPE));
    test.SetSubjectDescriptor(kAdminSubjectDescriptor);

    using NumericType             = NumericAttributeTraits<uint32_t>;
    using NullableType            = chip::app::DataModel::Nullable<typename NumericType::WorkingType>;
    AttributeValueDecoder decoder = test.DecoderFor<NullableType>(0x1223344);

    ASSERT_EQ(model.WriteAttribute(test.GetRequest(), decoder), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

TEST_F(TestCodegenModelViaMocks, EmberTestWriteOutOfRepresentableRangeOddIntegerNullable)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    WriteOperation test(kMockEndpoint3, MockClusterId(4), MOCK_ATTRIBUTE_ID_FOR_NULLABLE_TYPE(ZCL_INT24U_ATTRIBUTE_TYPE));
    test.SetSubjectDescriptor(kAdminSubjectDescriptor);

    using NumericType             = NumericAttributeTraits<uint32_t>;
    using NullableType            = chip::app::DataModel::Nullable<typename NumericType::WorkingType>;
    AttributeValueDecoder decoder = test.DecoderFor<NullableType>(0x1223344);

    ASSERT_EQ(model.WriteAttribute(test.GetRequest(), decoder), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeWriteBasicTypesLowestValue)
{
    TestEmberScalarTypeWrite<int8_t, ZCL_INT8S_ATTRIBUTE_TYPE>(-127);
    TestEmberScalarTypeWrite<int16_t, ZCL_INT16S_ATTRIBUTE_TYPE>(-32767);
    TestEmberScalarTypeWrite<OddSizedInteger<3, true>, ZCL_INT24S_ATTRIBUTE_TYPE>(-8388607);
    TestEmberScalarTypeWrite<int32_t, ZCL_INT32S_ATTRIBUTE_TYPE>(-2147483647);
    TestEmberScalarTypeWrite<OddSizedInteger<5, true>, ZCL_INT40S_ATTRIBUTE_TYPE>(-549755813887);
    TestEmberScalarTypeWrite<OddSizedInteger<6, true>, ZCL_INT48S_ATTRIBUTE_TYPE>(-140737488355327);
    TestEmberScalarTypeWrite<OddSizedInteger<7, true>, ZCL_INT56S_ATTRIBUTE_TYPE>(-36028797018963967);
    TestEmberScalarTypeWrite<int64_t, ZCL_INT64S_ATTRIBUTE_TYPE>(-9223372036854775807);
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeWriteNulls)
{
    TestEmberScalarNullWrite<uint8_t, ZCL_INT8U_ATTRIBUTE_TYPE>();
    TestEmberScalarNullWrite<uint16_t, ZCL_ENUM16_ATTRIBUTE_TYPE>();
    TestEmberScalarNullWrite<OddSizedInteger<3, false>, ZCL_INT24U_ATTRIBUTE_TYPE>();
    TestEmberScalarNullWrite<uint32_t, ZCL_INT32U_ATTRIBUTE_TYPE>();
    TestEmberScalarNullWrite<OddSizedInteger<5, false>, ZCL_INT40U_ATTRIBUTE_TYPE>();
    TestEmberScalarNullWrite<OddSizedInteger<6, false>, ZCL_INT48U_ATTRIBUTE_TYPE>();
    TestEmberScalarNullWrite<OddSizedInteger<7, false>, ZCL_INT56U_ATTRIBUTE_TYPE>();
    TestEmberScalarNullWrite<uint64_t, ZCL_INT64U_ATTRIBUTE_TYPE>();

    TestEmberScalarNullWrite<int8_t, ZCL_INT8S_ATTRIBUTE_TYPE>();
    TestEmberScalarNullWrite<int16_t, ZCL_INT16S_ATTRIBUTE_TYPE>();
    TestEmberScalarNullWrite<OddSizedInteger<3, true>, ZCL_INT24S_ATTRIBUTE_TYPE>();
    TestEmberScalarNullWrite<int32_t, ZCL_INT32S_ATTRIBUTE_TYPE>();
    TestEmberScalarNullWrite<OddSizedInteger<5, true>, ZCL_INT40S_ATTRIBUTE_TYPE>();
    TestEmberScalarNullWrite<OddSizedInteger<6, true>, ZCL_INT48S_ATTRIBUTE_TYPE>();
    TestEmberScalarNullWrite<OddSizedInteger<7, true>, ZCL_INT56S_ATTRIBUTE_TYPE>();
    TestEmberScalarNullWrite<int64_t, ZCL_INT64S_ATTRIBUTE_TYPE>();
    TestEmberScalarNullWrite<bool, ZCL_BOOLEAN_ATTRIBUTE_TYPE>();
    TestEmberScalarNullWrite<float, ZCL_SINGLE_ATTRIBUTE_TYPE>();
    TestEmberScalarNullWrite<double, ZCL_DOUBLE_ATTRIBUTE_TYPE>();
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeWriteShortString)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    WriteOperation test(kMockEndpoint3, MockClusterId(4), MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_CHAR_STRING_ATTRIBUTE_TYPE));
    test.SetSubjectDescriptor(kAdminSubjectDescriptor);

    AttributeValueDecoder decoder = test.DecoderFor<CharSpan>("hello world"_span);

    ASSERT_EQ(model.WriteAttribute(test.GetRequest(), decoder), CHIP_NO_ERROR);
    chip::ByteSpan writtenData = GetEmberBuffer();
    chip::CharSpan asCharSpan(reinterpret_cast<const char *>(writtenData.data()), writtenData[0] + 1);
    ASSERT_TRUE(asCharSpan.data_equal("\x0Bhello world"_span));
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeWriteLongStringOutOfBounds)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    WriteOperation test(kMockEndpoint3, MockClusterId(4),
                        MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE));
    test.SetSubjectDescriptor(kAdminSubjectDescriptor);

    // Mocks allow for 16 bytes only by default for string attributes
    AttributeValueDecoder decoder = test.DecoderFor<CharSpan>(
        "this is a very long string that will be longer than the default attribute size for our mocks"_span);

    ASSERT_EQ(model.WriteAttribute(test.GetRequest(), decoder), Status::InvalidValue);
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeWriteLongString)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    WriteOperation test(kMockEndpoint3, MockClusterId(4),
                        MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE));
    test.SetSubjectDescriptor(kAdminSubjectDescriptor);

    AttributeValueDecoder decoder = test.DecoderFor<CharSpan>("text"_span);

    ASSERT_EQ(model.WriteAttribute(test.GetRequest(), decoder), CHIP_NO_ERROR);
    chip::ByteSpan writtenData = GetEmberBuffer();

    uint16_t len = ReadLe16(writtenData.data());
    EXPECT_EQ(len, 4);
    chip::CharSpan asCharSpan(reinterpret_cast<const char *>(writtenData.data() + 2), 4);

    ASSERT_TRUE(asCharSpan.data_equal("text"_span));
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeWriteNullableLongStringValue)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    WriteOperation test(kMockEndpoint3, MockClusterId(4), MOCK_ATTRIBUTE_ID_FOR_NULLABLE_TYPE(ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE));
    test.SetSubjectDescriptor(kAdminSubjectDescriptor);

    AttributeValueDecoder decoder =
        test.DecoderFor<chip::app::DataModel::Nullable<CharSpan>>(chip::app::DataModel::MakeNullable("text"_span));

    ASSERT_EQ(model.WriteAttribute(test.GetRequest(), decoder), CHIP_NO_ERROR);
    chip::ByteSpan writtenData = GetEmberBuffer();

    uint16_t len = ReadLe16(writtenData.data());
    EXPECT_EQ(len, 4);
    chip::CharSpan asCharSpan(reinterpret_cast<const char *>(writtenData.data() + 2), 4);

    ASSERT_TRUE(asCharSpan.data_equal("text"_span));
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeWriteLongNullableStringNull)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    WriteOperation test(kMockEndpoint3, MockClusterId(4), MOCK_ATTRIBUTE_ID_FOR_NULLABLE_TYPE(ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE));
    test.SetSubjectDescriptor(kAdminSubjectDescriptor);

    AttributeValueDecoder decoder =
        test.DecoderFor<chip::app::DataModel::Nullable<CharSpan>>(chip::app::DataModel::Nullable<CharSpan>());

    ASSERT_EQ(model.WriteAttribute(test.GetRequest(), decoder), CHIP_NO_ERROR);
    chip::ByteSpan writtenData = GetEmberBuffer();
    ASSERT_EQ(writtenData[0], 0xFF);
    ASSERT_EQ(writtenData[1], 0xFF);
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeWriteShortBytes)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    WriteOperation test(kMockEndpoint3, MockClusterId(4), MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_OCTET_STRING_ATTRIBUTE_TYPE));
    test.SetSubjectDescriptor(kAdminSubjectDescriptor);

    uint8_t buffer[] = { 11, 12, 13 };

    AttributeValueDecoder decoder = test.DecoderFor<ByteSpan>(ByteSpan(buffer));

    ASSERT_EQ(model.WriteAttribute(test.GetRequest(), decoder), CHIP_NO_ERROR);
    chip::ByteSpan writtenData = GetEmberBuffer();

    EXPECT_EQ(writtenData[0], 3u);
    EXPECT_EQ(writtenData[1], 11u);
    EXPECT_EQ(writtenData[2], 12u);
    EXPECT_EQ(writtenData[3], 13u);
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeWriteLongBytes)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    WriteOperation test(kMockEndpoint3, MockClusterId(4),
                        MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE));
    test.SetSubjectDescriptor(kAdminSubjectDescriptor);

    uint8_t buffer[] = { 11, 12, 13 };

    AttributeValueDecoder decoder = test.DecoderFor<ByteSpan>(ByteSpan(buffer));

    ASSERT_EQ(model.WriteAttribute(test.GetRequest(), decoder), CHIP_NO_ERROR);
    chip::ByteSpan writtenData = GetEmberBuffer();

    uint16_t len = ReadLe16(writtenData.data());
    EXPECT_EQ(len, 3);

    EXPECT_EQ(writtenData[2], 11u);
    EXPECT_EQ(writtenData[3], 12u);
    EXPECT_EQ(writtenData[4], 13u);
}

TEST_F(TestCodegenModelViaMocks, EmberAttributeWriteDataVersion)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    WriteOperation test(kMockEndpoint3, MockClusterId(4), MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_INT32S_ATTRIBUTE_TYPE));
    test.SetSubjectDescriptor(kAdminSubjectDescriptor);

    // Initialize to some version
    ResetVersion();
    BumpVersion();
    test.SetDataVersion(MakeOptional(GetVersion()));

    // Make version invalid
    BumpVersion();

    AttributeValueDecoder decoder = test.DecoderFor<int32_t>(1234);

    ASSERT_EQ(model.WriteAttribute(test.GetRequest(), decoder), Status::DataVersionMismatch);

    // Write passes if we set the right version for the data
    test.SetDataVersion(MakeOptional(GetVersion()));
    ASSERT_EQ(model.WriteAttribute(test.GetRequest(), decoder), CHIP_NO_ERROR);
}

TEST_F(TestCodegenModelViaMocks, WriteToInvalidPath)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    {
        WriteOperation test(kInvalidEndpointId, MockClusterId(1234), 1234);
        test.SetSubjectDescriptor(kAdminSubjectDescriptor);

        AttributeValueDecoder decoder = test.DecoderFor<int32_t>(1234);
        ASSERT_EQ(model.WriteAttribute(test.GetRequest(), decoder), Status::UnsupportedEndpoint);
    }
    {
        WriteOperation test(kMockEndpoint1, MockClusterId(1234), 1234);
        test.SetSubjectDescriptor(kAdminSubjectDescriptor);

        AttributeValueDecoder decoder = test.DecoderFor<int32_t>(1234);
        ASSERT_EQ(model.WriteAttribute(test.GetRequest(), decoder), Status::UnsupportedCluster);
    }

    {
        WriteOperation test(kMockEndpoint1, MockClusterId(1), 1234);
        test.SetSubjectDescriptor(kAdminSubjectDescriptor);

        AttributeValueDecoder decoder = test.DecoderFor<int32_t>(1234);
        ASSERT_EQ(model.WriteAttribute(test.GetRequest(), decoder), Status::UnsupportedAttribute);
    }
}

TEST_F(TestCodegenModelViaMocks, WriteToGlobalAttribute)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    WriteOperation test(kMockEndpoint1, MockClusterId(1), AttributeList::Id);
    test.SetSubjectDescriptor(kAdminSubjectDescriptor);

    AttributeValueDecoder decoder = test.DecoderFor<int32_t>(1234);
    ASSERT_EQ(model.WriteAttribute(test.GetRequest(), decoder), Status::UnsupportedWrite);
}

TEST_F(TestCodegenModelViaMocks, EmberWriteFailure)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    WriteOperation test(kMockEndpoint3, MockClusterId(4), MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_INT32S_ATTRIBUTE_TYPE));
    test.SetSubjectDescriptor(kAdminSubjectDescriptor);

    {
        AttributeValueDecoder decoder = test.DecoderFor<int32_t>(1234);
        chip::Test::SetEmberReadOutput(Protocols::InteractionModel::Status::Failure);
        ASSERT_EQ(model.WriteAttribute(test.GetRequest(), decoder), Status::Failure);
    }
    {
        AttributeValueDecoder decoder = test.DecoderFor<int32_t>(1234);
        chip::Test::SetEmberReadOutput(Protocols::InteractionModel::Status::Busy);
        ASSERT_EQ(model.WriteAttribute(test.GetRequest(), decoder), Status::Busy);
    }
    // reset things to success to not affect other tests
    chip::Test::SetEmberReadOutput(ByteSpan());
}

TEST_F(TestCodegenModelViaMocks, EmberWriteAttributeAccessInterfaceTest)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    const ConcreteAttributePath kStructPath(kMockEndpoint3, MockClusterId(4),
                                            MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_STRUCT_ATTRIBUTE_TYPE));
    RegisteredAttributeAccessInterface<StructAttributeAccessInterface> aai(kStructPath);

    WriteOperation test(kStructPath);
    test.SetSubjectDescriptor(kAdminSubjectDescriptor);

    Clusters::UnitTesting::Structs::SimpleStruct::Type testValue{
        .a = 112,
        .b = true,
        .e = "aai_write_test"_span,
        .g = 0.5,
        .h = 0.125,
    };

    AttributeValueDecoder decoder = test.DecoderFor(testValue);
    ASSERT_EQ(model.WriteAttribute(test.GetRequest(), decoder), CHIP_NO_ERROR);

    EXPECT_EQ(aai->GetData().a, 112);
    EXPECT_TRUE(aai->GetData().e.data_equal("aai_write_test"_span));

    // AAI marks dirty paths
    ASSERT_EQ(model.ChangeListener().DirtyList().size(), 1u);
    EXPECT_EQ(model.ChangeListener().DirtyList()[0],
              AttributePathParams(kStructPath.mEndpointId, kStructPath.mClusterId, kStructPath.mAttributeId));

    // AAI does not prevent read/write of regular attributes
    // validate that once AAI is added, we still can go through writing regular bits (i.e.
    // AAI returning "unknown" has fallback to ember)
    TestEmberScalarTypeWrite<uint32_t, ZCL_INT32U_ATTRIBUTE_TYPE>(4321);
    TestEmberScalarNullWrite<int64_t, ZCL_INT64S_ATTRIBUTE_TYPE>();
}

TEST_F(TestCodegenModelViaMocks, EmberInvokeTest)
{
    // Ember invoke is fully code-generated - there is a single function for Dispatch
    // that will do a `switch` on the path elements and invoke a corresponding `emberAf*`
    // callback.
    //
    // The only thing that can be validated is that this `DispatchSingleClusterCommand`
    // is actually invoked.

    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModelProvider model;

    {
        const ConcreteCommandPath kCommandPath(kMockEndpoint1, MockClusterId(1), kMockCommandId1);
        const InvokeRequest kInvokeRequest{ .path = kCommandPath };
        chip::TLV::TLVReader tlvReader;

        const uint32_t kDispatchCountPre = chip::Test::DispatchCount();

        // Using a handler set to nullptr as it is not used by the impl
        ASSERT_EQ(model.Invoke(kInvokeRequest, tlvReader, /* handler = */ nullptr), std::nullopt);

        EXPECT_EQ(chip::Test::DispatchCount(), kDispatchCountPre + 1); // single dispatch
        EXPECT_EQ(chip::Test::GetLastDispatchPath(), kCommandPath);    // for the right path
    }

    {
        const ConcreteCommandPath kCommandPath(kMockEndpoint1, MockClusterId(1), kMockCommandId2);
        const InvokeRequest kInvokeRequest{ .path = kCommandPath };
        chip::TLV::TLVReader tlvReader;

        const uint32_t kDispatchCountPre = chip::Test::DispatchCount();

        // Using a handler set to nullpotr as it is not used by the impl
        ASSERT_EQ(model.Invoke(kInvokeRequest, tlvReader, /* handler = */ nullptr), std::nullopt);

        EXPECT_EQ(chip::Test::DispatchCount(), kDispatchCountPre + 1); // single dispatch
        EXPECT_EQ(chip::Test::GetLastDispatchPath(), kCommandPath);    // for the right path
    }
}

TEST_F(TestCodegenModelViaMocks, EmberWriteAttributeAccessInterfaceReturningError)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    const ConcreteAttributePath kStructPath(kMockEndpoint3, MockClusterId(4),
                                            MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_STRUCT_ATTRIBUTE_TYPE));
    RegisteredAttributeAccessInterface<ErrorAccessInterface> aai(kStructPath, CHIP_ERROR_KEY_NOT_FOUND);

    WriteOperation test(kStructPath);
    test.SetSubjectDescriptor(kAdminSubjectDescriptor);

    Clusters::UnitTesting::Structs::SimpleStruct::Type testValue{
        .a = 112,
        .b = true,
        .e = "aai_write_test"_span,
        .g = 0.5,
        .h = 0.125,
    };

    AttributeValueDecoder decoder = test.DecoderFor(testValue);
    ASSERT_EQ(model.WriteAttribute(test.GetRequest(), decoder), CHIP_ERROR_KEY_NOT_FOUND);
    ASSERT_TRUE(model.ChangeListener().DirtyList().empty());
}

TEST_F(TestCodegenModelViaMocks, EmberWriteInvalidDataType)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;
    ScopedMockAccessControl accessControl;

    const ConcreteAttributePath kStructPath(kMockEndpoint3, MockClusterId(4),
                                            MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_STRUCT_ATTRIBUTE_TYPE));

    WriteOperation test(kStructPath);
    test.SetSubjectDescriptor(kAdminSubjectDescriptor);

    Clusters::UnitTesting::Structs::SimpleStruct::Type testValue{
        .a = 112,
        .b = true,
        .e = "aai_write_test"_span,
        .g = 0.5,
        .h = 0.125,
    };

    AttributeValueDecoder decoder = test.DecoderFor(testValue);

    // Embed specifically DOES NOT support structures.
    // Without AAI, we expect a data type error (translated to failure)
    ASSERT_EQ(model.WriteAttribute(test.GetRequest(), decoder), Status::Failure);
    ASSERT_TRUE(model.ChangeListener().DirtyList().empty());
}

TEST_F(TestCodegenModelViaMocks, DeviceTypeIteration)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;

    // Mock endpoint 1 has 3 device types
    DataModel::ListBuilder<DataModel::DeviceTypeEntry> builder;
    ASSERT_EQ(model.DeviceTypes(kMockEndpoint1, builder), CHIP_NO_ERROR);
    auto deviceTypes = builder.TakeBuffer();
    ASSERT_EQ(deviceTypes.size(), 3u);

    const DeviceTypeEntry expected1[] = {
        { .deviceTypeId = kDeviceTypeId1, .deviceTypeRevision = kDeviceTypeId1Version },
        { .deviceTypeId = kDeviceTypeId2, .deviceTypeRevision = kDeviceTypeId2Version },
        { .deviceTypeId = kDeviceTypeId3, .deviceTypeRevision = kDeviceTypeId3Version },
    };
    for (unsigned i = 0; i < 3; i++)
    {
        ASSERT_EQ(deviceTypes[i], expected1[i]);
    }

    // Mock endpoint 2 has 1 device types
    ASSERT_TRUE(builder.IsEmpty()); // ownership taken above, we start fresh
    ASSERT_EQ(model.DeviceTypes(kMockEndpoint2, builder), CHIP_NO_ERROR);
    deviceTypes = builder.TakeBuffer();
    ASSERT_EQ(deviceTypes.size(), 1u);
    const DeviceTypeEntry expected2 = { .deviceTypeId = kDeviceTypeId2, .deviceTypeRevision = kDeviceTypeId2Version };
    ASSERT_EQ(deviceTypes[0], expected2);

    // empty endpoint works
    ASSERT_TRUE(builder.IsEmpty()); // ownership taken above, we start fresh
    ASSERT_EQ(model.DeviceTypes(kMockEndpoint3, builder), CHIP_NO_ERROR);
    ASSERT_TRUE(builder.IsEmpty());
    ASSERT_TRUE(builder.TakeBuffer().empty());
}

TEST_F(TestCodegenModelViaMocks, SemanticTagIteration)
{
    UseMockNodeConfig config(gTestNodeConfig);
    CodegenDataModelProviderWithContext model;

    DataModel::ListBuilder<Provider::SemanticTag> builder;
    ASSERT_EQ(model.SemanticTags(kMockEndpoint2, builder), CHIP_NO_ERROR);
    ASSERT_TRUE(builder.IsEmpty());
    auto tags = builder.TakeBuffer();
    ASSERT_TRUE(tags.empty());

    // Mock endpoint 1 has 3 semantic tags
    ASSERT_EQ(model.SemanticTags(kMockEndpoint1, builder), CHIP_NO_ERROR);
    ASSERT_EQ(builder.Size(), 3u);
    tags = builder.TakeBuffer();
    ASSERT_EQ(tags.size(), 3u);
    ASSERT_TRUE(builder.IsEmpty()); // ownership taken

    auto tag = tags[0];
    EXPECT_EQ(tag.mfgCode, MakeNullable(VendorId::TestVendor1));
    EXPECT_EQ(tag.namespaceID, kNamespaceID1);
    EXPECT_EQ(tag.tag, kTag1);
    ASSERT_TRUE(tag.label.HasValue() && (!tag.label.Value().IsNull()));
    EXPECT_TRUE(tag.label.Value().Value().data_equal(CharSpan::fromCharString(kLabel1)));

    tag = tags[1];
    EXPECT_TRUE(tag.mfgCode.IsNull());
    EXPECT_EQ(tag.namespaceID, kNamespaceID2);
    EXPECT_EQ(tag.tag, kTag2);
    ASSERT_TRUE(tag.label.HasValue() && (!tag.label.Value().IsNull()));
    EXPECT_TRUE(tag.label.Value().Value().data_equal(CharSpan::fromCharString(kLabel2)));

    tag = tags[2];
    EXPECT_EQ(tag.mfgCode, MakeNullable(VendorId::TestVendor3));
    EXPECT_EQ(tag.namespaceID, kNamespaceID3);
    EXPECT_EQ(tag.tag, kTag3);
    EXPECT_FALSE(tag.label.HasValue());
}
