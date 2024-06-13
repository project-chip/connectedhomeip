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
#include <app/codegen-data-model/CodegenDataModel.h>

#include <app/codegen-data-model/tests/EmberReadWriteOverride.h>
#include <app/codegen-data-model/tests/TestAttributeReportIBsEncoding.h>

#include <access/AccessControl.h>
#include <access/SubjectDescriptor.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/AttributeEncodeState.h>
#include <app/AttributeValueDecoder.h>
#include <app/ConcreteAttributePath.h>
#include <app/GlobalAttributes.h>
#include <app/MessageDef/ReportDataMessage.h>
#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>
#include <app/util/attribute-storage-null-handling.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <app/util/mock/MockNodeConfig.h>
#include <app/util/odd-sized-integers.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVReader.h>
#include <lib/core/TLVWriter.h>

#include <gtest/gtest.h>
#include <vector>

using namespace chip;
using namespace chip::Test;
using namespace chip::app;
using namespace chip::app::InteractionModel;
using namespace chip::app::Clusters::Globals::Attributes;

namespace {

constexpr FabricIndex kTestFabrixIndex = kMinValidFabricIndex;
constexpr NodeId kTestNodeId           = 0xFFFF'1234'ABCD'4321;

constexpr EndpointId kEndpointIdThatIsMissing = kMockEndpointMin - 1;

static_assert(kEndpointIdThatIsMissing != kInvalidEndpointId);
static_assert(kEndpointIdThatIsMissing != kMockEndpoint1);
static_assert(kEndpointIdThatIsMissing != kMockEndpoint2);
static_assert(kEndpointIdThatIsMissing != kMockEndpoint3);

constexpr Access::SubjectDescriptor kAdminSubjectDescriptor{
    .fabricIndex = kTestFabrixIndex,
    .authMode    = Access::AuthMode::kCase,
    .subject     = kTestNodeId,
};
constexpr Access::SubjectDescriptor kViewSubjectDescriptor{
    .fabricIndex = kTestFabrixIndex + 1,
    .authMode    = Access::AuthMode::kCase,
    .subject     = kTestNodeId,
};

constexpr Access::SubjectDescriptor kDenySubjectDescriptor{
    .fabricIndex = kTestFabrixIndex + 2,
    .authMode    = Access::AuthMode::kCase,
    .subject     = kTestNodeId,
};

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
            {4, 6}   /* generatedCommands */
        ),
    }),
    MockEndpointConfig(kMockEndpoint3, {
        MockClusterConfig(MockClusterId(1), {
            ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1),
        }),
        MockClusterConfig(MockClusterId(2), {
            ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1), MockAttributeId(2), MockAttributeId(3), MockAttributeId(4),
        }),
        MockClusterConfig(MockClusterId(3), {
            ClusterRevision::Id, FeatureMap::Id,
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

    void SetReturnedData(const Clusters::UnitTesting::Structs::SimpleStruct::Type & data) { mData = data; }
    Clusters::UnitTesting::Structs::SimpleStruct::Type simpleStruct;

private:
    ConcreteAttributePath mPath;
    Clusters::UnitTesting::Structs::SimpleStruct::Type mData;
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
    Clusters::UnitTesting::Structs::SimpleStruct::Type simpleStruct;

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
        VerifyOrDie(registerAttributeAccessOverride(&mData));
    }
    ~RegisteredAttributeAccessInterface() { unregisterAttributeAccessOverride(&mData); }

    T * operator->() { return &mData; }
    T & operator*() { return mData; }

private:
    T mData;
};

/// Contains a `ReadAttributeRequest` as well as classes to convert this into a AttributeReportIBs
/// and later decode it
///
/// It wraps boilerplate code to obtain a `AttributeValueEncoder` as well as later decoding
/// the underlying encoded data for verification.
struct TestReadRequest
{
    ReadAttributeRequest request;

    // encoded-used classes
    EncodedReportIBs encodedIBs;
    AttributeReportIBs::Builder reportBuilder;
    std::unique_ptr<AttributeValueEncoder> encoder;

    TestReadRequest(const Access::SubjectDescriptor & subject, const ConcreteAttributePath & path)
    {
        // operationFlags is 0 i.e. not internal
        // readFlags is 0 i.e. not fabric filtered
        // dataVersion is missing (no data version filtering)
        request.subjectDescriptor = subject;
        request.path              = path;
    }

    std::unique_ptr<AttributeValueEncoder> StartEncoding(chip::app::InteractionModel::DataModel * model,
                                                         AttributeEncodeState state = AttributeEncodeState())
    {
        std::optional<ClusterInfo> info = model->GetClusterInfo(request.path);
        if (!info.has_value())
        {
            ChipLogError(Test, "Missing cluster information - no data version");
            return nullptr;
        }

        DataVersion dataVersion = info->dataVersion; // NOLINT(bugprone-unchecked-optional-access)

        CHIP_ERROR err = encodedIBs.StartEncoding(reportBuilder);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Test, "FAILURE starting encoding %" CHIP_ERROR_FORMAT, err.Format());
            return nullptr;
        }

        // TODO: could we test isFabricFiltered and EncodeState?
    
        // request.subjectDescriptor is known non-null because it is set in the constructor
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        return std::make_unique<AttributeValueEncoder>(reportBuilder, *request.subjectDescriptor, request.path, dataVersion,
                                                       false /* aIsFabricFiltered */, state);
    }

    CHIP_ERROR FinishEncoding() { return encodedIBs.FinishEncoding(reportBuilder); }
};

template <typename T, EmberAfAttributeType ZclType>
void TestEmberScalarTypeRead(typename NumericAttributeTraits<T>::WorkingType value)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;
    ScopedMockAccessControl accessControl;

    TestReadRequest testRequest(
        kAdminSubjectDescriptor,
        ConcreteAttributePath(kMockEndpoint3, MockClusterId(4), MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZclType)));

    // Ember encoding for integers is IDENTICAL to the in-memory representation for them
    typename NumericAttributeTraits<T>::StorageType storage;
    NumericAttributeTraits<T>::WorkingToStorage(value, storage);
    chip::Test::SetEmberReadOutput(ByteSpan(reinterpret_cast<const uint8_t *>(&storage), sizeof(storage)));

    // Data read via the encoder
    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding(&model);
    ASSERT_EQ(model.ReadAttribute(testRequest.request, *encoder), CHIP_NO_ERROR);
    ASSERT_EQ(testRequest.FinishEncoding(), CHIP_NO_ERROR);

    // Validate after read
    std::vector<DecodedAttributeData> attribute_data;
    ASSERT_EQ(testRequest.encodedIBs.Decode(attribute_data), CHIP_NO_ERROR);
    ASSERT_EQ(attribute_data.size(), 1u);

    DecodedAttributeData & encodedData = attribute_data[0];
    ASSERT_EQ(encodedData.attributePath, testRequest.request.path);

    typename NumericAttributeTraits<T>::WorkingType actual;
    ASSERT_EQ(chip::app::DataModel::Decode<typename NumericAttributeTraits<T>::WorkingType>(encodedData.dataReader, actual),
              CHIP_NO_ERROR);
    ASSERT_EQ(actual, value);
}

