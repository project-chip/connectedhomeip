/*
 *    Copyright (c) 2020-2025 Project CHIP Authors
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

#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <app/clusters/basic-information/BasicInformationCluster.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/OperationTypes.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
// Add for ServerClusterContext
#include <app/SpecificationDefinedRevisions.h>
#include <app/persistence/AttributePersistence.h>
#include <clusters/BasicInformation/Attributes.h>
#include <clusters/BasicInformation/Enums.h>
#include <clusters/BasicInformation/Metadata.h>
#include <clusters/BasicInformation/Structs.h>
#include <cstdint>
#include <cstring>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/core/TLVReader.h>
#include <lib/core/TLVWriter.h>
#include <lib/support/BitFlags.h>
#include <lib/support/Span.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/PlatformManager.h>
#include <pw_unit_test/framework.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::BasicInformation;
using namespace chip::app::Clusters::BasicInformation::Attributes;

// Mock DeviceInstanceInfoProvider for testing
class MockDeviceInstanceInfoProvider : public DeviceLayer::DeviceInstanceInfoProvider
{
public:
    CHIP_ERROR GetVendorName(char * buf, size_t bufSize) override
    {
        strncpy(buf, "TestVendor", bufSize);
        buf[bufSize - 1] = '\0';
        return CHIP_NO_ERROR;
    }
    // Why not of VendorId enum type?
    CHIP_ERROR GetVendorId(uint16_t & vendorId) override
    {
        vendorId = static_cast<uint16_t>(VendorId::TestVendor1);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetProductName(char * buf, size_t bufSize) override
    {
        strncpy(buf, "TestProduct", bufSize);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetProductId(uint16_t & productId) override
    {
        productId = 0x5678;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetHardwareVersion(uint16_t & hardwareVersion) override
    {
        hardwareVersion = 1;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetHardwareVersionString(char * buf, size_t bufSize) override
    {
        strncpy(buf, "HW1.0", bufSize);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day) override
    {
        year  = 2023;
        month = 6;
        day   = 15;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetPartNumber(char * buf, size_t bufSize) override
    {
        strncpy(buf, "PART123", bufSize);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetProductURL(char * buf, size_t bufSize) override
    {
        strncpy(buf, "http://example.com", bufSize);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetProductLabel(char * buf, size_t bufSize) override
    {
        strncpy(buf, "Label123", bufSize);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetSerialNumber(char * buf, size_t bufSize) override
    {
        strncpy(buf, "SN123456", bufSize);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetProductFinish(ProductFinishEnum * finish) override
    {
        *finish = ProductFinishEnum::kMatte;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetProductPrimaryColor(ColorEnum * color) override
    {
        *color = ColorEnum::kBlack;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan) override { return CHIP_NO_ERROR; }
};

// Static member definition
MockDeviceInstanceInfoProvider gMockDeviceInstanceInfoProvider;
// Test fixture
struct TestBasicInformationReadWrite : public ::testing::Test
{
    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        DeviceLayer::SetDeviceInstanceInfoProvider(&gMockDeviceInstanceInfoProvider);
    }

    static void TearDownTestSuite()
    {
        DeviceLayer::SetDeviceInstanceInfoProvider(nullptr);
        chip::Platform::MemoryShutdown();
    }

    TestBasicInformationReadWrite() : testContext(), context(testContext.Create()) {}

    void SetUp() override
    {
        // Start the cluster with the context initialized in the constructor
        ASSERT_EQ(BasicInformationCluster::Instance().Startup(context), CHIP_NO_ERROR);
    }

    void TearDown() override { BasicInformationCluster::Instance().Shutdown(); }

    chip::Test::TestServerClusterContext testContext;
    ServerClusterContext context;
};

// Generic helper to read any attribute value into the provided type
template <typename DecodedT>
CHIP_ERROR ReadAttribute(const ConcreteAttributePath & path, DecodedT & value)
{
    AttributeReportIBs::Builder reportBuilder;
    uint8_t reportBuffer[512];
    TLV::TLVWriter reportWriter;
    reportWriter.Init(reportBuffer);
    reportBuilder.Init(&reportWriter);

    Access::SubjectDescriptor subjectDescriptor;
    AttributeValueEncoder encoder(reportBuilder, subjectDescriptor, path, 0 /* dataVersion */);

    DataModel::ReadAttributeRequest request;
    request.path                         = path;
    DataModel::ActionReturnStatus status = BasicInformationCluster::Instance().ReadAttribute(request, encoder);
    if (!status.IsSuccess())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ReturnErrorOnFailure(reportBuilder.EndOfAttributeReportIBs());
    ReturnErrorOnFailure(reportWriter.Finalize());

    TLV::TLVReader reader;
    reader.Init(reportBuffer, reportWriter.GetLengthWritten());
    ReturnErrorOnFailure(reader.Next());

    TLV::TLVType outerType;
    ReturnErrorOnFailure(reader.EnterContainer(outerType));
    ReturnErrorOnFailure(reader.Next());

    AttributeReportIB::Parser reportParser;
    ReturnErrorOnFailure(reportParser.Init(reader));

    AttributeDataIB::Parser dataParser;
    ReturnErrorOnFailure(reportParser.GetAttributeData(&dataParser));

    TLV::TLVReader dataReader;
    ReturnErrorOnFailure(dataParser.GetData(&dataReader));

    AttributeValueDecoder decoder(dataReader, subjectDescriptor);
    ReturnErrorOnFailure(decoder.Decode(value));

    return reader.ExitContainer(outerType);
}

