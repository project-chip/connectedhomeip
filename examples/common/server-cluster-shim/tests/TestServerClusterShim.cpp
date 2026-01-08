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
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <app/util/mock/MockNodeConfig.h>
#include <data-model-providers/codegen/tests/EmberInvokeOverride.h>
#include <data-model-providers/codegen/tests/EmberReadWriteOverride.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <pw_unit_test/framework.h>
#include <server-cluster-shim/ServerClusterShim.h>

// This file has just basic tests for the ServerClusterShim.
// TestCodeDrivenModelViaMocks has a more comprehensive suite of tests for the DataModel
// and also validates the ServerClusterShim.

using namespace chip;
using namespace chip::Testing;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::DataModel;
using namespace chip::Testing;
using namespace chip::Protocols::InteractionModel;
using namespace chip::app::Clusters::Globals::Attributes;

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

#define MOCK_ATTRIBUTE_ID_FOR_NULLABLE_TYPE(zcl_type) MockAttributeId(zcl_type + 0x1000)
#define MOCK_ATTRIBUTE_CONFIG_NULLABLE(zcl_type)                                                                                   \
    MockAttributeConfig(MOCK_ATTRIBUTE_ID_FOR_NULLABLE_TYPE(zcl_type), zcl_type,                                                   \
                        MATTER_ATTRIBUTE_FLAG_WRITABLE | MATTER_ATTRIBUTE_FLAG_NULLABLE)

#define MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(zcl_type) MockAttributeId(zcl_type + 0x2000)
#define MOCK_ATTRIBUTE_CONFIG_NON_NULLABLE(zcl_type)                                                                               \
    MockAttributeConfig(MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(zcl_type), zcl_type, MATTER_ATTRIBUTE_FLAG_WRITABLE)

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
              MATTER_ATTRIBUTE_FLAG_NULLABLE    // NOTE: explicltly NOT MATTER_ATTRIBUTE_FLAG_WRITABLE
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
            MockAttributeConfig(kAttributeIdTimedWrite, ZCL_INT32S_ATTRIBUTE_TYPE, MATTER_ATTRIBUTE_FLAG_WRITABLE | MATTER_ATTRIBUTE_FLAG_MUST_USE_TIMED_WRITE),
        }),
    }),
});
// clang-format on

} // namespace

struct TestServerClusterShim : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestServerClusterShim, TestWriteAttributeOnlyProvidedPathsAreValid)
{
    TestServerClusterContext testContext;
    SetMockNodeConfig(gTestNodeConfig);
    ServerClusterShim invalid_cluster(
        { { kMockEndpoint1,
            MockClusterId(1) } }); // Only adds path endpoint 1, cluster 1, even though behind the scenes ember knows more stuff.
    EXPECT_SUCCESS(invalid_cluster.Startup(testContext.Get()));

    WriteOperation test(kMockEndpoint3, MockClusterId(4), MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_CHAR_STRING_ATTRIBUTE_TYPE));
    test.SetSubjectDescriptor(kAdminSubjectDescriptor);

    AttributeValueDecoder decoder = test.DecoderFor<CharSpan>("hello world"_span);

    SetEmberReadOutput(Protocols::InteractionModel::Status::Success);
    // Expect unsupported cluster because the ServerClusterShim instance was initialized with path 1,1 only
    ASSERT_EQ(invalid_cluster.WriteAttribute(test.GetRequest(), decoder), Status::Failure);

    // Create another cluster with the valid paths for the write command
    ServerClusterShim valid_cluster({ { kMockEndpoint1, MockClusterId(1) }, { kMockEndpoint3, MockClusterId(4) } });
    EXPECT_SUCCESS(valid_cluster.Startup(testContext.Get()));

    SetEmberReadOutput(Protocols::InteractionModel::Status::Success);
    // Expect success now
    ASSERT_EQ(valid_cluster.WriteAttribute(test.GetRequest(), decoder), CHIP_NO_ERROR);

    chip::ByteSpan writtenData = GetEmberBuffer();
    ASSERT_GT(writtenData.size(), 0U);

    chip::CharSpan asCharSpan(reinterpret_cast<const char *>(writtenData.data()), writtenData[0] + 1);
    ASSERT_TRUE(asCharSpan.data_equal("\x0Bhello world"_span));
}