template <typename T, EmberAfAttributeType ZclType>
void TestEmberScalarNullRead()
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;
    ScopedMockAccessControl accessControl;

    TestReadRequest testRequest(
        kAdminSubjectDescriptor,
        ConcreteAttributePath(kMockEndpoint3, MockClusterId(4), MOCK_ATTRIBUTE_ID_FOR_NULLABLE_TYPE(ZclType)));

    // Ember encoding for integers is IDENTICAL to the in-memory representation for them
    typename NumericAttributeTraits<T>::StorageType nullValue;
    NumericAttributeTraits<T>::SetNull(nullValue);
    chip::Test::SetEmberReadOutput(ByteSpan(reinterpret_cast<const uint8_t *>(&nullValue), sizeof(nullValue)));

    // Data read via the encoder
    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding(&model);
    ASSERT_EQ(model.ReadAttribute(testRequest.request, *encoder), CHIP_NO_ERROR);
    ASSERT_EQ(testRequest.FinishEncoding(), CHIP_NO_ERROR);

    // Validate after read
    std::vector<DecodedAttributeData> attribute_data;
    ASSERT_EQ(testRequest.encodedIBs.Decode(attribute_data), CHIP_NO_ERROR);
    ASSERT_EQ(attribute_data.size(), 1u);

    DecodedAttributeData & encodedData = attribute_data[0];
    ASSERT_EQ(encodedData.attributePath, testRequest.request.path);
    chip::app::DataModel::Nullable<typename NumericAttributeTraits<T>::WorkingType> actual;
    ASSERT_EQ(chip::app::DataModel::Decode(encodedData.dataReader, actual), CHIP_NO_ERROR);
    ASSERT_TRUE(actual.IsNull());
}

} // namespace

TEST(TestCodegenModelViaMocks, IterateOverEndpoints)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;

    // This iteration relies on the hard-coding that occurs when mock_ember is used
    EXPECT_EQ(model.FirstEndpoint(), kMockEndpoint1);
    EXPECT_EQ(model.NextEndpoint(kMockEndpoint1), kMockEndpoint2);
    EXPECT_EQ(model.NextEndpoint(kMockEndpoint2), kMockEndpoint3);
    EXPECT_EQ(model.NextEndpoint(kMockEndpoint3), kInvalidEndpointId);

    /// Some out of order requests should work as well
    EXPECT_EQ(model.NextEndpoint(kMockEndpoint2), kMockEndpoint3);
    EXPECT_EQ(model.NextEndpoint(kMockEndpoint2), kMockEndpoint3);
    EXPECT_EQ(model.NextEndpoint(kMockEndpoint1), kMockEndpoint2);
    EXPECT_EQ(model.NextEndpoint(kMockEndpoint1), kMockEndpoint2);
    EXPECT_EQ(model.NextEndpoint(kMockEndpoint2), kMockEndpoint3);
    EXPECT_EQ(model.NextEndpoint(kMockEndpoint1), kMockEndpoint2);
    EXPECT_EQ(model.NextEndpoint(kMockEndpoint3), kInvalidEndpointId);
    EXPECT_EQ(model.NextEndpoint(kMockEndpoint3), kInvalidEndpointId);
    EXPECT_EQ(model.FirstEndpoint(), kMockEndpoint1);
    EXPECT_EQ(model.FirstEndpoint(), kMockEndpoint1);

    // invalid endpoiunts
    EXPECT_EQ(model.NextEndpoint(kInvalidEndpointId), kInvalidEndpointId);
    EXPECT_EQ(model.NextEndpoint(987u), kInvalidEndpointId);
}

TEST(TestCodegenModelViaMocks, IterateOverClusters)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;

    chip::Test::ResetVersion();

    EXPECT_FALSE(model.FirstCluster(kEndpointIdThatIsMissing).path.HasValidIds());
    EXPECT_FALSE(model.FirstCluster(kInvalidEndpointId).path.HasValidIds());
    EXPECT_FALSE(model.NextCluster(ConcreteClusterPath(kInvalidEndpointId, 123)).path.HasValidIds());
    EXPECT_FALSE(model.NextCluster(ConcreteClusterPath(kMockEndpoint1, kInvalidClusterId)).path.HasValidIds());
    EXPECT_FALSE(model.NextCluster(ConcreteClusterPath(kMockEndpoint1, 981u)).path.HasValidIds());

    // mock endpoint 1 has 2 mock clusters: 1 and 2
    ClusterEntry entry = model.FirstCluster(kMockEndpoint1);
    ASSERT_TRUE(entry.path.HasValidIds());
    EXPECT_EQ(entry.path.mEndpointId, kMockEndpoint1);
    EXPECT_EQ(entry.path.mClusterId, MockClusterId(1));
    EXPECT_EQ(entry.info.dataVersion, 0u);
    EXPECT_EQ(entry.info.flags.Raw(), 0u);

    chip::Test::BumpVersion();

    entry = model.NextCluster(entry.path);
    ASSERT_TRUE(entry.path.HasValidIds());
    EXPECT_EQ(entry.path.mEndpointId, kMockEndpoint1);
    EXPECT_EQ(entry.path.mClusterId, MockClusterId(2));
    EXPECT_EQ(entry.info.dataVersion, 1u);
    EXPECT_EQ(entry.info.flags.Raw(), 0u);

    entry = model.NextCluster(entry.path);
    EXPECT_FALSE(entry.path.HasValidIds());

    // mock endpoint 3 has 4 mock clusters: 1 through 4
    entry = model.FirstCluster(kMockEndpoint3);
    for (uint16_t clusterId = 1; clusterId <= 4; clusterId++)
    {
        ASSERT_TRUE(entry.path.HasValidIds());
        EXPECT_EQ(entry.path.mEndpointId, kMockEndpoint3);
        EXPECT_EQ(entry.path.mClusterId, MockClusterId(clusterId));
        entry = model.NextCluster(entry.path);
    }
    EXPECT_FALSE(entry.path.HasValidIds());

    // repeat calls should work
    for (int i = 0; i < 10; i++)
    {
        entry = model.FirstCluster(kMockEndpoint1);
        ASSERT_TRUE(entry.path.HasValidIds());
        EXPECT_EQ(entry.path.mEndpointId, kMockEndpoint1);
        EXPECT_EQ(entry.path.mClusterId, MockClusterId(1));
    }

    for (int i = 0; i < 10; i++)
    {
        ClusterEntry nextEntry = model.NextCluster(entry.path);
        ASSERT_TRUE(nextEntry.path.HasValidIds());
        EXPECT_EQ(nextEntry.path.mEndpointId, kMockEndpoint1);
        EXPECT_EQ(nextEntry.path.mClusterId, MockClusterId(2));
    }
}