TEST_F(TestBasicInformationReadWrite, TestReadVendorNameWithTLV)
{
    char vendorNameBuf[32];
    CharSpan vendorName(vendorNameBuf);

    ASSERT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, BasicInformation::Id, VendorName::Id), vendorName),
              CHIP_NO_ERROR);
    const char * expected = "TestVendor";
    ASSERT_TRUE(vendorName.data_equal(CharSpan::fromCharString(expected)));
}

TEST_F(TestBasicInformationReadWrite, TestNodeLabelLoadAndSave)
{
    // 1. GIVEN: A mock storage with a pre-existing "Old Label".
    const char * oldLabel = "Old Label";

    // Initialize AttributePersistence with the provider from *this test's* context
    AttributePersistence persistence(context.attributeStorage);

    Storage::String<32> labelStorage;
    labelStorage.SetContent(CharSpan::fromCharString(oldLabel));
    persistence.StoreString({ kRootEndpointId, BasicInformation::Id, NodeLabel::Id }, labelStorage);

    // 2. WHEN: The BasicInformationCluster starts up.
    // We must shut down the one from SetUp and re-start it to force a load.
    BasicInformationCluster::Instance().Shutdown();
    BasicInformationCluster::Instance().Startup(context);

    // 3. THEN: The cluster should have loaded "Old Label" into its memory.
    char readBuffer[32];
    CharSpan readSpan(readBuffer);

    // Read NodeLabel via generic helper
    EXPECT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, BasicInformation::Id, NodeLabel::Id), readSpan), CHIP_NO_ERROR);
    EXPECT_TRUE(readSpan.data_equal(CharSpan::fromCharString(oldLabel)));

    // 4. WHEN: A "New Label" is written to the attribute.
    const char * newLabel = "New Label";
    uint8_t tlvBuffer[128];
    TLV::TLVWriter writer;
    writer.Init(tlvBuffer);
    DataModel::Encode(writer, TLV::AnonymousTag(), CharSpan::fromCharString(newLabel));

    TLV::TLVReader reader;
    reader.Init(tlvBuffer, writer.GetLengthWritten());
    reader.Next();

    AttributeValueDecoder decoder(reader, Access::SubjectDescriptor{});
    DataModel::WriteAttributeRequest writeRequest;
    writeRequest.path = { kRootEndpointId, BasicInformation::Id, NodeLabel::Id };
    BasicInformationCluster::Instance().WriteAttribute(writeRequest, decoder);

    // 5. THEN: The cluster's in-memory value should be updated to "New Label".
    // Read NodeLabel via generic helper
    EXPECT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, BasicInformation::Id, NodeLabel::Id), readSpan), CHIP_NO_ERROR);
    EXPECT_TRUE(readSpan.data_equal(CharSpan::fromCharString(newLabel)));

    // 6. AND THEN: The "New Label" should have been saved back to persistent storage.
    Storage::String<32> persistedLabel;
    persistence.LoadString({ kRootEndpointId, BasicInformation::Id, NodeLabel::Id }, persistedLabel);
    EXPECT_TRUE(persistedLabel.Content().data_equal(CharSpan::fromCharString(newLabel)));
}

