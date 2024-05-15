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
#include <app/codegen-interaction-model/Model.h>

#include "EmberReadWriteOverride.h"
#include "TestAttributeReportIBsEncoding.h"

#include <access/AccessControl.h>
#include <access/SubjectDescriptor.h>
#include <app/MessageDef/ReportDataMessage.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <app/util/mock/MockNodeConfig.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVReader.h>
#include <lib/core/TLVWriter.h>

#include <gtest/gtest.h>
#include <vector>

// TODO: CHIP_ERROR tostring should be separated out
#include <pw_span/span.h>

using namespace chip;
using namespace chip::Test;
using namespace chip::app;
using namespace chip::app::InteractionModel;
using namespace chip::app::Clusters::Globals::Attributes;

namespace pw {

template <>
StatusWithSize ToString<CHIP_ERROR>(const CHIP_ERROR & err, pw::span<char> buffer)
{
    if (CHIP_ERROR::IsSuccess(err))
    {
        // source location probably does not matter
        return pw::string::Format(buffer, "CHIP_NO_ERROR");
    }
    return pw::string::Format(buffer, "CHIP_ERROR:<%" CHIP_ERROR_FORMAT ">", err.Format());
}

} // namespace pw

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

#define MOCK_ATTRIBUTE_ID_FOR_TYPE(zcl_type) MockAttributeId(zcl_type + 0x1000)
#define MOCK_ATTRIBUTE_CONFIG(zcl_type) MockAttributeConfig(MOCK_ATTRIBUTE_ID_FOR_TYPE(zcl_type), zcl_type)

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
        MockClusterConfig(MockClusterId(2), {
            ClusterRevision::Id,
            FeatureMap::Id,
            MockAttributeId(1),
            MockAttributeConfig(MockAttributeId(2), ZCL_ARRAY_ATTRIBUTE_TYPE),
        }),
        MockClusterConfig(MockClusterId(3), {
            ClusterRevision::Id, FeatureMap::Id, MockAttributeId(1), MockAttributeId(2), MockAttributeId(3),
        }),
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
            MOCK_ATTRIBUTE_CONFIG(ZCL_BOOLEAN_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_BITMAP8_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_BITMAP16_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_BITMAP32_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_BITMAP64_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_INT8U_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_INT16U_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_INT24U_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_INT32U_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_INT40U_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_INT48U_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_INT56U_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_INT64U_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_INT8S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_INT16S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_INT24S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_INT32S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_INT40S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_INT48S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_INT56S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_INT64S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_ENUM8_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_ENUM16_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_PRIORITY_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_STATUS_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_SINGLE_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_DOUBLE_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_OCTET_STRING_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_CHAR_STRING_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_ARRAY_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_STRUCT_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_GROUP_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_ENDPOINT_NO_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_VENDOR_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_DEVTYPE_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_FABRIC_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_FABRIC_IDX_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_ENTRY_IDX_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_DATA_VER_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_EVENT_NO_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_SEMTAG_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_NAMESPACE_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_TAG_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_SYSTIME_US_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_SYSTIME_MS_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_ELAPSED_S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_TEMPERATURE_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_POWER_MW_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_AMPERAGE_MA_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_VOLTAGE_MV_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_ENERGY_MWH_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_TOD_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_DATE_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_EPOCH_US_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_EPOCH_S_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_POSIX_MS_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_PERCENT_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_PERCENT100THS_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_CLUSTER_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_ATTRIB_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_FIELD_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_EVENT_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_COMMAND_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_ACTION_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_TRANS_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_NODE_ID_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_IPADR_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_IPV4ADR_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_IPV6ADR_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_IPV6PRE_ATTRIBUTE_TYPE),
            MOCK_ATTRIBUTE_CONFIG(ZCL_HWADR_ATTRIBUTE_TYPE),
        }),
    }),
});
// clang-format on