TEST(TestCodegenModelViaMocks, GetClusterInfo)
{

    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;

    chip::Test::ResetVersion();

    ASSERT_FALSE(model.GetClusterInfo(ConcreteClusterPath(kInvalidEndpointId, kInvalidClusterId)).has_value());
    ASSERT_FALSE(model.GetClusterInfo(ConcreteClusterPath(kInvalidEndpointId, MockClusterId(1))).has_value());
    ASSERT_FALSE(model.GetClusterInfo(ConcreteClusterPath(kMockEndpoint1, kInvalidClusterId)).has_value());
    ASSERT_FALSE(model.GetClusterInfo(ConcreteClusterPath(kMockEndpoint1, MockClusterId(10))).has_value());

    // now get the value
    std::optional<ClusterInfo> info = model.GetClusterInfo(ConcreteClusterPath(kMockEndpoint1, MockClusterId(1)));
    ASSERT_TRUE(info.has_value());
    EXPECT_EQ(info->dataVersion, 0u); // NOLINT(bugprone-unchecked-optional-access)
    EXPECT_EQ(info->flags.Raw(), 0u); // NOLINT(bugprone-unchecked-optional-access)

    chip::Test::BumpVersion();
    info = model.GetClusterInfo(ConcreteClusterPath(kMockEndpoint1, MockClusterId(1)));
    ASSERT_TRUE(info.has_value());
    EXPECT_EQ(info->dataVersion, 1u); // NOLINT(bugprone-unchecked-optional-access)
    EXPECT_EQ(info->flags.Raw(), 0u); // NOLINT(bugprone-unchecked-optional-access)
}

TEST(TestCodegenModelViaMocks, IterateOverAttributes)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;

    // invalid paths should return in "no more data"
    ASSERT_FALSE(model.FirstAttribute(ConcreteClusterPath(kEndpointIdThatIsMissing, MockClusterId(1))).path.HasValidIds());
    ASSERT_FALSE(model.FirstAttribute(ConcreteClusterPath(kInvalidEndpointId, MockClusterId(1))).path.HasValidIds());
    ASSERT_FALSE(model.FirstAttribute(ConcreteClusterPath(kMockEndpoint1, MockClusterId(10))).path.HasValidIds());
    ASSERT_FALSE(model.FirstAttribute(ConcreteClusterPath(kMockEndpoint1, kInvalidClusterId)).path.HasValidIds());

    ASSERT_FALSE(model.NextAttribute(ConcreteAttributePath(kEndpointIdThatIsMissing, MockClusterId(1), 1u)).path.HasValidIds());
    ASSERT_FALSE(model.NextAttribute(ConcreteAttributePath(kInvalidEndpointId, MockClusterId(1), 1u)).path.HasValidIds());
    ASSERT_FALSE(model.NextAttribute(ConcreteAttributePath(kMockEndpoint1, MockClusterId(10), 1u)).path.HasValidIds());
    ASSERT_FALSE(model.NextAttribute(ConcreteAttributePath(kMockEndpoint1, kInvalidClusterId, 1u)).path.HasValidIds());
    ASSERT_FALSE(model.NextAttribute(ConcreteAttributePath(kMockEndpoint1, MockClusterId(1), 987u)).path.HasValidIds());

    // should be able to iterate over valid paths
    AttributeEntry entry = model.FirstAttribute(ConcreteClusterPath(kMockEndpoint2, MockClusterId(2)));
    ASSERT_TRUE(entry.path.HasValidIds());
    ASSERT_EQ(entry.path.mEndpointId, kMockEndpoint2);
    ASSERT_EQ(entry.path.mClusterId, MockClusterId(2));
    ASSERT_EQ(entry.path.mAttributeId, ClusterRevision::Id);
    ASSERT_FALSE(entry.info.flags.Has(AttributeQualityFlags::kListAttribute));

    entry = model.NextAttribute(entry.path);
    ASSERT_TRUE(entry.path.HasValidIds());
    ASSERT_EQ(entry.path.mEndpointId, kMockEndpoint2);
    ASSERT_EQ(entry.path.mClusterId, MockClusterId(2));
    ASSERT_EQ(entry.path.mAttributeId, FeatureMap::Id);
    ASSERT_FALSE(entry.info.flags.Has(AttributeQualityFlags::kListAttribute));

    entry = model.NextAttribute(entry.path);
    ASSERT_TRUE(entry.path.HasValidIds());
    ASSERT_EQ(entry.path.mEndpointId, kMockEndpoint2);
    ASSERT_EQ(entry.path.mClusterId, MockClusterId(2));
    ASSERT_EQ(entry.path.mAttributeId, MockAttributeId(1));
    ASSERT_FALSE(entry.info.flags.Has(AttributeQualityFlags::kListAttribute));

    entry = model.NextAttribute(entry.path);
    ASSERT_TRUE(entry.path.HasValidIds());
    ASSERT_EQ(entry.path.mEndpointId, kMockEndpoint2);
    ASSERT_EQ(entry.path.mClusterId, MockClusterId(2));
    ASSERT_EQ(entry.path.mAttributeId, MockAttributeId(2));
    ASSERT_TRUE(entry.info.flags.Has(AttributeQualityFlags::kListAttribute));

    entry = model.NextAttribute(entry.path);
    ASSERT_FALSE(entry.path.HasValidIds());

    // repeated calls should work
    for (int i = 0; i < 10; i++)
    {
        entry = model.FirstAttribute(ConcreteClusterPath(kMockEndpoint2, MockClusterId(2)));
        ASSERT_TRUE(entry.path.HasValidIds());
        ASSERT_EQ(entry.path.mEndpointId, kMockEndpoint2);
        ASSERT_EQ(entry.path.mClusterId, MockClusterId(2));
        ASSERT_EQ(entry.path.mAttributeId, ClusterRevision::Id);
        ASSERT_FALSE(entry.info.flags.Has(AttributeQualityFlags::kListAttribute));
    }

    for (int i = 0; i < 10; i++)
    {
        entry = model.NextAttribute(ConcreteAttributePath(kMockEndpoint2, MockClusterId(2), MockAttributeId(1)));
        ASSERT_TRUE(entry.path.HasValidIds());
        ASSERT_EQ(entry.path.mEndpointId, kMockEndpoint2);
        ASSERT_EQ(entry.path.mClusterId, MockClusterId(2));
        ASSERT_EQ(entry.path.mAttributeId, MockAttributeId(2));
        ASSERT_TRUE(entry.info.flags.Has(AttributeQualityFlags::kListAttribute));
    }
}