TEST_F(TestBasicInformationReadWrite, TestAllAttributesSpecCompliance)
{
    using namespace chip::app::Clusters::BasicInformation;
    // DataModelRevision
    {
        uint16_t val = 0;
        ASSERT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::DataModelRevision::Id), val), CHIP_NO_ERROR);
        EXPECT_EQ(val, Revision::kDataModelRevision);
    }

    // VendorName
    {
        char buf[64];
        CharSpan val(buf);
        ASSERT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::VendorName::Id), val), CHIP_NO_ERROR);
        EXPECT_TRUE(val.data_equal(CharSpan::fromCharString("TestVendor")));
    }

    // VendorID
    {
        VendorId val = VendorId::NotSpecified;
        ASSERT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::VendorID::Id), val), CHIP_NO_ERROR);
        EXPECT_EQ(val, VendorId::TestVendor1);
    }

    // ProductName
    {
        char buf[64];
        CharSpan val(buf);
        ASSERT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::ProductName::Id), val), CHIP_NO_ERROR);
        EXPECT_TRUE(val.data_equal(CharSpan::fromCharString("TestProduct")));
    }

    // ProductID
    {
        uint16_t val = 0;
        ASSERT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::ProductID::Id), val), CHIP_NO_ERROR);
        EXPECT_EQ(val, 0x5678);
    }

    // NodeLabel (default empty on fresh boot)
    {
        char buf[32];
        CharSpan val(buf);
        ASSERT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::NodeLabel::Id), val), CHIP_NO_ERROR);
        EXPECT_LE(val.size(), static_cast<size_t>(32));
    }

    // Location is fixed-length 2
    {
        char buf[8];
        CharSpan val(buf);
        ASSERT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::Location::Id), val), CHIP_NO_ERROR);
        EXPECT_EQ(val.size(), static_cast<size_t>(2));
    }

    // HardwareVersion
    {
        uint16_t val = 0;
        ASSERT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::HardwareVersion::Id), val), CHIP_NO_ERROR);
        EXPECT_EQ(val, 1);
    }

    // HardwareVersionString
    {
        char buf[64];
        CharSpan val(buf);
        ASSERT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::HardwareVersionString::Id), val),
                  CHIP_NO_ERROR);
        EXPECT_TRUE(val.data_equal(CharSpan::fromCharString("HW1.0")));
    }

    // SoftwareVersion (opaque value, just ensure decodes)
    {
        uint32_t val = 0;
        EXPECT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::SoftwareVersion::Id), val), CHIP_NO_ERROR);
    }

    // SoftwareVersionString (ensure decodes, non-empty typical)
    {
        char buf[128];
        CharSpan val(buf);
        EXPECT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::SoftwareVersionString::Id), val),
                  CHIP_NO_ERROR);
        EXPECT_LE(val.size(), sizeof(buf));
    }

    // ManufacturingDate (YYYYMMDD from mock)
    {
        char buf[32];
        CharSpan val(buf);
        ASSERT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::ManufacturingDate::Id), val), CHIP_NO_ERROR);
        EXPECT_TRUE(val.data_equal(CharSpan::fromCharString("20230615")));
    }

    // PartNumber
    {
        char buf[64];
        CharSpan val(buf);
        ASSERT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::PartNumber::Id), val), CHIP_NO_ERROR);
        EXPECT_TRUE(val.data_equal(CharSpan::fromCharString("PART123")));
    }

    // ProductURL
    {
        char buf[128];
        CharSpan val(buf);
        ASSERT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::ProductURL::Id), val), CHIP_NO_ERROR);
        EXPECT_TRUE(val.data_equal(CharSpan::fromCharString("http://example.com")));
    }

    // ProductLabel
    {
        char buf[64];
        CharSpan val(buf);
        ASSERT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::ProductLabel::Id), val), CHIP_NO_ERROR);
        EXPECT_TRUE(val.data_equal(CharSpan::fromCharString("Label123")));
    }

    // SerialNumber
    {
        char buf[64];
        CharSpan val(buf);
        ASSERT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::SerialNumber::Id), val), CHIP_NO_ERROR);
        EXPECT_TRUE(val.data_equal(CharSpan::fromCharString("SN123456")));
    }

    // LocalConfigDisabled (defaults to false)
    {
        bool val = true;
        ASSERT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::LocalConfigDisabled::Id), val),
                  CHIP_NO_ERROR);
        EXPECT_FALSE(val);
    }

    // Reachable (cluster returns true)
    {
        bool val = false;
        ASSERT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::Reachable::Id), val), CHIP_NO_ERROR);
        EXPECT_TRUE(val);
    }

    // UniqueID (Mandatory in Rev 4+, so if it fails, cluster rev must be < 4)
    {
        char buf[256];
        CharSpan val(buf);
        CHIP_ERROR err = ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::UniqueID::Id), val);
        if (err == CHIP_NO_ERROR)
        {
            EXPECT_LE(val.size(), sizeof(buf));
        }
        else
        {
            // If UniqueID cannot be read, the cluster MUST report a revision < 4.
            uint16_t clusterRev = 0;
            ASSERT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::ClusterRevision::Id), clusterRev),
                      CHIP_NO_ERROR);
            EXPECT_EQ(clusterRev, 5);
        }
    }

    // CapabilityMinima
    {
        Structs::CapabilityMinimaStruct::Type val;
        ASSERT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::CapabilityMinima::Id), val), CHIP_NO_ERROR);
        EXPECT_GE(val.caseSessionsPerFabric, 3);
        EXPECT_GE(val.subscriptionsPerFabric, 3);
    }

    // ProductAppearance
    {
        Structs::ProductAppearanceStruct::Type val;
        ASSERT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::ProductAppearance::Id), val), CHIP_NO_ERROR);
        EXPECT_EQ(val.finish, ProductFinishEnum::kMatte);
        ASSERT_FALSE(val.primaryColor.IsNull());
        EXPECT_EQ(val.primaryColor.Value(), ColorEnum::kBlack);
    }

    // SpecificationVersion
    {
        uint32_t val   = 0;
        CHIP_ERROR err = ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::SpecificationVersion::Id), val);
        if (err == CHIP_NO_ERROR)
        {
            // Expect a valid non-zero value for revision 3+
            EXPECT_GT(val, 0u);
        }
        else
        {
            // Accept failure if optional and cluster rev < 3
            uint16_t clusterRev = 0;
            ASSERT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::ClusterRevision::Id), clusterRev),
                      CHIP_NO_ERROR);
            if (clusterRev >= 3)
            {
                ADD_FAILURE() << "SpecificationVersion supported in rev >=3 but read failed: " << err;
            }
        }
    }

    // MaxPathsPerInvoke (ensure decodes and >= 1)
    {
        uint16_t val = 0;
        ASSERT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::MaxPathsPerInvoke::Id), val), CHIP_NO_ERROR);
        EXPECT_GE(val, 1);
    }

    // FeatureMap
    {
        uint32_t val = 1; // non-zero init
        ASSERT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::FeatureMap::Id), val), CHIP_NO_ERROR);
        EXPECT_EQ(val, 0u);
    }

    // ClusterRevision (with UniqueID enabled by default, should equal latest)
    {
        uint16_t val = 0;
        ASSERT_EQ(ReadAttribute(ConcreteAttributePath(kRootEndpointId, Id, Attributes::ClusterRevision::Id), val), CHIP_NO_ERROR);
        EXPECT_EQ(val, kRevision);
    }
}