struct UseMockNodeConfig
{
    UseMockNodeConfig(const MockNodeConfig & config) { SetMockNodeConfig(config); }
    ~UseMockNodeConfig() { ResetMockNodeConfig(); }
};

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

    std::unique_ptr<AttributeValueEncoder> StartEncoding(chip::app::InteractionModel::Model * model)
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

        // TODO: isFabricFiltered? EncodeState?
        return std::make_unique<AttributeValueEncoder>(reportBuilder, request.subjectDescriptor.value(), request.path, dataVersion);
    }

    CHIP_ERROR FinishEncoding() { return encodedIBs.FinishEncoding(reportBuilder); }
};

} // namespace

TEST(TestCodegenModelViaMocks, IterateOverEndpoints)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel::Model model;

    // This iteration relies on the hard-coding that occurs when mock_ember is used
    EXPECT_EQ(model.FirstEndpoint(), kMockEndpoint1);
    EXPECT_EQ(model.NextEndpoint(kMockEndpoint1), kMockEndpoint2);
    ASSERT_EQ(model.NextEndpoint(kMockEndpoint2), kMockEndpoint3);
    ASSERT_EQ(model.NextEndpoint(kMockEndpoint3), kInvalidEndpointId);

    /// Some out of order requests should work as well
    ASSERT_EQ(model.NextEndpoint(kMockEndpoint2), kMockEndpoint3);
    ASSERT_EQ(model.NextEndpoint(kMockEndpoint2), kMockEndpoint3);
    ASSERT_EQ(model.NextEndpoint(kMockEndpoint1), kMockEndpoint2);
    ASSERT_EQ(model.NextEndpoint(kMockEndpoint1), kMockEndpoint2);
    ASSERT_EQ(model.NextEndpoint(kMockEndpoint2), kMockEndpoint3);
    ASSERT_EQ(model.NextEndpoint(kMockEndpoint1), kMockEndpoint2);
    ASSERT_EQ(model.NextEndpoint(kMockEndpoint3), kInvalidEndpointId);
    ASSERT_EQ(model.NextEndpoint(kMockEndpoint3), kInvalidEndpointId);
    ASSERT_EQ(model.FirstEndpoint(), kMockEndpoint1);
    ASSERT_EQ(model.FirstEndpoint(), kMockEndpoint1);
}

TEST(TestCodegenModelViaMocks, IterateOverClusters)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel::Model model;

    chip::Test::ResetVersion();

    EXPECT_FALSE(model.FirstCluster(kEndpointIdThatIsMissing).path.HasValidIds());
    EXPECT_FALSE(model.FirstCluster(kInvalidEndpointId).path.HasValidIds());

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
    chip::app::CodegenDataModel::Model model;

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
    chip::app::CodegenDataModel::Model model;

    // invalid paths should return in "no more data"
    ASSERT_FALSE(model.FirstAttribute(ConcreteClusterPath(kEndpointIdThatIsMissing, MockClusterId(1))).path.HasValidIds());
    ASSERT_FALSE(model.FirstAttribute(ConcreteClusterPath(kInvalidEndpointId, MockClusterId(1))).path.HasValidIds());
    ASSERT_FALSE(model.FirstAttribute(ConcreteClusterPath(kMockEndpoint1, MockClusterId(10))).path.HasValidIds());
    ASSERT_FALSE(model.FirstAttribute(ConcreteClusterPath(kMockEndpoint1, kInvalidClusterId)).path.HasValidIds());

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
    chip::app::CodegenDataModel::Model model;

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

class ADelegate : public Access::AccessControl::Delegate
{
public:
    virtual CHIP_ERROR Check(const Access::SubjectDescriptor & subjectDescriptor, const Access::RequestPath & requestPath,
                             Access::Privilege requestPrivilege)
    {
        // return CHIP_ERROR_ACCESS_DENIED;
        return CHIP_NO_ERROR;
    }
};

class AResolver : public Access::AccessControl::DeviceTypeResolver
{
public:
    bool IsDeviceTypeOnEndpoint(DeviceTypeId deviceType, EndpointId endpoint) override { return true; }
};

TEST(TestCodegenModelViaMocks, EmberAttributeReadAclDeny)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel::Model model;
    ScopedMockAccessControl accessControl;

    TestReadRequest testRequest(kDenySubjectDescriptor,
                                ConcreteAttributePath(kMockEndpoint1, MockClusterId(1), MockAttributeId(10)));
    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding(&model);

    ASSERT_EQ(model.ReadAttribute(testRequest.request, *encoder), CHIP_ERROR_ACCESS_DENIED);
}