TEST(TestCodegenModelViaMocks, GetAttributeInfo)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;

    // various non-existent or invalid paths should return no info data
    ASSERT_FALSE(
        model.GetAttributeInfo(ConcreteAttributePath(kInvalidEndpointId, kInvalidClusterId, kInvalidAttributeId)).has_value());
    ASSERT_FALSE(model.GetAttributeInfo(ConcreteAttributePath(kInvalidEndpointId, kInvalidClusterId, FeatureMap::Id)).has_value());
    ASSERT_FALSE(model.GetAttributeInfo(ConcreteAttributePath(kInvalidEndpointId, MockClusterId(1), FeatureMap::Id)).has_value());
    ASSERT_FALSE(model.GetAttributeInfo(ConcreteAttributePath(kMockEndpoint1, kInvalidClusterId, FeatureMap::Id)).has_value());
    ASSERT_FALSE(model.GetAttributeInfo(ConcreteAttributePath(kMockEndpoint1, MockClusterId(10), FeatureMap::Id)).has_value());
    ASSERT_FALSE(model.GetAttributeInfo(ConcreteAttributePath(kMockEndpoint1, MockClusterId(10), kInvalidAttributeId)).has_value());
    ASSERT_FALSE(model.GetAttributeInfo(ConcreteAttributePath(kMockEndpoint1, MockClusterId(1), MockAttributeId(10))).has_value());

    // valid info
    std::optional<AttributeInfo> info =
        model.GetAttributeInfo(ConcreteAttributePath(kMockEndpoint1, MockClusterId(1), FeatureMap::Id));
    ASSERT_TRUE(info.has_value());
    EXPECT_FALSE(info->flags.Has(AttributeQualityFlags::kListAttribute)); // NOLINT(bugprone-unchecked-optional-access)

    info = model.GetAttributeInfo(ConcreteAttributePath(kMockEndpoint2, MockClusterId(2), MockAttributeId(2)));
    ASSERT_TRUE(info.has_value());
    EXPECT_TRUE(info->flags.Has(AttributeQualityFlags::kListAttribute)); // NOLINT(bugprone-unchecked-optional-access)
}

// global attributes are EXPLICITLY not supported
TEST(TestCodegenModelViaMocks, GlobalAttributeInfo)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;

    std::optional<AttributeInfo> info = model.GetAttributeInfo(
        ConcreteAttributePath(kMockEndpoint1, MockClusterId(1), Clusters::Globals::Attributes::GeneratedCommandList::Id));

    ASSERT_FALSE(info.has_value());

    info = model.GetAttributeInfo(
        ConcreteAttributePath(kMockEndpoint1, MockClusterId(1), Clusters::Globals::Attributes::AttributeList::Id));
    ASSERT_FALSE(info.has_value());
}

TEST(TestCodegenModelViaMocks, IterateOverAcceptedCommands)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;

    // invalid paths should return in "no more data"
    ASSERT_FALSE(model.FirstAcceptedCommand(ConcreteClusterPath(kEndpointIdThatIsMissing, MockClusterId(1))).path.HasValidIds());
    ASSERT_FALSE(model.FirstAcceptedCommand(ConcreteClusterPath(kInvalidEndpointId, MockClusterId(1))).path.HasValidIds());
    ASSERT_FALSE(model.FirstAcceptedCommand(ConcreteClusterPath(kMockEndpoint1, MockClusterId(10))).path.HasValidIds());
    ASSERT_FALSE(model.FirstAcceptedCommand(ConcreteClusterPath(kMockEndpoint1, kInvalidClusterId)).path.HasValidIds());

    // should be able to iterate over valid paths
    CommandEntry entry = model.FirstAcceptedCommand(ConcreteClusterPath(kMockEndpoint2, MockClusterId(2)));
    ASSERT_TRUE(entry.path.HasValidIds());
    EXPECT_EQ(entry.path.mEndpointId, kMockEndpoint2);
    EXPECT_EQ(entry.path.mClusterId, MockClusterId(2));
    EXPECT_EQ(entry.path.mCommandId, 1u);

    entry = model.NextAcceptedCommand(entry.path);
    ASSERT_TRUE(entry.path.HasValidIds());
    EXPECT_EQ(entry.path.mEndpointId, kMockEndpoint2);
    EXPECT_EQ(entry.path.mClusterId, MockClusterId(2));
    EXPECT_EQ(entry.path.mCommandId, 2u);

    entry = model.NextAcceptedCommand(entry.path);
    ASSERT_TRUE(entry.path.HasValidIds());
    EXPECT_EQ(entry.path.mEndpointId, kMockEndpoint2);
    EXPECT_EQ(entry.path.mClusterId, MockClusterId(2));
    EXPECT_EQ(entry.path.mCommandId, 23u);

    entry = model.NextAcceptedCommand(entry.path);
    ASSERT_FALSE(entry.path.HasValidIds());

    // attempt some out-of-order requests as well
    entry = model.FirstAcceptedCommand(ConcreteClusterPath(kMockEndpoint2, MockClusterId(3)));
    ASSERT_TRUE(entry.path.HasValidIds());
    EXPECT_EQ(entry.path.mEndpointId, kMockEndpoint2);
    EXPECT_EQ(entry.path.mClusterId, MockClusterId(3));
    EXPECT_EQ(entry.path.mCommandId, 11u);

    for (int i = 0; i < 10; i++)
    {
        entry = model.NextAcceptedCommand(ConcreteCommandPath(kMockEndpoint2, MockClusterId(2), 2));
        ASSERT_TRUE(entry.path.HasValidIds());
        EXPECT_EQ(entry.path.mEndpointId, kMockEndpoint2);
        EXPECT_EQ(entry.path.mClusterId, MockClusterId(2));
        EXPECT_EQ(entry.path.mCommandId, 23u);
    }

    for (int i = 0; i < 10; i++)
    {
        entry = model.NextAcceptedCommand(ConcreteCommandPath(kMockEndpoint2, MockClusterId(2), 1));
        ASSERT_TRUE(entry.path.HasValidIds());
        EXPECT_EQ(entry.path.mEndpointId, kMockEndpoint2);
        EXPECT_EQ(entry.path.mClusterId, MockClusterId(2));
        EXPECT_EQ(entry.path.mCommandId, 2u);
    }

    for (int i = 0; i < 10; i++)
    {
        entry = model.NextAcceptedCommand(ConcreteCommandPath(kMockEndpoint2, MockClusterId(3), 10));
        EXPECT_FALSE(entry.path.HasValidIds());
    }
}

TEST(TestCodegenModelViaMocks, AcceptedCommandInfo)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;

    // invalid paths should return in "no more data"
    ASSERT_FALSE(model.GetAcceptedCommandInfo(ConcreteCommandPath(kEndpointIdThatIsMissing, MockClusterId(1), 1)).has_value());
    ASSERT_FALSE(model.GetAcceptedCommandInfo(ConcreteCommandPath(kInvalidEndpointId, MockClusterId(1), 1)).has_value());
    ASSERT_FALSE(model.GetAcceptedCommandInfo(ConcreteCommandPath(kMockEndpoint1, MockClusterId(10), 1)).has_value());
    ASSERT_FALSE(model.GetAcceptedCommandInfo(ConcreteCommandPath(kMockEndpoint1, kInvalidClusterId, 1)).has_value());
    ASSERT_FALSE(
        model.GetAcceptedCommandInfo(ConcreteCommandPath(kMockEndpoint1, MockClusterId(1), kInvalidCommandId)).has_value());

    std::optional<CommandInfo> info = model.GetAcceptedCommandInfo(ConcreteCommandPath(kMockEndpoint2, MockClusterId(2), 1u));
    ASSERT_TRUE(info.has_value());

    info = model.GetAcceptedCommandInfo(ConcreteCommandPath(kMockEndpoint2, MockClusterId(2), 2u));
    ASSERT_TRUE(info.has_value());

    info = model.GetAcceptedCommandInfo(ConcreteCommandPath(kMockEndpoint2, MockClusterId(2), 1u));
    ASSERT_TRUE(info.has_value());

    info = model.GetAcceptedCommandInfo(ConcreteCommandPath(kMockEndpoint2, MockClusterId(2), 1u));
    ASSERT_TRUE(info.has_value());

    info = model.GetAcceptedCommandInfo(ConcreteCommandPath(kMockEndpoint2, MockClusterId(2), 23u));
    ASSERT_TRUE(info.has_value());

    info = model.GetAcceptedCommandInfo(ConcreteCommandPath(kMockEndpoint2, MockClusterId(2), 1234u));
    ASSERT_FALSE(info.has_value());
}