// Generic helper to write any attribute value of a given type
template <typename T>
CHIP_ERROR WriteAttribute(const ConcreteAttributePath & path, const T & value)
{
    uint8_t tlvBuffer[128];
    TLV::TLVWriter writer;
    writer.Init(tlvBuffer);
    CHIP_ERROR err = DataModel::Encode(writer, TLV::AnonymousTag(), value);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    TLV::TLVReader reader;
    reader.Init(tlvBuffer, writer.GetLengthWritten());
    ReturnErrorOnFailure(reader.Next());

    AttributeValueDecoder decoder(reader, Access::SubjectDescriptor{});
    DataModel::WriteAttributeRequest writeRequest;
    writeRequest.path                    = path;
    DataModel::ActionReturnStatus status = BasicInformationCluster::Instance().WriteAttribute(writeRequest, decoder);

    return status.IsSuccess() ? CHIP_NO_ERROR : CHIP_ERROR_WRITE_FAILED;
}

TEST_F(TestBasicInformationReadWrite, TestWriteNodeLabel)
{
    // 1. ARRANGE: Define a new valid label
    const char * newLabelStr = "My Awesome Hub";
    CharSpan newLabel        = CharSpan::fromCharString(newLabelStr);
    char readBuffer[32];
    CharSpan readSpan(readBuffer);

    // 2. ACT: Write the new label to the attribute
    CHIP_ERROR writeErr = WriteAttribute(ConcreteAttributePath(kRootEndpointId, BasicInformation::Id, NodeLabel::Id), newLabel);
    ASSERT_EQ(writeErr, CHIP_NO_ERROR);

    // 3. ASSERT: Read the attribute back and verify it matches the new label
    CHIP_ERROR readErr = ReadAttribute(ConcreteAttributePath(kRootEndpointId, BasicInformation::Id, NodeLabel::Id), readSpan);
    ASSERT_EQ(readErr, CHIP_NO_ERROR);
    EXPECT_TRUE(readSpan.data_equal(newLabel));
}