TEST_F(TestServerClusterShim, TestDataVersion)
{
    TestServerClusterContext testContext;
    SetMockNodeConfig(gTestNodeConfig);

    ServerClusterShim cluster({ { kMockEndpoint3, MockClusterId(4) } });
    EXPECT_SUCCESS(cluster.Startup(testContext.Get()));

    DataVersion v1 = cluster.GetDataVersion({ kMockEndpoint3, MockClusterId(4) });

    WriteOperation test(kMockEndpoint3, MockClusterId(4), MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_CHAR_STRING_ATTRIBUTE_TYPE));
    test.SetSubjectDescriptor(kAdminSubjectDescriptor);

    AttributeValueDecoder decoder = test.DecoderFor<CharSpan>("hello world"_span);

    SetEmberReadOutput(Protocols::InteractionModel::Status::Success);
    ASSERT_EQ(cluster.WriteAttribute(test.GetRequest(), decoder), CHIP_NO_ERROR);

    ASSERT_EQ(cluster.GetDataVersion({ kMockEndpoint3, MockClusterId(4) }), v1 + 1);
}

TEST_F(TestServerClusterShim, AttributeWriteShortString)
{
    TestServerClusterContext testContext;
    SetMockNodeConfig(gTestNodeConfig);
    ServerClusterShim cluster({ { kMockEndpoint3, MockClusterId(4) } });
    EXPECT_SUCCESS(cluster.Startup(testContext.Get()));

    WriteOperation test(kMockEndpoint3, MockClusterId(4), MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_CHAR_STRING_ATTRIBUTE_TYPE));
    test.SetSubjectDescriptor(kAdminSubjectDescriptor);

    AttributeValueDecoder decoder = test.DecoderFor<CharSpan>("hello world"_span);

    SetEmberReadOutput(Protocols::InteractionModel::Status::Success);
    ASSERT_EQ(cluster.WriteAttribute(test.GetRequest(), decoder), CHIP_NO_ERROR);
    chip::ByteSpan writtenData = GetEmberBuffer();
    ASSERT_GT(writtenData.size(), 0U);

    chip::CharSpan asCharSpan(reinterpret_cast<const char *>(writtenData.data()), writtenData[0] + 1);
    ASSERT_TRUE(asCharSpan.data_equal("\x0Bhello world"_span));
}

TEST_F(TestServerClusterShim, EmberAttributeReadOctetString)
{
    TestServerClusterContext testContext;
    SetMockNodeConfig(gTestNodeConfig);
    ServerClusterShim invalid_cluster({ { 1, 1 } });
    ServerClusterShim valid_cluster({ { kMockEndpoint3, MockClusterId(4) } });
    EXPECT_SUCCESS(invalid_cluster.Startup(testContext.Get()));
    EXPECT_SUCCESS(valid_cluster.Startup(testContext.Get()));

    ReadOperation testRequest(kMockEndpoint3, MockClusterId(4),
                              MOCK_ATTRIBUTE_ID_FOR_NON_NULLABLE_TYPE(ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE));
    testRequest.SetSubjectDescriptor(kAdminSubjectDescriptor);

    // NOTE: This is a pascal string, so actual data is "test"
    //       the longer encoding is to make it clear we do not encode the overflow
    char data[] = "\0\0testing here with overflow";
    uint8_t * p = reinterpret_cast<uint8_t *>(data);
    chip::Encoding::LittleEndian::Write16(p, 4);
    SetEmberReadOutput(ByteSpan(reinterpret_cast<const uint8_t *>(data), sizeof(data)));

    // Read on invalid cluster, expect Status::Failure
    std::unique_ptr<AttributeValueEncoder> encoder = testRequest.StartEncoding();
    ASSERT_EQ(invalid_cluster.ReadAttribute(testRequest.GetRequest(), *encoder), Status::Failure);

    // Actual read via an encoder on the valid cluster
    ASSERT_EQ(valid_cluster.ReadAttribute(testRequest.GetRequest(), *encoder), CHIP_NO_ERROR);
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

TEST_F(TestServerClusterShim, IterateOverAttributes)
{
    TestServerClusterContext testContext;
    SetMockNodeConfig(gTestNodeConfig);
    ServerClusterShim cluster({ { kMockEndpoint3, MockClusterId(4) } });
    EXPECT_SUCCESS(cluster.Startup(testContext.Get()));

    // should be able to iterate over valid paths
    ReadOnlyBufferBuilder<DataModel::AttributeEntry> builder;

    // invalid paths return errors
    ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kEndpointIdThatIsMissing, MockClusterId(1)), builder), CHIP_ERROR_NOT_FOUND);
    ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kInvalidEndpointId, MockClusterId(1)), builder), CHIP_ERROR_NOT_FOUND);
    ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kMockEndpoint1, MockClusterId(10)), builder), CHIP_ERROR_NOT_FOUND);
    ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kMockEndpoint1, kInvalidClusterId), builder), CHIP_ERROR_NOT_FOUND);

    EXPECT_EQ(cluster.Attributes(ConcreteClusterPath(kMockEndpoint2, MockClusterId(2)), builder), CHIP_NO_ERROR);
    auto attributes = builder.TakeBuffer();
    ASSERT_EQ(attributes.size(), 7u);

    ASSERT_EQ(attributes[0].attributeId, ClusterRevision::Id);
    ASSERT_FALSE(attributes[0].HasFlags(AttributeQualityFlags::kListAttribute));

    ASSERT_EQ(attributes[1].attributeId, FeatureMap::Id);
    ASSERT_FALSE(attributes[1].HasFlags(AttributeQualityFlags::kListAttribute));

    ASSERT_EQ(attributes[2].attributeId, MockAttributeId(1));
    ASSERT_FALSE(attributes[2].HasFlags(AttributeQualityFlags::kListAttribute));

    ASSERT_EQ(attributes[3].attributeId, MockAttributeId(2));
    ASSERT_TRUE(attributes[3].HasFlags(AttributeQualityFlags::kListAttribute));

    // Ends with global list attributes
    ASSERT_EQ(attributes[4].attributeId, GeneratedCommandList::Id);
    ASSERT_TRUE(attributes[4].HasFlags(AttributeQualityFlags::kListAttribute));

    ASSERT_EQ(attributes[5].attributeId, AcceptedCommandList::Id);
    ASSERT_TRUE(attributes[5].HasFlags(AttributeQualityFlags::kListAttribute));

    ASSERT_EQ(attributes[6].attributeId, AttributeList::Id);
    ASSERT_TRUE(attributes[6].HasFlags(AttributeQualityFlags::kListAttribute));
}