TEST(TestCodegenModelViaMocks, IterateOverGeneratedCommands)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;

    // invalid paths should return in "no more data"
    ASSERT_FALSE(model.FirstGeneratedCommand(ConcreteClusterPath(kEndpointIdThatIsMissing, MockClusterId(1))).HasValidIds());
    ASSERT_FALSE(model.FirstGeneratedCommand(ConcreteClusterPath(kInvalidEndpointId, MockClusterId(1))).HasValidIds());
    ASSERT_FALSE(model.FirstGeneratedCommand(ConcreteClusterPath(kMockEndpoint1, MockClusterId(10))).HasValidIds());
    ASSERT_FALSE(model.FirstGeneratedCommand(ConcreteClusterPath(kMockEndpoint1, kInvalidClusterId)).HasValidIds());

    // should be able to iterate over valid paths
    ConcreteCommandPath path = model.FirstGeneratedCommand(ConcreteClusterPath(kMockEndpoint2, MockClusterId(2)));
    ASSERT_TRUE(path.HasValidIds());
    EXPECT_EQ(path.mEndpointId, kMockEndpoint2);
    EXPECT_EQ(path.mClusterId, MockClusterId(2));
    EXPECT_EQ(path.mCommandId, 2u);

    path = model.NextGeneratedCommand(path);
    ASSERT_TRUE(path.HasValidIds());
    EXPECT_EQ(path.mEndpointId, kMockEndpoint2);
    EXPECT_EQ(path.mClusterId, MockClusterId(2));
    EXPECT_EQ(path.mCommandId, 10u);

    path = model.NextGeneratedCommand(path);
    ASSERT_FALSE(path.HasValidIds());

    // attempt some out-of-order requests as well
    path = model.FirstGeneratedCommand(ConcreteClusterPath(kMockEndpoint2, MockClusterId(3)));
    ASSERT_TRUE(path.HasValidIds());
    EXPECT_EQ(path.mEndpointId, kMockEndpoint2);
    EXPECT_EQ(path.mClusterId, MockClusterId(3));
    EXPECT_EQ(path.mCommandId, 4u);

    for (int i = 0; i < 10; i++)
    {
        path = model.NextGeneratedCommand(ConcreteCommandPath(kMockEndpoint2, MockClusterId(2), 2));
        ASSERT_TRUE(path.HasValidIds());
        EXPECT_EQ(path.mEndpointId, kMockEndpoint2);
        EXPECT_EQ(path.mClusterId, MockClusterId(2));
        EXPECT_EQ(path.mCommandId, 10u);
    }

    for (int i = 0; i < 10; i++)
    {
        path = model.NextGeneratedCommand(ConcreteCommandPath(kMockEndpoint2, MockClusterId(3), 4));
        ASSERT_TRUE(path.HasValidIds());
        EXPECT_EQ(path.mEndpointId, kMockEndpoint2);
        EXPECT_EQ(path.mClusterId, MockClusterId(3));
        EXPECT_EQ(path.mCommandId, 6u);
    }

    for (int i = 0; i < 10; i++)
    {
        path = model.NextGeneratedCommand(ConcreteCommandPath(kMockEndpoint2, MockClusterId(3), 6));
        EXPECT_FALSE(path.HasValidIds());
    }
}

TEST(TestCodegenModelViaMocks, EmberAttributeReadAclDeny)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;
    ScopedMockAccessControl accessControl;

    TestReadRequest testRequest(kDenySubjectDescriptor,
                                ConcreteAttributePath(kMockEndpoint1, MockClusterId(1), MockAttributeId(10)));
    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding(&model);

    ASSERT_EQ(model.ReadAttribute(testRequest.request, *encoder), CHIP_ERROR_ACCESS_DENIED);
}

TEST(TestCodegenModelViaMocks, EmberAttributeInvalidRead)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;
    ScopedMockAccessControl accessControl;

    // Invalid attribute
    {
        TestReadRequest testRequest(kAdminSubjectDescriptor,
                                    ConcreteAttributePath(kMockEndpoint1, MockClusterId(1), MockAttributeId(10)));
        std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding(&model);

        ASSERT_EQ(model.ReadAttribute(testRequest.request, *encoder), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    }

    // Invalid cluster
    {
        TestReadRequest testRequest(kAdminSubjectDescriptor,
                                    ConcreteAttributePath(kMockEndpoint1, MockClusterId(100), MockAttributeId(1)));
        std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding(&model);

        ASSERT_EQ(model.ReadAttribute(testRequest.request, *encoder), CHIP_IM_GLOBAL_STATUS(UnsupportedCluster));
    }

    // Invalid endpoint
    {
        TestReadRequest testRequest(kAdminSubjectDescriptor,
                                    ConcreteAttributePath(kEndpointIdThatIsMissing, MockClusterId(1), MockAttributeId(1)));
        std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding(&model);

        ASSERT_EQ(model.ReadAttribute(testRequest.request, *encoder), CHIP_IM_GLOBAL_STATUS(UnsupportedEndpoint));
    }
}

TEST(TestCodegenModelViaMocks, EmberAttributeReadInt32S)
{
    TestEmberScalarTypeRead<int32_t, ZCL_INT32S_ATTRIBUTE_TYPE>(-1234);
}

TEST(TestCodegenModelViaMocks, EmberAttributeReadEnum16)
{
    TestEmberScalarTypeRead<uint16_t, ZCL_ENUM16_ATTRIBUTE_TYPE>(0x1234);
}

TEST(TestCodegenModelViaMocks, EmberAttributeReadFloat)
{
    TestEmberScalarTypeRead<float, ZCL_SINGLE_ATTRIBUTE_TYPE>(0.625);
}

TEST(TestCodegenModelViaMocks, EmberAttributeReadDouble)
{
    TestEmberScalarTypeRead<double, ZCL_DOUBLE_ATTRIBUTE_TYPE>(0.625);
}