TEST(TestCodegenModelViaMocks, EmberAttributeInvalidRead)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel::Model model;
    ScopedMockAccessControl accessControl;

    TestReadRequest testRequest(kAdminSubjectDescriptor,
                                ConcreteAttributePath(kMockEndpoint1, MockClusterId(1), MockAttributeId(10)));
    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding(&model);

    ASSERT_EQ(model.ReadAttribute(testRequest.request, *encoder), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
}

TEST(TestCodegenModelViaMocks, EmberAttributeReadInt32U)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel::Model model;
    ScopedMockAccessControl accessControl;

    TestReadRequest testRequest(
        kAdminSubjectDescriptor,
        ConcreteAttributePath(kMockEndpoint3, MockClusterId(4), MOCK_ATTRIBUTE_ID_FOR_TYPE(ZCL_INT32U_ATTRIBUTE_TYPE)));

    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding(&model);

    // Ember encoding for integers is IDENTICAL to the in-memory representation for them
    const uint32_t expected = 0x01020304;
    chip::Test::SetEmberReadOutput(ByteSpan(reinterpret_cast<const uint8_t *>(&expected), sizeof(expected)));

    ASSERT_EQ(model.ReadAttribute(testRequest.request, *encoder), CHIP_NO_ERROR);

    ASSERT_EQ(testRequest.FinishEncoding(), CHIP_NO_ERROR);

    /////// VALIDATE
    std::vector<DecodedAttributeData> attribute_data;
    ASSERT_EQ(testRequest.encodedIBs.Decode(attribute_data), CHIP_NO_ERROR);
    ASSERT_EQ(attribute_data.size(), 1u);

    const DecodedAttributeData & encodedData = attribute_data[0];
    ASSERT_EQ(encodedData.attributePath, testRequest.request.path);

    // data element should be a uint32 encoded as TLV
    ASSERT_EQ(encodedData.dataReader.GetType(), TLV::kTLVType_UnsignedInteger);
    uint32_t actual;
    ASSERT_EQ(encodedData.dataReader.Get(actual), CHIP_NO_ERROR);
    ASSERT_EQ(actual, expected);
}

TEST(TestCodegenModelViaMocks, EmberAttributeReadOctetString)
{
    UseMockNodeConfig config(gTestNodeConfig);
    chip::app::CodegenDataModel::Model model;
    ScopedMockAccessControl accessControl;

    TestReadRequest testRequest(
        kAdminSubjectDescriptor,
        ConcreteAttributePath(kMockEndpoint3, MockClusterId(4), MOCK_ATTRIBUTE_ID_FOR_TYPE(ZCL_INT32U_ATTRIBUTE_TYPE)));

    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding(&model);

    // NOTE: This is a pascal string, so actual data is "test"
    //       the longer encoding is to make it clear we do not encode the overflow
    const char data[] = "\x04testing here with overflow";
    chip::Test::SetEmberReadOutput(ByteSpan(reinterpret_cast<const uint8_t *>(data), sizeof(data)));

    ASSERT_EQ(model.ReadAttribute(testRequest.request, *encoder), CHIP_NO_ERROR);

    ASSERT_EQ(testRequest.FinishEncoding(), CHIP_NO_ERROR);

    /////// VALIDATE
    std::vector<DecodedAttributeData> attribute_data;
    ASSERT_EQ(testRequest.encodedIBs.Decode(attribute_data), CHIP_NO_ERROR);
    ASSERT_EQ(attribute_data.size(), 1u);

    const DecodedAttributeData & encodedData = attribute_data[0];
    ASSERT_EQ(encodedData.attributePath, testRequest.request.path);

    // data element should be a uint32 encoded as TLV
    ASSERT_EQ(encodedData.dataReader.GetType(), TLV::kTLVType_ByteString);
    ByteSpan actual;
    ASSERT_EQ(encodedData.dataReader.Get(actual), CHIP_NO_ERROR);

    ByteSpan expected(reinterpret_cast<const uint8_t *>(data + 1), 4);
    ASSERT_TRUE(actual.data_equal(expected));
}