TEST_F(TestServerClusterShim, EmberInvokeTest)
{
    // Ember invoke is fully code-generated - there is a single function for Dispatch
    // that will do a `switch` on the path elements and invoke a corresponding `emberAf*`
    // callback.
    //
    // The only thing that can be validated is that this `DispatchSingleClusterCommand`
    // is actually invoked.

    TestServerClusterContext testContext;
    SetMockNodeConfig(gTestNodeConfig);
    ServerClusterShim cluster_with_invalid_path({ { kMockEndpoint3, MockClusterId(4) } });
    ServerClusterShim cluster_with_valid_path({ { kMockEndpoint1, MockClusterId(1) } });

    EXPECT_SUCCESS(cluster_with_invalid_path.Startup(testContext.Get()));
    EXPECT_SUCCESS(cluster_with_valid_path.Startup(testContext.Get()));

    {
        const ConcreteCommandPath kCommandPath(kMockEndpoint1, MockClusterId(1), kMockCommandId1);
        const InvokeRequest kInvokeRequest{ .path = kCommandPath };
        chip::TLV::TLVReader tlvReader;

        const uint32_t kDispatchCountPre = DispatchCount();

        // Using a handler set to nullptr as it is not used by the impl
        ASSERT_EQ(cluster_with_invalid_path.InvokeCommand(kInvokeRequest, tlvReader, /* handler = */ nullptr), Status::Failure);
        EXPECT_EQ(DispatchCount(), kDispatchCountPre); // no dispatch expected

        ASSERT_EQ(cluster_with_valid_path.InvokeCommand(kInvokeRequest, tlvReader, /* handler = */ nullptr), std::nullopt);
        EXPECT_EQ(DispatchCount(), kDispatchCountPre + 1); // one dispatch expected
        EXPECT_EQ(GetLastDispatchPath(), kCommandPath);    // for the right path
    }

    {
        const ConcreteCommandPath kCommandPath(kMockEndpoint1, MockClusterId(1), kMockCommandId2);
        const InvokeRequest kInvokeRequest{ .path = kCommandPath };
        chip::TLV::TLVReader tlvReader;

        const uint32_t kDispatchCountPre = DispatchCount();

        // Using a handler set to nullptr as it is not used by the impl
        ASSERT_EQ(cluster_with_invalid_path.InvokeCommand(kInvokeRequest, tlvReader, /* handler = */ nullptr), Status::Failure);

        EXPECT_EQ(DispatchCount(), kDispatchCountPre); // no dispatch expected

        ASSERT_EQ(cluster_with_valid_path.InvokeCommand(kInvokeRequest, tlvReader, /* handler = */ nullptr), std::nullopt);
        EXPECT_EQ(DispatchCount(), kDispatchCountPre + 1); // one dispatch expected
        EXPECT_EQ(GetLastDispatchPath(), kCommandPath);    // for the right path
    }
}