TEST(TestCodegenModelViaMocks, EmberAttributeReadInt24U)
{
    TestEmberScalarTypeRead<OddSizedInteger<3, false>, ZCL_INT24U_ATTRIBUTE_TYPE>(0x1234AB);
}

TEST(TestCodegenModelViaMocks, EmberAttributeReadInt32U)
{
    TestEmberScalarTypeRead<uint32_t, ZCL_INT32U_ATTRIBUTE_TYPE>(0x1234ABCD);
}

TEST(TestCodegenModelViaMocks, EmberAttributeReadInt40U)
{
    TestEmberScalarTypeRead<OddSizedInteger<5, false>, ZCL_INT40U_ATTRIBUTE_TYPE>(0x1122334455);
}

TEST(TestCodegenModelViaMocks, EmberAttributeReadInt48U)
{
    TestEmberScalarTypeRead<OddSizedInteger<6, false>, ZCL_INT48U_ATTRIBUTE_TYPE>(0xAABB11223344);
}

TEST(TestCodegenModelViaMocks, EmberAttributeReadInt56U)
{
    TestEmberScalarTypeRead<OddSizedInteger<7, false>, ZCL_INT56U_ATTRIBUTE_TYPE>(0xAABB11223344);
}

TEST(TestCodegenModelViaMocks, EmberAttributeReadBool)
{
    TestEmberScalarTypeRead<bool, ZCL_BOOLEAN_ATTRIBUTE_TYPE>(true);
    TestEmberScalarTypeRead<bool, ZCL_BOOLEAN_ATTRIBUTE_TYPE>(false);
}

TEST(TestCodegenModelViaMocks, EmberAttributeReadInt8U)
{
    TestEmberScalarTypeRead<uint8_t, ZCL_INT8U_ATTRIBUTE_TYPE>(0x12);
}

TEST(TestCodegenModelViaMocks, EmberAttributeReadNulls)
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

TEST(TestCodegenModelViaMocks, EmberAttributeReadErrorReading)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;
    ScopedMockAccessControl accessControl;

    {
        TestReadRequest testRequest(
            kAdminSubjectDescriptor,
            ConcreteAttributePath(kMockEndpoint3, MockClusterId(4),
                                  MOCK_ATTRIBUTE_ID_FOR_NULLABLE_TYPE(ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE)));

        chip::Test::SetEmberReadOutput(Protocols::InteractionModel::Status::Failure);

        // Actual read via an encoder
        std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding(&model);
        ASSERT_EQ(model.ReadAttribute(testRequest.request, *encoder), CHIP_IM_GLOBAL_STATUS(Failure));
    }

    {
        TestReadRequest testRequest(
            kAdminSubjectDescriptor,
            ConcreteAttributePath(kMockEndpoint3, MockClusterId(4),
                                  MOCK_ATTRIBUTE_ID_FOR_NULLABLE_TYPE(ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE)));

        chip::Test::SetEmberReadOutput(Protocols::InteractionModel::Status::Busy);

        // Actual read via an encoder
        std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding(&model);
        ASSERT_EQ(model.ReadAttribute(testRequest.request, *encoder), CHIP_IM_GLOBAL_STATUS(Busy));
    }
}

TEST(TestCodegenModelViaMocks, EmberAttributeReadNullOctetString)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;
    ScopedMockAccessControl accessControl;

    TestReadRequest testRequest(kAdminSubjectDescriptor,
                                ConcreteAttributePath(kMockEndpoint3, MockClusterId(4),
                                                      MOCK_ATTRIBUTE_ID_FOR_NULLABLE_TYPE(ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE)));

    // NOTE: This is a pascal string of size 0xFFFF which for null strings is a null marker
    char data[] = "\xFF\xFFInvalid length string is null";
    chip::Test::SetEmberReadOutput(ByteSpan(reinterpret_cast<const uint8_t *>(data), sizeof(data)));

    // Actual read via an encoder
    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding(&model);
    ASSERT_EQ(model.ReadAttribute(testRequest.request, *encoder), CHIP_NO_ERROR);
    ASSERT_EQ(testRequest.FinishEncoding(), CHIP_NO_ERROR);

    // Validate after read
    std::vector<DecodedAttributeData> attribute_data;
    ASSERT_EQ(testRequest.encodedIBs.Decode(attribute_data), CHIP_NO_ERROR);
    ASSERT_EQ(attribute_data.size(), 1u);

    DecodedAttributeData & encodedData = attribute_data[0];
    ASSERT_EQ(encodedData.attributePath, testRequest.request.path);

    // data element should be null for the given 0xFFFF length
    ASSERT_EQ(encodedData.dataReader.GetType(), TLV::kTLVType_Null);

    chip::app::DataModel::Nullable<ByteSpan> actual;
    ASSERT_EQ(chip::app::DataModel::Decode(encodedData.dataReader, actual), CHIP_NO_ERROR);
    ASSERT_TRUE(actual.IsNull());
}

TEST(TestCodegenModelViaMocks, EmberAttributeReadOctetString)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;
    ScopedMockAccessControl accessControl;

    TestReadRequest testRequest(
        kAdminSubjectDescriptor,
        ConcreteAttributePath(kMockEndpoint3, MockClusterId(4),
                              MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE)));

    // NOTE: This is a pascal string, so actual data is "test"
    //       the longer encoding is to make it clear we do not encode the overflow
    char data[]  = "\0\0testing here with overflow";
    uint16_t len = 4;
    memcpy(data, &len, sizeof(uint16_t));
    chip::Test::SetEmberReadOutput(ByteSpan(reinterpret_cast<const uint8_t *>(data), sizeof(data)));

    // Actual read via an encoder
    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding(&model);
    ASSERT_EQ(model.ReadAttribute(testRequest.request, *encoder), CHIP_NO_ERROR);
    ASSERT_EQ(testRequest.FinishEncoding(), CHIP_NO_ERROR);

    // Validate after read
    std::vector<DecodedAttributeData> attribute_data;
    ASSERT_EQ(testRequest.encodedIBs.Decode(attribute_data), CHIP_NO_ERROR);
    ASSERT_EQ(attribute_data.size(), 1u);

    const DecodedAttributeData & encodedData = attribute_data[0];
    ASSERT_EQ(encodedData.attributePath, testRequest.request.path);

    // data element should be a encoded byte string as this is what the attribute type is
    ASSERT_EQ(encodedData.dataReader.GetType(), TLV::kTLVType_ByteString);
    ByteSpan actual;
    ASSERT_EQ(encodedData.dataReader.Get(actual), CHIP_NO_ERROR);

    ByteSpan expected(reinterpret_cast<const uint8_t *>(data + 2), 4);
    ASSERT_TRUE(actual.data_equal(expected));
}

