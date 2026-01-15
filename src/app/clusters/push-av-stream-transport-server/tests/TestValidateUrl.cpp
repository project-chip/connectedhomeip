/*
 *
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
#include <string>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/push-av-stream-transport-server/PushAVStreamTransportCluster.h>
#include <app/clusters/push-av-stream-transport-server/push-av-stream-transport-delegate.h>
#include <app/clusters/tls-client-management-server/TlsClientManagementCluster.h>
#include <app/server-cluster/testing/MockCommandHandler.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/Span.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace PushAvStreamTransport {

using TransportTriggerOptionsDecodableStruct = Structs::TransportTriggerOptionsStruct::DecodableType;
using TransportOptionsDecodableStruct        = Structs::TransportOptionsStruct::DecodableType;

using namespace Protocols::InteractionModel;

class TestValidateUrlDelegate : public PushAvStreamTransportDelegate
{
public:
    Status AllocatePushTransport(const TransportOptionsStruct &, const uint16_t, FabricIndex) override { return Status::Success; }

    Status DeallocatePushTransport(const uint16_t) override { return Status::Success; }

    Status ModifyPushTransport(const uint16_t, const TransportOptionsStorage) override { return Status::Success; }

    Status SetTransportStatus(const std::vector<uint16_t>, TransportStatusEnum) override { return Status::Success; }

    Status ManuallyTriggerTransport(const uint16_t, TriggerActivationReasonEnum,
                                    const Optional<Structs::TransportMotionTriggerTimeControlStruct::DecodableType> &) override
    {
        return Status::Success;
    }

    bool ValidateStreamUsage(StreamUsageEnum) override { return true; }

    bool ValidateSegmentDuration(uint16_t, const Optional<DataModel::Nullable<uint16_t>> &) override { return true; }

    Status ValidateBandwidthLimit(StreamUsageEnum, const Optional<DataModel::Nullable<uint16_t>> &,
                                  const Optional<DataModel::Nullable<uint16_t>> &) override
    {
        return Status::Success;
    }

    Status SelectVideoStream(StreamUsageEnum, uint16_t &) override { return Status::Success; }

    Status SelectAudioStream(StreamUsageEnum, uint16_t &) override { return Status::Success; }

    Status SetVideoStream(uint16_t) override { return Status::Success; }

    Status SetAudioStream(uint16_t) override { return Status::Success; }

    Status ValidateZoneId(uint16_t) override { return Status::Success; }

    bool ValidateMotionZoneListSize(size_t) override { return true; }

    PushAvStreamTransportStatusEnum GetTransportBusyStatus(const uint16_t) override
    {
        return PushAvStreamTransportStatusEnum::kIdle;
    }

    void OnAttributeChanged(AttributeId) override {}

    CHIP_ERROR LoadCurrentConnections(std::vector<TransportConfigurationStorage> &) override { return CHIP_NO_ERROR; }

    CHIP_ERROR PersistentAttributesLoadedCallback() override { return CHIP_NO_ERROR; }

    void SetTLSCerts(Tls::CertificateTable::BufferedClientCert &, Tls::CertificateTable::BufferedRootCert &) override {}

    CHIP_ERROR IsHardPrivacyModeActive(bool & isActive) override
    {
        isActive = false;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR IsSoftRecordingPrivacyModeActive(bool & isActive) override
    {
        isActive = false;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR IsSoftLivestreamPrivacyModeActive(bool & isActive) override
    {
        isActive = false;
        return CHIP_NO_ERROR;
    }

    void SetPushAvStreamTransportServer(PushAvStreamTransportServer *) override {}
};

class TestValidateUrlTlsDelegate : public TlsClientManagementDelegate
{
public:
    CHIP_ERROR Init(PersistentStorageDelegate &) override { return CHIP_NO_ERROR; }

    CHIP_ERROR ForEachEndpoint(EndpointId, FabricIndex, LoadedEndpointCallback) override { return CHIP_NO_ERROR; }

    ClusterStatusCode ProvisionEndpoint(EndpointId, FabricIndex,
                                        const TlsClientManagement::Commands::ProvisionEndpoint::DecodableType &,
                                        uint16_t &) override
    {
        return ClusterStatusCode(Status::Success);
    }

    CHIP_ERROR FindProvisionedEndpointByID(EndpointId, FabricIndex, uint16_t, LoadedEndpointCallback) override
    {
        return CHIP_NO_ERROR;
    }

    Status RemoveProvisionedEndpointByID(EndpointId, FabricIndex, uint16_t) override { return Status::Success; }

    CHIP_ERROR RootCertCanBeRemoved(EndpointId, FabricIndex, Tls::TLSCAID) override { return CHIP_NO_ERROR; }

    CHIP_ERROR ClientCertCanBeRemoved(EndpointId, FabricIndex, Tls::TLSCCDID) override { return CHIP_NO_ERROR; }

    void RemoveFabric(FabricIndex) override {}

    CHIP_ERROR MutateEndpointReferenceCount(EndpointId, FabricIndex, uint16_t, int8_t) override { return CHIP_NO_ERROR; }
};

class TestValidateUrl : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    TransportOptionsDecodableStruct CreateTransportOptionsWithUrl(const std::string & url, const std::string & trackName)
    {
        std::string cencKey   = "1234567890ABCDEF";
        std::string cencKeyID = "1234567890ABCDEF";

        CMAFContainerOptionsStruct cmafContainerOptions;
        ContainerOptionsStruct containerOptions;
        TransportTriggerOptionsDecodableStruct triggerOptions;

        TransportOptionsDecodableStruct transportOptions;

        // Create CMAFContainerOptionsStruct object
        cmafContainerOptions.segmentDuration = 4000;
        cmafContainerOptions.chunkDuration   = 1000;
        cmafContainerOptions.trackName       = Span(trackName.data(), trackName.size());
        cmafContainerOptions.metadataEnabled.ClearValue();

        cmafContainerOptions.CENCKey.SetValue(ByteSpan(reinterpret_cast<const uint8_t *>(cencKey.c_str()), cencKey.size()));
        cmafContainerOptions.CENCKeyID.SetValue(ByteSpan(reinterpret_cast<const uint8_t *>(cencKeyID.c_str()), cencKeyID.size()));

        // Create ContainerOptionsStruct object
        containerOptions.containerType = ContainerFormatEnum::kCmaf;
        containerOptions.CMAFContainerOptions.SetValue(cmafContainerOptions);

        triggerOptions.triggerType = TransportTriggerTypeEnum::kCommand;
        triggerOptions.motionZones.ClearValue();
        triggerOptions.motionSensitivity.ClearValue();
        triggerOptions.motionTimeControl.ClearValue();
        triggerOptions.maxPreRollLen.SetValue(1000);

        // Create TransportOptionsStruct object
        transportOptions.streamUsage = StreamUsageEnum::kAnalysis;
        transportOptions.videoStreamID.SetValue(1);
        transportOptions.audioStreamID.SetValue(2);
        transportOptions.TLSEndpointID    = 1;
        transportOptions.url              = Span(url.data(), url.size());
        transportOptions.triggerOptions   = triggerOptions;
        transportOptions.ingestMethod     = IngestMethodsEnum::kCMAFIngest;
        transportOptions.containerOptions = containerOptions;
        transportOptions.expiryTime.ClearValue();

        return transportOptions;
    }

    bool TestUrlValidation(const std::string & urlInput, bool shouldSucceed)
    {
        PushAvStreamTransportServer server(1, BitFlags<Feature>(1));
        TestValidateUrlDelegate mockDelegate;
        TestValidateUrlTlsDelegate tlsClientManagementDelegate;

        Testing::MockCommandHandler commandHandler;
        commandHandler.SetFabricIndex(1);
        ConcreteCommandPath kCommandPath{ 1, Clusters::PushAvStreamTransport::Id, Commands::AllocatePushTransport::Id };
        Commands::AllocatePushTransport::DecodableType commandData;

        std::string url              = urlInput;
        std::string trackName        = "test-track";
        commandData.transportOptions = CreateTransportOptionsWithUrl(url, trackName);

        server.GetLogic().SetDelegate(&mockDelegate);
        server.GetLogic().SetTLSClientManagementDelegate(&tlsClientManagementDelegate);

        auto result = server.GetLogic().HandleAllocatePushTransport(commandHandler, kCommandPath, commandData);

        if (shouldSucceed)
        {
            return result == std::nullopt && commandHandler.HasResponse();
        }

        if (result == std::nullopt && commandHandler.HasStatus())
        {
            return true;
        }
        return false;
    }
};

TEST_F(TestValidateUrl, ValidHttpsUrl)
{
    EXPECT_TRUE(TestUrlValidation("https://192.168.1.100:554/stream/", true));
    EXPECT_TRUE(TestUrlValidation("https://example.com/", true));
    EXPECT_TRUE(TestUrlValidation("https://example.com/path/", true));
    EXPECT_TRUE(TestUrlValidation("https://subdomain.example.com/", true));
    EXPECT_TRUE(TestUrlValidation("https://example.com:443/", true));
    EXPECT_TRUE(TestUrlValidation("https://192.168.1.1/", true));
}

TEST_F(TestValidateUrl, CaseInsensitiveScheme)
{
    // Case-insensitive scheme matching
    EXPECT_TRUE(TestUrlValidation("HTTPS://example.com/", true));
    EXPECT_TRUE(TestUrlValidation("Https://example.com/", true));
    EXPECT_TRUE(TestUrlValidation("HtTpS://example.com/", true));
    EXPECT_TRUE(TestUrlValidation("https://example.com/", true));
}

TEST_F(TestValidateUrl, InvalidScheme)
{
    // Non-HTTPS schemes should fail
    EXPECT_TRUE(TestUrlValidation("http://example.com/", false));
    EXPECT_TRUE(TestUrlValidation("ftp://example.com/", false));
    EXPECT_TRUE(TestUrlValidation("file://example.com/", false));
    EXPECT_TRUE(TestUrlValidation("ws://example.com/", false));
    EXPECT_TRUE(TestUrlValidation("wss://example.com/", false));
}

TEST_F(TestValidateUrl, MissingScheme)
{
    EXPECT_TRUE(TestUrlValidation("example.com/path/", false));
    EXPECT_TRUE(TestUrlValidation("//example.com/", false));
}

TEST_F(TestValidateUrl, MissingHost)
{
    EXPECT_TRUE(TestUrlValidation("https:///path/", false));
    EXPECT_TRUE(TestUrlValidation("https:///long/path/", false));
}

TEST_F(TestValidateUrl, WithFragment)
{
    // URLs with fragment should fail
    EXPECT_TRUE(TestUrlValidation("https://example.com/#fragment", false));
    EXPECT_TRUE(TestUrlValidation("https://example.com/path/#fragment", false));
    EXPECT_TRUE(TestUrlValidation("https://example.com/#", false));
}

TEST_F(TestValidateUrl, WithQuery)
{
    // URLs with query parameters should fail
    EXPECT_TRUE(TestUrlValidation("https://example.com/?query=value", false));
    EXPECT_TRUE(TestUrlValidation("https://example.com/path/?query=value", false));
    EXPECT_TRUE(TestUrlValidation("https://example.com/?", false));
}

TEST_F(TestValidateUrl, NoTrailingSlash)
{
    // URLs without trailing slash should fail
    EXPECT_TRUE(TestUrlValidation("https://example.com", false));
    EXPECT_TRUE(TestUrlValidation("https://example.com/path", false));
}

TEST_F(TestValidateUrl, MalformedUrl)
{
    EXPECT_TRUE(TestUrlValidation("https:/example.com/", false));
    EXPECT_TRUE(TestUrlValidation("https:example.com/", false));
    EXPECT_TRUE(TestUrlValidation("://example.com/", false));
}

TEST_F(TestValidateUrl, ComplexValidPaths)
{
    // Valid complex paths
    EXPECT_TRUE(TestUrlValidation("https://example.com/api/v1/", true));
    EXPECT_TRUE(TestUrlValidation("https://example.com/path/to/resource/", true));
    EXPECT_TRUE(TestUrlValidation("https://example.com/very/long/path/with/many/segments/", true));
}

TEST_F(TestValidateUrl, IPv6Addresses)
{
    // Properly parses IPv6 addresses
    EXPECT_TRUE(TestUrlValidation("https://[2001:0db8:85a3:0000:0000:8a2e:0370:7334]/", true));
    EXPECT_TRUE(TestUrlValidation("https://[::1]/", true));
    EXPECT_TRUE(TestUrlValidation("https://[2001:db8::1]/", true));
}

TEST_F(TestValidateUrl, PortNumbers)
{
    // URLs with port numbers should be valid
    EXPECT_TRUE(TestUrlValidation("https://example.com:443/", true));
    EXPECT_TRUE(TestUrlValidation("https://example.com:8080/", true));
    EXPECT_TRUE(TestUrlValidation("https://example.com:8443/", true));
}

} // namespace PushAvStreamTransport
} // namespace Clusters
} // namespace app
} // namespace chip