TEST_F(TestServerClusterShim, IterateOverAcceptedCommands)
{
    TestServerClusterContext testContext;
    SetMockNodeConfig(gTestNodeConfig);
    ServerClusterShim cluster({ { kMockEndpoint3, MockClusterId(4) }, { kMockEndpoint2, MockClusterId(2) } });
    EXPECT_SUCCESS(cluster.Startup(testContext.Get()));

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> builder;

    // invalid paths should return in "no more data"
    ASSERT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kEndpointIdThatIsMissing, MockClusterId(1)), builder),
              CHIP_ERROR_NOT_FOUND);
    ASSERT_TRUE(builder.IsEmpty());
    ASSERT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kInvalidEndpointId, MockClusterId(1)), builder), CHIP_ERROR_NOT_FOUND);
    ASSERT_TRUE(builder.IsEmpty());
    ASSERT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kMockEndpoint1, MockClusterId(10)), builder), CHIP_ERROR_NOT_FOUND);
    ASSERT_TRUE(builder.IsEmpty());
    ASSERT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kMockEndpoint1, kInvalidClusterId), builder), CHIP_ERROR_NOT_FOUND);
    ASSERT_TRUE(builder.IsEmpty());

    // Valid ember path but not added to ServerCluster paths
    ASSERT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kMockEndpoint1, MockClusterId(1)), builder), CHIP_ERROR_NOT_FOUND);
    ASSERT_TRUE(builder.IsEmpty());

    ASSERT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kMockEndpoint2, MockClusterId(2)), builder), CHIP_NO_ERROR);
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

TEST_F(TestServerClusterShim, IterateOverGeneratedCommands)
{
    TestServerClusterContext testContext;
    SetMockNodeConfig(gTestNodeConfig);
    ServerClusterShim cluster({ { kMockEndpoint2, MockClusterId(2) }, { kMockEndpoint2, MockClusterId(3) } });
    EXPECT_SUCCESS(cluster.Startup(testContext.Get()));

    ServerClusterShim cluster_without_id3({ { kMockEndpoint2, MockClusterId(2) } });
    EXPECT_SUCCESS(cluster_without_id3.Startup(testContext.Get()));

    ReadOnlyBufferBuilder<CommandId> builder;

    // invalid paths should return in "no more data"
    ASSERT_EQ(cluster.GeneratedCommands(ConcreteClusterPath(kEndpointIdThatIsMissing, MockClusterId(1)), builder),
              CHIP_ERROR_NOT_FOUND);
    ASSERT_TRUE(builder.IsEmpty());
    ASSERT_EQ(cluster.GeneratedCommands(ConcreteClusterPath(kInvalidEndpointId, MockClusterId(1)), builder), CHIP_ERROR_NOT_FOUND);
    ASSERT_TRUE(builder.IsEmpty());
    ASSERT_EQ(cluster.GeneratedCommands(ConcreteClusterPath(kMockEndpoint1, MockClusterId(10)), builder), CHIP_ERROR_NOT_FOUND);
    ASSERT_TRUE(builder.IsEmpty());
    ASSERT_EQ(cluster.GeneratedCommands(ConcreteClusterPath(kMockEndpoint1, kInvalidClusterId), builder), CHIP_ERROR_NOT_FOUND);
    ASSERT_TRUE(builder.IsEmpty());

    // should fail if it's a valid ember path but not added to server cluster interface
    ASSERT_EQ(cluster_without_id3.GeneratedCommands(ConcreteClusterPath(kMockEndpoint2, MockClusterId(3)), builder),
              CHIP_ERROR_NOT_FOUND);
    ASSERT_TRUE(builder.IsEmpty());

    // should be able to iterate over valid paths
    const CommandId expectedCommands2[] = { 2, 10 };

    ASSERT_EQ(cluster.GeneratedCommands(ConcreteClusterPath(kMockEndpoint2, MockClusterId(2)), builder), CHIP_NO_ERROR);
    auto cmds = builder.TakeBuffer();
    ASSERT_TRUE(cmds.data_equal(Span<const CommandId>(expectedCommands2)));

    ASSERT_EQ(cluster_without_id3.GeneratedCommands(ConcreteClusterPath(kMockEndpoint2, MockClusterId(2)), builder), CHIP_NO_ERROR);
    cmds = builder.TakeBuffer();
    ASSERT_TRUE(cmds.data_equal(Span<const CommandId>(expectedCommands2)));

    ASSERT_EQ(cluster.GeneratedCommands(ConcreteClusterPath(kMockEndpoint2, MockClusterId(3)), builder), CHIP_NO_ERROR);
    cmds                                = builder.TakeBuffer();
    const CommandId expectedCommands3[] = { 4, 6 };
    ASSERT_TRUE(cmds.data_equal(Span<const CommandId>(expectedCommands3)));
}