TEST(TestCodegenModelViaMocks, EmberAttributeReadLongOctetString)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;
    ScopedMockAccessControl accessControl;

    TestReadRequest testRequest(kAdminSubjectDescriptor,
                                ConcreteAttributePath(kMockEndpoint3, MockClusterId(4),
                                                      MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_OCTET_STRING_ATTRIBUTE_TYPE)));

    // NOTE: This is a pascal string, so actual data is "test"
    //       the longer encoding is to make it clear we do not encode the overflow
    const char data[] = "\x04testing here with overflow";
    chip::Test::SetEmberReadOutput(ByteSpan(reinterpret_cast<const uint8_t *>(data), sizeof(data)));

    // Actual read via an encoder
    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding(&model);
    ASSERT_EQ(model.ReadAttribute(testRequest.request, *encoder), CHIP_NO_ERROR);
    ASSERT_EQ(testRequest.FinishEncoding(), CHIP_NO_ERROR);

    // Validate after read
    std::vector<DecodedAttributeData> attribute_data;
    ASSERT_EQ(testRequest.encodedIBs.Decode(attribute_data), CHIP_NO_ERROR);
    ASSERT_EQ(attribute_data.size(), 1u);

    const DecodedAttributeData & encodedData = attribute_data[0];
    ASSERT_EQ(encodedData.attributePath, testRequest.request.path);

    // data element should be a encoded byte string as this is what the attribute type is
    ASSERT_EQ(encodedData.dataReader.GetType(), TLV::kTLVType_ByteString);
    ByteSpan actual;
    ASSERT_EQ(encodedData.dataReader.Get(actual), CHIP_NO_ERROR);

    ByteSpan expected(reinterpret_cast<const uint8_t *>(data + 1), 4);
    ASSERT_TRUE(actual.data_equal(expected));
}

TEST(TestCodegenModelViaMocks, EmberAttributeReadShortString)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;
    ScopedMockAccessControl accessControl;

    TestReadRequest testRequest(kAdminSubjectDescriptor,
                                ConcreteAttributePath(kMockEndpoint3, MockClusterId(4),
                                                      MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_CHAR_STRING_ATTRIBUTE_TYPE)));

    // NOTE: This is a pascal string, so actual data is "abcde"
    //       the longer encoding is to make it clear we do not encode the overflow
    char data[]  = "\0abcdef...this is the alphabet";
    uint16_t len = 5;
    memcpy(data, &len, sizeof(uint8_t));
    chip::Test::SetEmberReadOutput(ByteSpan(reinterpret_cast<const uint8_t *>(data), sizeof(data)));

    // Actual read via an encoder
    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding(&model);
    ASSERT_EQ(model.ReadAttribute(testRequest.request, *encoder), CHIP_NO_ERROR);
    ASSERT_EQ(testRequest.FinishEncoding(), CHIP_NO_ERROR);

    // Validate after reading
    std::vector<DecodedAttributeData> attribute_data;
    ASSERT_EQ(testRequest.encodedIBs.Decode(attribute_data), CHIP_NO_ERROR);
    ASSERT_EQ(attribute_data.size(), 1u);

    const DecodedAttributeData & encodedData = attribute_data[0];
    ASSERT_EQ(encodedData.attributePath, testRequest.request.path);

    // data element should be a encoded byte string as this is what the attribute type is
    ASSERT_EQ(encodedData.dataReader.GetType(), TLV::kTLVType_UTF8String);
    CharSpan actual;
    ASSERT_EQ(encodedData.dataReader.Get(actual), CHIP_NO_ERROR);
    ASSERT_TRUE(actual.data_equal("abcde"_span));
}

TEST(TestCodegenModelViaMocks, EmberAttributeReadLongString)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;
    ScopedMockAccessControl accessControl;

    TestReadRequest testRequest(
        kAdminSubjectDescriptor,
        ConcreteAttributePath(kMockEndpoint3, MockClusterId(4),
                              MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE)));

    // NOTE: This is a pascal string, so actual data is "abcde"
    //       the longer encoding is to make it clear we do not encode the overflow
    char data[]  = "\0\0abcdef...this is the alphabet";
    uint16_t len = 5;
    memcpy(data, &len, sizeof(uint16_t));
    chip::Test::SetEmberReadOutput(ByteSpan(reinterpret_cast<const uint8_t *>(data), sizeof(data)));

    // Actual read via an encoder
    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding(&model);
    ASSERT_EQ(model.ReadAttribute(testRequest.request, *encoder), CHIP_NO_ERROR);
    ASSERT_EQ(testRequest.FinishEncoding(), CHIP_NO_ERROR);

    // Validate after reading
    std::vector<DecodedAttributeData> attribute_data;
    ASSERT_EQ(testRequest.encodedIBs.Decode(attribute_data), CHIP_NO_ERROR);
    ASSERT_EQ(attribute_data.size(), 1u);

    const DecodedAttributeData & encodedData = attribute_data[0];
    ASSERT_EQ(encodedData.attributePath, testRequest.request.path);

    // data element should be a encoded byte string as this is what the attribute type is
    ASSERT_EQ(encodedData.dataReader.GetType(), TLV::kTLVType_UTF8String);
    CharSpan actual;
    ASSERT_EQ(encodedData.dataReader.Get(actual), CHIP_NO_ERROR);
    ASSERT_TRUE(actual.data_equal("abcde"_span));
}

TEST(TestCodegenModelViaMocks, AttributeAccessInterfaceStructRead)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;
    ScopedMockAccessControl accessControl;

    const ConcreteAttributePath kStructPath(kMockEndpoint3, MockClusterId(4),
                                            MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_STRUCT_ATTRIBUTE_TYPE));

    TestReadRequest testRequest(kAdminSubjectDescriptor, kStructPath);
    RegisteredAttributeAccessInterface<StructAttributeAccessInterface> aai(kStructPath);

    aai->SetReturnedData(Clusters::UnitTesting::Structs::SimpleStruct::Type{
        .a = 123,
        .b = true,
        .e = "foo"_span,
        .g = 0.5,
        .h = 0.125,
    });

    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding(&model);
    ASSERT_EQ(model.ReadAttribute(testRequest.request, *encoder), CHIP_NO_ERROR);
    ASSERT_EQ(testRequest.FinishEncoding(), CHIP_NO_ERROR);

    // Validate after read
    std::vector<DecodedAttributeData> attribute_data;
    ASSERT_EQ(testRequest.encodedIBs.Decode(attribute_data), CHIP_NO_ERROR);
    ASSERT_EQ(attribute_data.size(), 1u);

    DecodedAttributeData & encodedData = attribute_data[0];
    ASSERT_EQ(encodedData.attributePath, testRequest.request.path);

    ASSERT_EQ(encodedData.dataReader.GetType(), TLV::kTLVType_Structure);
    Clusters::UnitTesting::Structs::SimpleStruct::DecodableType actual;
    ASSERT_EQ(chip::app::DataModel::Decode(encodedData.dataReader, actual), CHIP_NO_ERROR);

    ASSERT_EQ(actual.a, 123);
    ASSERT_EQ(actual.b, true);
    ASSERT_EQ(actual.g, 0.5);
    ASSERT_EQ(actual.h, 0.125);
    ASSERT_TRUE(actual.e.data_equal("foo"_span));
}

