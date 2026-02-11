/**
 *
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

#include <cassert>
#include <cstring>
#include <pw_unit_test/framework.h>
#include <string>
#include <uriparser/Uri.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/push-av-stream-transport-server/PushAVStreamTransportCluster.h>
#include <app/clusters/push-av-stream-transport-server/PushAVStreamTransportLogic.h>
#include <app/clusters/push-av-stream-transport-server/push-av-stream-transport-delegate.h>
#include <app/clusters/tls-client-management-server/TLSClientManagementCluster.h>
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

class TestUriExtractionHelpers : public ::testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

class TestValidateUrlDelegate : public PushAvStreamTransportDelegate
{
public:
    Status AllocatePushTransport(const TransportOptionsStruct &, const uint16_t, FabricIndex) override { return Status::Success; }
    Status DeallocatePushTransport(const uint16_t) override { return Status::Success; }
    Status ModifyPushTransport(const uint16_t, const TransportOptionsStorage) override { return Status::Success; }
    Status SetTransportStatus(const std::vector<uint16_t>, TransportStatusEnum) override { return Status::Success; }
    Status ManuallyTriggerTransport(const uint16_t, TriggerActivationReasonEnum,
                                    const Optional<Structs::TransportMotionTriggerTimeControlStruct::Type> &) override
    {
        return Status::Success;
    }
    bool ValidateStreamUsage(StreamUsageEnum) override { return true; }
    bool ValidateSegmentDuration(uint16_t, const Optional<DataModel::Nullable<uint16_t>> &) override { return true; }
    bool ValidateMaxPreRollLength(uint16_t maxPreRollLength, const DataModel::Nullable<uint16_t> & videoStreamId) override
    {
        return true;
    }
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
    bool GetCMAFSessionNumber(const uint16_t connectionID, uint64_t & sessionNumber) override
    {
        // Mock implementation for tests - return a simple session number
        sessionNumber = static_cast<uint64_t>(connectionID) + 1000;
        return true;
    }
};

class TestValidateUrlTLSDelegate : public TLSClientManagementDelegate
{
public:
    CHIP_ERROR Init(PersistentStorageDelegate &) override { return CHIP_NO_ERROR; }
    CHIP_ERROR ForEachEndpoint(EndpointId, FabricIndex, LoadedEndpointCallback) override { return CHIP_NO_ERROR; }
    CHIP_ERROR FindProvisionedEndpointByID(EndpointId, FabricIndex, uint16_t, LoadedEndpointCallback) override
    {
        return CHIP_NO_ERROR;
    }
    ClusterStatusCode ProvisionEndpoint(EndpointId, FabricIndex,
                                        const TlsClientManagement::Commands::ProvisionEndpoint::DecodableType &,
                                        uint16_t &) override
    {
        return ClusterStatusCode(Status::Success);
    }
    Status RemoveProvisionedEndpointByID(EndpointId, FabricIndex, uint16_t) override { return Status::Success; }
    void RemoveFabric(FabricIndex) override {}
    CHIP_ERROR MutateEndpointReferenceCount(EndpointId, FabricIndex, uint16_t, int8_t) override { return CHIP_NO_ERROR; }
    CHIP_ERROR RootCertCanBeRemoved(EndpointId, FabricIndex, Tls::TLSCAID) override { return CHIP_NO_ERROR; }
    CHIP_ERROR ClientCertCanBeRemoved(EndpointId, FabricIndex, Tls::TLSCCDID) override { return CHIP_NO_ERROR; }
};

class TestValidateUrl : public ::testing::Test
{
public:
    void SetUp() override
    {
        CHIP_ERROR err = chip::Platform::MemoryInit();
        assert(err == CHIP_NO_ERROR);
    }

    void TearDown() override { chip::Platform::MemoryShutdown(); }

    TransportOptionsDecodableStruct CreateTransportOptionsWithUrl(const std::string & urlInput, const std::string & trackName)
    {
        TransportOptionsDecodableStruct transportOptions;

        // Set URL
        transportOptions.url = Span(urlInput.data(), urlInput.size());
        transportOptions.videoStreamID.SetValue(1);
        transportOptions.audioStreamID.SetValue(2);

        // Set container options
        CMAFContainerOptionsStruct cmafOptions;
        cmafOptions.segmentDuration                     = 4000;
        cmafOptions.trackName                           = Span(trackName.data(), trackName.size());
        transportOptions.containerOptions.containerType = ContainerFormatEnum::kCmaf;
        transportOptions.containerOptions.CMAFContainerOptions.SetValue(cmafOptions);

        // Set trigger options
        transportOptions.triggerOptions.triggerType = TransportTriggerTypeEnum::kCommand;
        transportOptions.triggerOptions.maxPreRollLen.SetValue(1000);
        transportOptions.streamUsage   = StreamUsageEnum::kAnalysis;
        transportOptions.TLSEndpointID = 1;
        transportOptions.ingestMethod  = IngestMethodsEnum::kCMAFIngest;
        transportOptions.expiryTime.ClearValue();

        return transportOptions;
    }

    bool TestUrlValidation(const std::string & urlInput, bool shouldSucceed)
    {
        PushAvStreamTransportServer server(1, BitFlags<Feature>(1));
        TestValidateUrlDelegate mockDelegate;
        TestValidateUrlTLSDelegate tlsClientManagementDelegate;

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

        // HandleAllocatePushTransport always returns std::nullopt.
        // On success, it sets a response via the handler.
        // On failure, it sets a status via the handler.
        if (result != std::nullopt)
        {
            return false;
        }

        if (shouldSucceed)
        {
            return commandHandler.HasResponse();
        }

        return commandHandler.HasStatus();
    }
};

TEST_F(TestUriExtractionHelpers, ExtractTextRange_ValidRange)
{
    const char * text = "example";
    UriTextRangeA range;
    range.first     = text;
    range.afterLast = text + strlen(text);

    std::string result = Internal::extractTextRange(range);
    EXPECT_STREQ(result.c_str(), "example");
}

TEST_F(TestUriExtractionHelpers, ExtractTextRange_EmptyRange)
{
    const char * text = "example";
    UriTextRangeA range;
    range.first     = text;
    range.afterLast = text; // Same pointer = empty range

    std::string result = Internal::extractTextRange(range);
    EXPECT_STREQ(result.c_str(), "");
}

TEST_F(TestUriExtractionHelpers, ExtractTextRange_NullFirst)
{
    UriTextRangeA range;
    range.first     = nullptr;
    range.afterLast = reinterpret_cast<const char *>(0x1000);

    std::string result = Internal::extractTextRange(range);
    EXPECT_STREQ(result.c_str(), "");
}

TEST_F(TestUriExtractionHelpers, ExtractTextRange_NullAfterLast)
{
    const char * text = "example";
    UriTextRangeA range;
    range.first     = text;
    range.afterLast = nullptr;

    std::string result = Internal::extractTextRange(range);
    EXPECT_STREQ(result.c_str(), "");
}

TEST_F(TestUriExtractionHelpers, ExtractTextRange_InvalidOrder)
{
    const char * text = "example";
    UriTextRangeA range;
    range.first     = text + 5; // After afterLast
    range.afterLast = text;     // Before first (invalid)

    std::string result = Internal::extractTextRange(range);
    EXPECT_STREQ(result.c_str(), "");
}

TEST_F(TestUriExtractionHelpers, ExtractTextRange_SingleChar)
{
    const char * text = "a";
    UriTextRangeA range;
    range.first     = text;
    range.afterLast = text + 1;

    std::string result = Internal::extractTextRange(range);
    EXPECT_STREQ(result.c_str(), "a");
}

TEST_F(TestUriExtractionHelpers, ExtractTextRange_WithSpecialChars)
{
    const char * text = "path/to/resource?query=value#fragment";
    UriTextRangeA range;
    range.first     = text;
    range.afterLast = text + strlen(text);

    std::string result = Internal::extractTextRange(range);
    EXPECT_STREQ(result.c_str(), "path/to/resource?query=value#fragment");
}

TEST_F(TestUriExtractionHelpers, ExtractPath_NullHead)
{
    std::string result = Internal::extractPath(nullptr);
    EXPECT_STREQ(result.c_str(), "");
}

TEST_F(TestUriExtractionHelpers, ExtractPath_SingleSegment)
{
    // Create a single path segment
    char segmentText[] = "api";
    UriPathSegmentA segment;
    segment.text.first     = segmentText;
    segment.text.afterLast = segmentText + strlen(segmentText);
    segment.next           = nullptr;

    std::string result = Internal::extractPath(&segment);
    EXPECT_STREQ(result.c_str(), "/api");
}

TEST_F(TestUriExtractionHelpers, ExtractPath_MultipleSegments)
{
    // Create multiple path segments: /api/v1/resource
    char seg1Text[] = "api";
    char seg2Text[] = "v1";
    char seg3Text[] = "resource";

    UriPathSegmentA segment3;
    segment3.text.first     = seg3Text;
    segment3.text.afterLast = seg3Text + strlen(seg3Text);
    segment3.next           = nullptr;

    UriPathSegmentA segment2;
    segment2.text.first     = seg2Text;
    segment2.text.afterLast = seg2Text + strlen(seg2Text);
    segment2.next           = &segment3;

    UriPathSegmentA segment1;
    segment1.text.first     = seg1Text;
    segment1.text.afterLast = seg1Text + strlen(seg1Text);
    segment1.next           = &segment2;

    std::string result = Internal::extractPath(&segment1);
    EXPECT_STREQ(result.c_str(), "/api/v1/resource");
}

TEST_F(TestUriExtractionHelpers, ExtractPath_SegmentWithNullText)
{
    // Create a segment with null text pointers
    UriPathSegmentA segment;
    segment.text.first     = nullptr;
    segment.text.afterLast = nullptr;
    segment.next           = nullptr;

    std::string result = Internal::extractPath(&segment);
    EXPECT_STREQ(result.c_str(), "/");
}

TEST_F(TestUriExtractionHelpers, ExtractPath_SegmentWithEmptyText)
{
    // Create a segment with empty text (first == afterLast)
    char emptyText[] = "";
    UriPathSegmentA segment;
    segment.text.first     = emptyText;
    segment.text.afterLast = emptyText;
    segment.next           = nullptr;

    std::string result = Internal::extractPath(&segment);
    EXPECT_STREQ(result.c_str(), "/");
}

TEST_F(TestUriExtractionHelpers, ExtractPath_MixedValidAndNullSegments)
{
    // Create segments where some have null text
    char seg1Text[] = "api";
    UriPathSegmentA segment2;
    segment2.text.first     = nullptr;
    segment2.text.afterLast = nullptr;
    segment2.next           = nullptr;

    UriPathSegmentA segment1;
    segment1.text.first     = seg1Text;
    segment1.text.afterLast = seg1Text + strlen(seg1Text);
    segment1.next           = &segment2;

    std::string result = Internal::extractPath(&segment1);
    EXPECT_STREQ(result.c_str(), "/api/");
}

TEST_F(TestUriExtractionHelpers, ExtractPath_ComplexPath)
{
    // Create a complex path: /users/123/posts/456/comments
    char seg1Text[] = "users";
    char seg2Text[] = "123";
    char seg3Text[] = "posts";
    char seg4Text[] = "456";
    char seg5Text[] = "comments";

    UriPathSegmentA segment5;
    segment5.text.first     = seg5Text;
    segment5.text.afterLast = seg5Text + strlen(seg5Text);
    segment5.next           = nullptr;

    UriPathSegmentA segment4;
    segment4.text.first     = seg4Text;
    segment4.text.afterLast = seg4Text + strlen(seg4Text);
    segment4.next           = &segment5;

    UriPathSegmentA segment3;
    segment3.text.first     = seg3Text;
    segment3.text.afterLast = seg3Text + strlen(seg3Text);
    segment3.next           = &segment4;

    UriPathSegmentA segment2;
    segment2.text.first     = seg2Text;
    segment2.text.afterLast = seg2Text + strlen(seg2Text);
    segment2.next           = &segment3;

    UriPathSegmentA segment1;
    segment1.text.first     = seg1Text;
    segment1.text.afterLast = seg1Text + strlen(seg1Text);
    segment1.next           = &segment2;

    std::string result = Internal::extractPath(&segment1);
    EXPECT_STREQ(result.c_str(), "/users/123/posts/456/comments");
}

TEST_F(TestUriExtractionHelpers, ExtractPath_RealUrlWithTrailingSlash)
{
    // Test with a real URL parsed by uriparser
    const char * url = "https://example.com/";
    UriUriA uri;
    const char * errorPos;
    int result = uriParseSingleUriA(&uri, url, &errorPos);

    ASSERT_EQ(result, URI_SUCCESS);

    std::string path = Internal::extractPath(uri.pathHead);
    uriFreeUriMembersA(&uri);

    EXPECT_STREQ(path.c_str(), "/");
}

TEST_F(TestUriExtractionHelpers, ExtractPath_RealUrlWithPath)
{
    // Test with a real URL with a path segment
    const char * url = "https://192.168.1.100:554/stream/";
    UriUriA uri;
    const char * errorPos;
    int result = uriParseSingleUriA(&uri, url, &errorPos);

    ASSERT_EQ(result, URI_SUCCESS);

    std::string path = Internal::extractPath(uri.pathHead);
    uriFreeUriMembersA(&uri);

    EXPECT_STREQ(path.c_str(), "/stream/");
}

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
    EXPECT_TRUE(TestUrlValidation("https//example.com/", false));
    EXPECT_TRUE(TestUrlValidation("https/example.com/", false));
    EXPECT_TRUE(TestUrlValidation("https::://example.com/", false));
    EXPECT_TRUE(TestUrlValidation("https://example.com::443/", false));
    EXPECT_TRUE(TestUrlValidation("https:///example.com/", false));
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

TEST_F(TestValidateUrl, InvalidIPv6Addresses)
{
    // Invalid IPv6 syntax - missing brackets
    EXPECT_TRUE(TestUrlValidation("https://2001:0db8::1/", false));
    EXPECT_TRUE(TestUrlValidation("https://::12/", false));

    // Invalid IPv6 - multiple zippers (::)
    EXPECT_TRUE(TestUrlValidation("https://[2001::db8::1]/", false));
    EXPECT_TRUE(TestUrlValidation("https://[::1::2]/", false));

    // Invalid IPv6 - triple colon
    EXPECT_TRUE(TestUrlValidation("https://[:::/", false));
    EXPECT_TRUE(TestUrlValidation("https://[2001:::1]/", false));

    // Invalid IPv6 - too many segments
    EXPECT_TRUE(TestUrlValidation("https://[1:2:3:4:5:6:7:8:9]/", false));
    EXPECT_TRUE(TestUrlValidation("https://[::1:2:3:4:5:6:7:8]/", false));

    // Invalid IPv6 - invalid characters
    EXPECT_TRUE(TestUrlValidation("https://[g:0:0:0:0:0:0:1]/", false));
    EXPECT_TRUE(TestUrlValidation("https://[2001:0db8:g:1]/", false));

    // Invalid IPv6 - malformed IPv4 part
    EXPECT_TRUE(TestUrlValidation("https://[::ffff:256.1.1.1]/", false));
    EXPECT_TRUE(TestUrlValidation("https://[::ffff:1.2.3.256]/", false));
    EXPECT_TRUE(TestUrlValidation("https://[::ffff:1.2.3]/", false)); // Missing octet

    // Valid IPv6 addresses
    EXPECT_TRUE(TestUrlValidation("https://[2001:0db8:85a3:0000:0000:8a2e:0370:7334]/", true));
    EXPECT_TRUE(TestUrlValidation("https://[::1]/", true));
    EXPECT_TRUE(TestUrlValidation("https://[2001:db8::1]/", true));
    EXPECT_TRUE(TestUrlValidation("https://[::ffff:192.168.1.1]/", true)); // IPv4-mapped IPv6
}

TEST_F(TestValidateUrl, PercentEncodingEdgeCases)
{
    EXPECT_TRUE(TestUrlValidation("https://example.com/path%20with%20spaces/", true));
    EXPECT_TRUE(TestUrlValidation("https://example.com/path%2Fwith%2Fslashes/", true));

    // Invalid percent encoding - incomplete
    EXPECT_TRUE(TestUrlValidation("https://example.com/path%/", false));
    EXPECT_TRUE(TestUrlValidation("https://example.com/path%2/", false));

    // Invalid percent encoding - non-hex characters
    EXPECT_TRUE(TestUrlValidation("https://example.com/path%g2/", false));
    EXPECT_TRUE(TestUrlValidation("https://example.com/path%2g/", false));
}

TEST_F(TestValidateUrl, InvalidPort)
{
    EXPECT_TRUE(TestUrlValidation("https://example.com:port/", false));
    EXPECT_TRUE(TestUrlValidation("https://example.com:-1/", false));
    EXPECT_TRUE(TestUrlValidation("https://example.com:12abc/", false));
}

TEST_F(TestValidateUrl, VerifyPath)
{
    EXPECT_TRUE(TestUrlValidation("https://example.com/path-with-dashes/", true));
    EXPECT_TRUE(TestUrlValidation("https://example.com/path_with_underscores/", true));
    EXPECT_TRUE(TestUrlValidation("https://example.com/path.with.dots/", true));
    EXPECT_TRUE(TestUrlValidation("https://example.com/123/", true));

    EXPECT_TRUE(TestUrlValidation("https://user@pass@example.com/", false));

    EXPECT_TRUE(TestUrlValidation("https://example.com/path with spaces/", false));
    EXPECT_TRUE(TestUrlValidation("https://example.com/path\nwith\nnewlines/", false));
    EXPECT_TRUE(TestUrlValidation("https://example.com//", true));
    EXPECT_TRUE(TestUrlValidation("https://example.com///", true));
    EXPECT_TRUE(TestUrlValidation("https://example.com/api%2Fv1%2Fendpoint/", true));
}

TEST_F(TestValidateUrl, HostnameEdgeCases)
{
    EXPECT_TRUE(TestUrlValidation("https://example .com/", false));
    EXPECT_TRUE(TestUrlValidation("https://example\n.com/", false));
    EXPECT_TRUE(TestUrlValidation("https://example\t.com/", false));
}

} // namespace PushAvStreamTransport
} // namespace Clusters
} // namespace app
} // namespace chip