TEST_F(TestBasicInformationReadWrite, TestWriteLocation)
{
    // --- Test Case 1: Write a valid 2-character location ---
    // TODO: Add case for valid 2-character location

    // --- Test Case 2: Write an invalid location (not 2 characters) ---
    // 1. ARRANGE: Define an invalid 3-character location. Spec requires exactly 2.
    const char * invalidLocationStr = "USA";
    CharSpan invalidLocation        = CharSpan::fromCharString(invalidLocationStr);

    // 2. ACT & ASSERT: Attempt to write the invalid location and confirm it fails
    CHIP_ERROR writeErr =
        WriteAttribute(ConcreteAttributePath(kRootEndpointId, BasicInformation::Id, Location::Id), invalidLocation);
    EXPECT_NE(writeErr, CHIP_NO_ERROR); // Expect a failure (ConstraintError)
}

TEST_F(TestBasicInformationReadWrite, TestWriteLocalConfigDisabled)
{
    bool readValue;

    // --- Test Case 1: Write 'true' ---

    // 1. ARRANGE: The default value is false, so we'll write true
    constexpr bool newValue = true;

    // 2. ACT: Write 'true' to the attribute
    CHIP_ERROR writeErr =
        WriteAttribute(ConcreteAttributePath(kRootEndpointId, BasicInformation::Id, LocalConfigDisabled::Id), newValue);
    ASSERT_EQ(writeErr, CHIP_NO_ERROR);

    // 3. ASSERT: Read the value back and confirm it is now true
    CHIP_ERROR readErr =
        ReadAttribute(ConcreteAttributePath(kRootEndpointId, BasicInformation::Id, LocalConfigDisabled::Id), readValue);
    ASSERT_EQ(readErr, CHIP_NO_ERROR);
    EXPECT_EQ(readValue, newValue);

    // --- Test Case 2: Write 'false' back ---

    // 1. ARRANGE: We will now write false back
    constexpr bool finalValue = false;

    // 2. ACT: Write 'false' to the attribute
    writeErr = WriteAttribute(ConcreteAttributePath(kRootEndpointId, BasicInformation::Id, LocalConfigDisabled::Id), finalValue);
    ASSERT_EQ(writeErr, CHIP_NO_ERROR);

    // 3. ASSERT: Read the value back and confirm it is now false
    readErr = ReadAttribute(ConcreteAttributePath(kRootEndpointId, BasicInformation::Id, LocalConfigDisabled::Id), readValue);
    ASSERT_EQ(readErr, CHIP_NO_ERROR);
    EXPECT_EQ(readValue, finalValue);
}

} // namespace