TEST(TestCodegenModelViaMocks, AttributeAccessInterfaceListRead)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;
    ScopedMockAccessControl accessControl;

    const ConcreteAttributePath kStructPath(kMockEndpoint3, MockClusterId(4),
                                            MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_ARRAY_ATTRIBUTE_TYPE));

    TestReadRequest testRequest(kAdminSubjectDescriptor, kStructPath);
    RegisteredAttributeAccessInterface<ListAttributeAcessInterface> aai(kStructPath);

    constexpr unsigned kDataCount = 5;
    aai->SetReturnedData(Clusters::UnitTesting::Structs::SimpleStruct::Type{
        .b = true,
        .e = "xyz"_span,
        .g = 0.25,
        .h = 0.5,
    });
    aai->SetReturnedDataCount(kDataCount);

    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding(&model);
    ASSERT_EQ(model.ReadAttribute(testRequest.request, *encoder), CHIP_NO_ERROR);
    ASSERT_EQ(testRequest.FinishEncoding(), CHIP_NO_ERROR);

    // Validate after read
    std::vector<DecodedAttributeData> attribute_data;
    ASSERT_EQ(testRequest.encodedIBs.Decode(attribute_data), CHIP_NO_ERROR);
    ASSERT_EQ(attribute_data.size(), 1u);

    DecodedAttributeData & encodedData = attribute_data[0];
    ASSERT_EQ(encodedData.attributePath, testRequest.request.path);

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

TEST(TestCodegenModelViaMocks, AttributeAccessInterfaceListOverflowRead)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;
    ScopedMockAccessControl accessControl;

    const ConcreteAttributePath kStructPath(kMockEndpoint3, MockClusterId(4),
                                            MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_ARRAY_ATTRIBUTE_TYPE));

    TestReadRequest testRequest(kAdminSubjectDescriptor, kStructPath);
    RegisteredAttributeAccessInterface<ListAttributeAcessInterface> aai(kStructPath);

    constexpr unsigned kDataCount = 1024;
    aai->SetReturnedData(Clusters::UnitTesting::Structs::SimpleStruct::Type{
        .b = true,
        .e = "thisislongertofillupfaster"_span,
        .g = 0.25,
        .h = 0.5,
    });
    aai->SetReturnedDataCount(kDataCount);

    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding(&model);
    // NOTE: overflow, however data should be valid. Technically both NO_MEMORY and BUFFER_TOO_SMALL
    // should be ok here, however we know buffer-too-small is the error in this case hence
    // the compare (easier to write the test and read the output)
    ASSERT_EQ(model.ReadAttribute(testRequest.request, *encoder), CHIP_ERROR_BUFFER_TOO_SMALL);
    ASSERT_EQ(testRequest.FinishEncoding(), CHIP_NO_ERROR);

    // Validate after read
    std::vector<DecodedAttributeData> attribute_data;
    ASSERT_EQ(testRequest.encodedIBs.Decode(attribute_data), CHIP_NO_ERROR);
    ASSERT_EQ(attribute_data.size(), 1u);

    DecodedAttributeData & encodedData = attribute_data[0];
    ASSERT_EQ(encodedData.attributePath, testRequest.request.path);

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

TEST(TestCodegenModelViaMocks, AttributeAccessInterfaceListIncrementalRead)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;
    ScopedMockAccessControl accessControl;

    const ConcreteAttributePath kStructPath(kMockEndpoint3, MockClusterId(4),
                                            MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_ARRAY_ATTRIBUTE_TYPE));

    TestReadRequest testRequest(kAdminSubjectDescriptor, kStructPath);
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

    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding(&model, encodeState);
    // NOTE: overflow, however data should be valid. Technically both NO_MEMORY and BUFFER_TOO_SMALL
    // should be ok here, however we know buffer-too-small is the error in this case hence
    // the compare (easier to write the test and read the output)
    ASSERT_EQ(model.ReadAttribute(testRequest.request, *encoder), CHIP_ERROR_BUFFER_TOO_SMALL);
    ASSERT_EQ(testRequest.FinishEncoding(), CHIP_NO_ERROR);

    // Validate after read
    std::vector<DecodedAttributeData> attribute_data;
    ASSERT_EQ(testRequest.encodedIBs.Decode(attribute_data), CHIP_NO_ERROR);

    // Incremental encodes are separate list items, repeated
    // actual size IS ARBITRARY (current test sets it at 11)
    ASSERT_GT(attribute_data.size(), 3u);

    for (unsigned i = 0; i < attribute_data.size(); i++)
    {
        DecodedAttributeData & encodedData = attribute_data[i];
        ASSERT_EQ(encodedData.attributePath.mEndpointId, testRequest.request.path.mEndpointId);
        ASSERT_EQ(encodedData.attributePath.mClusterId, testRequest.request.path.mClusterId);
        ASSERT_EQ(encodedData.attributePath.mAttributeId, testRequest.request.path.mAttributeId);
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

TEST(TestCodegenModelViaMocks, ReadGlobalAttributeAttributeList)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel model;
    ScopedMockAccessControl accessControl;

    TestReadRequest testRequest(kAdminSubjectDescriptor,
                                ConcreteAttributePath(kMockEndpoint2, MockClusterId(3), AttributeList::Id));

    // Data read via the encoder
    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding(&model);
    ASSERT_EQ(model.ReadAttribute(testRequest.request, *encoder), CHIP_NO_ERROR);
    ASSERT_EQ(testRequest.FinishEncoding(), CHIP_NO_ERROR);

    // Validate after read
    std::vector<DecodedAttributeData> attribute_data;
    ASSERT_EQ(testRequest.encodedIBs.Decode(attribute_data), CHIP_NO_ERROR);
    ASSERT_EQ(attribute_data.size(), 1u);

    DecodedAttributeData & encodedData = attribute_data[0];
    ASSERT_EQ(encodedData.attributePath, testRequest.request.path);

    ASSERT_EQ(encodedData.dataReader.GetType(), TLV::kTLVType_Array);

    std::vector<AttributeId> items;
    ASSERT_EQ(DecodeList(encodedData.dataReader, items), CHIP_NO_ERROR);

    // Mock data contains ClusterRevision and FeatureMap.
    // After this, Global attributes are auto-added
    std::vector<AttributeId> expected;

    // Encoding in global-attribute-access-interface has a logic of:
    //   - Append global attributes in front of the first specified
    //     large number global attribute.
    // Since ClusterRevision and FeatureMap are
    // global attributes, the order here is reversed for them
    for (AttributeId id : GlobalAttributesNotInMetadata)
    {
        expected.push_back(id);
    }
    expected.push_back(ClusterRevision::Id);
    expected.push_back(FeatureMap::Id);
    expected.push_back(MockAttributeId(1));
    expected.push_back(MockAttributeId(2));
    expected.push_back(MockAttributeId(3));

    ASSERT_EQ(items.size(), expected.size());

    // Since we have no std::vector formatter, comparing element by element is somewhat
    // more readable in case of failure.
    for (unsigned i = 0; i < items.size(); i++)
    {
        EXPECT_EQ(items[i], expected[i]);
    }
}
