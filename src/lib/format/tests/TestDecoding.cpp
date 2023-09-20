/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <lib/core/TLVWriter.h>
#include <lib/format/protocol_decoder.h>
#include <lib/support/StringBuilder.h>
#include <lib/support/UnitTestRegistration.h>

#include <tlv/meta/clusters_meta.h>
#include <tlv/meta/protocols_meta.h>

#include <nlunit-test.h>

#include "sample_data.h"

namespace {

using namespace chip::Decoders;
using namespace chip::FlatTree;
using namespace chip::TLV;
using namespace chip::TLVMeta;
using namespace chip::TestData;

// size 1 to avoid compilers complaining about empty arrays
// (not allowed by ISO 9899:2011 6.7.6.2:
//    If the expression is a constant expression, it shall have a value greater
//    than zero.
// ). We still claim its size is 0 in empty_meta though.
const Entry<ItemInfo> _empty_item[1]                 = {};
const std::array<const Node<ItemInfo>, 1> empty_meta = { { { 0, _empty_item } } };

const Entry<ItemInfo> _FakeProtocolData[] = {
    { { AttributeTag(5), "proto5", ItemType::kDefault }, kInvalidNodeIndex },
    { { AttributeTag(16), "proto16", ItemType::kDefault }, kInvalidNodeIndex },
};

const Entry<ItemInfo> _FakeProtocols[] = {
    { { ClusterTag(0xFFFF0000), "FakeSC", ItemType::kDefault }, 1 },
    { { ClusterTag(0xFFFF0001), "FakeIM", ItemType::kDefault }, 1 },
};

const std::array<const Node<ItemInfo>, 53 + 2> fake_protocols_meta = { {
    { 2, _FakeProtocols },
    { 2, _FakeProtocolData },
} };

void TestSampleData(nlTestSuite * inSuite, const PayloadDecoderInitParams & params, const SamplePayload & data,
                    const char * expectation)
{
    chip::Decoders::PayloadDecoder<64, 128> decoder(
        PayloadDecoderInitParams(params).SetProtocol(data.protocolId).SetMessageType(data.messageType));

    decoder.StartDecoding(data.payload);

    chip::StringBuilder<4096> output_builder;

    PayloadEntry entry;
    int nesting = 0;
    while (decoder.Next(entry))
    {
        switch (entry.GetType())
        {
        case PayloadEntry::IMPayloadType::kNestingExit:
            nesting--;
            continue;
        case PayloadEntry::IMPayloadType::kAttribute:
            output_builder.AddFormat("%*sATTRIBUTE: %" PRIi32 "/%" PRIi32 "\n", nesting * 2, "", entry.GetClusterId(),
                                     entry.GetAttributeId());
            continue;
        case PayloadEntry::IMPayloadType::kCommand:
            output_builder.AddFormat("%*sCOMMAND: %" PRIi32 "/%" PRIi32 "\n", nesting * 2, "", entry.GetClusterId(),
                                     entry.GetCommandId());
            continue;
        case PayloadEntry::IMPayloadType::kEvent:
            output_builder.AddFormat("%*sEVENT: %" PRIi32 "/%" PRIi32 "\n", nesting * 2, "", entry.GetClusterId(),
                                     entry.GetEventId());
            continue;
        default:
            break;
        }

        output_builder.AddFormat("%*s%s", nesting * 2, "", entry.GetName());

        if (entry.GetType() == PayloadEntry::IMPayloadType::kNestingEnter)
        {
            nesting++;
        }
        else if (entry.GetValueText()[0] != '\0')
        {
            output_builder.AddFormat(": %s", entry.GetValueText());
        }
        else
        {
            output_builder.Add(": EMPTY");
        }
        output_builder.AddFormat("\n");
    }
    output_builder.AddMarkerIfOverflow();

    if (strcmp(output_builder.c_str(), expectation) != 0)
    {
        printf("!!!!!!!!!!!!!!!!!!! EXPECTED OUTPUT !!!!!!!!!!!!!!!!!\n");
        printf("%s\n", expectation);
        printf("!!!!!!!!!!!!!!!!!!! ACTUAL OUTPUT   !!!!!!!!!!!!!!!!!\n");
        printf("%s\n", output_builder.c_str());

        unsigned idx = 0;
        while (expectation[idx] == output_builder.c_str()[idx])
        {
            idx++;
        }
        printf("!!!!!!!!!!!!!!!!!!! DIFF LOCATION !!!!!!!!!!!!!!!!!\n");
        printf("First diff at index %u\n", idx);

        chip::StringBuilder<31> partial;
        printf("EXPECT: '%s'\n", partial.Reset().Add(expectation + idx).AddMarkerIfOverflow().c_str());
        printf("ACTUAL: '%s'\n", partial.Reset().Add(output_builder.c_str() + idx).AddMarkerIfOverflow().c_str());
    }

    NL_TEST_ASSERT(inSuite, strcmp(output_builder.c_str(), expectation) == 0);
}

void TestFullDataDecoding(nlTestSuite * inSuite, void * inContext)
{
    PayloadDecoderInitParams params;

    params.SetProtocolDecodeTree(chip::TLVMeta::protocols_meta).SetClusterDecodeTree(chip::TLVMeta::clusters_meta);

    TestSampleData(inSuite, params, secure_channel_mrp_ack, "mrp_ack: EMPTY\n");
    TestSampleData(inSuite, params, secure_channel_pkbdf_param_request,
                   "pbkdf_param_request\n"
                   "  initiator_random: hex:7C8698755B8E9866BB4FFDC27B733F3B6EF7F83D43FBE0CA6AD2B8C52C8F4236\n"
                   "  initiator_session_id: 37677\n"
                   "  passcode_id: 0\n"
                   "  has_pbkdf_parameters: false\n");
    TestSampleData(inSuite, params, secure_channel_pkbdf_param_response,
                   "pbkdf_param_response\n"
                   "  initiator_random: hex:7C8698755B8E9866BB4FFDC27B733F3B6EF7F83D43FBE0CA6AD2B8C52C8F4236\n"
                   "  responder_random: hex:A44EB3E1A751A88A32BAB59EF16EB9764C20E1A9DDBEF6EFE3F588C943C58424\n"
                   "  responder_session_id: 40168\n"
                   "  pbkdf_parameters\n"
                   "    iterations: 1000\n"
                   "    salt: hex:E8FC1E6FD0023422B3CA7ECEDD344444551C814D3D0B0EB9C096F00E8A8051B2\n");
    TestSampleData(inSuite, params, secure_channel_pase_pake1,
                   // clang-format off
                   "pase_pake1\n"
                   "  pA: hex:0422ABC7A84352850456BD4A510905FE6BB782A0863A9382550E1228020801B22EEC4102C60F80082842B9739705FCD37F134651442A41E3723DFFE0...\n"
                   // clang-format on
    );
    TestSampleData(inSuite, params, secure_channel_pase_pake2,
                   // clang-format off
                   "pase_pake2\n"
                   "  pB: hex:04B6A44A3347C6B77900A3674CA19F40F25F056F8CB344EC1B4FA7888B9E6B570B7010431C5D0BE4021FE74A96C40721765FDA6802BE8DFDF5624332...\n"
                   "  cB: hex:40E7452275E38AEBAF0E0F6FAB33A1B0CB5AEB5E824230DD40D0071DC7E55C87\n"
                   // clang-format on
    );
    TestSampleData(inSuite, params, secure_channel_pase_pake3,
                   "pase_pake3\n"
                   "  cA: hex:6008C72EDEC9D25D4A36522F0BF23058F9378EFE38CBBCCE8C6853900169BC38\n");
    TestSampleData(inSuite, params, secure_channel_status_report, "status_report: BINARY DATA\n");
    TestSampleData(inSuite, params, im_protocol_read_request,
                   "read_request\n"
                   "  attribute_requests\n"
                   "    Anonymous<>\n"
                   "      cluster_id: 49 == 'NetworkCommissioning'\n"
                   "      attribute_id: 65532 == 'featureMap'\n"
                   "    Anonymous<>\n"
                   "      endpoint_id: 0\n"
                   "      cluster_id: 48 == 'GeneralCommissioning'\n"
                   "      attribute_id: 0 == 'breadcrumb'\n"
                   "    Anonymous<>\n"
                   "      endpoint_id: 0\n"
                   "      cluster_id: 48 == 'GeneralCommissioning'\n"
                   "      attribute_id: 1 == 'basicCommissioningInfo'\n"
                   "    Anonymous<>\n"
                   "      endpoint_id: 0\n"
                   "      cluster_id: 48 == 'GeneralCommissioning'\n"
                   "      attribute_id: 2 == 'regulatoryConfig'\n"
                   "    Anonymous<>\n"
                   "      endpoint_id: 0\n"
                   "      cluster_id: 48 == 'GeneralCommissioning'\n"
                   "      attribute_id: 3 == 'locationCapability'\n"
                   "    Anonymous<>\n"
                   "      endpoint_id: 0\n"
                   "      cluster_id: 40 == 'BasicInformation'\n"
                   "      attribute_id: 2 == 'vendorID'\n"
                   "    Anonymous<>\n"
                   "      endpoint_id: 0\n"
                   "      cluster_id: 40 == 'BasicInformation'\n"
                   "      attribute_id: 4 == 'productID'\n"
                   "    Anonymous<>\n"
                   "      cluster_id: 49 == 'NetworkCommissioning'\n"
                   "      attribute_id: 3 == 'connectMaxTimeSeconds'\n"
                   "  fabric_filtered: false\n"
                   "  interaction_model_revison: 1\n");
    TestSampleData(inSuite, params, im_protocol_report_data,
                   "report_data\n"
                   "  attribute_reports\n"
                   "    Anonymous<>\n"
                   "      attribute_data\n"
                   "        data_version: 28559721\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 49 == 'NetworkCommissioning'\n"
                   "          attribute_id: 3 == 'connectMaxTimeSeconds'\n"
                   "        NetworkCommissioning::connectMaxTimeSeconds: 0\n"
                   "    Anonymous<>\n"
                   "      attribute_data\n"
                   "        data_version: 664978787\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 40 == 'BasicInformation'\n"
                   "          attribute_id: 4 == 'productID'\n"
                   "        BasicInformation::productID: 32769\n"
                   "    Anonymous<>\n"
                   "      attribute_data\n"
                   "        data_version: 664978787\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 40 == 'BasicInformation'\n"
                   "          attribute_id: 2 == 'vendorID'\n"
                   "        BasicInformation::vendorID: 65521\n"
                   "    Anonymous<>\n"
                   "      attribute_data\n"
                   "        data_version: 1414030794\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 48 == 'GeneralCommissioning'\n"
                   "          attribute_id: 3 == 'locationCapability'\n"
                   "        GeneralCommissioning::locationCapability: 2 == kIndoorOutdoor\n"
                   "    Anonymous<>\n"
                   "      attribute_data\n"
                   "        data_version: 1414030794\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 48 == 'GeneralCommissioning'\n"
                   "          attribute_id: 2 == 'regulatoryConfig'\n"
                   "        GeneralCommissioning::regulatoryConfig: 0 == kIndoor\n"
                   "    Anonymous<>\n"
                   "      attribute_data\n"
                   "        data_version: 1414030794\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 48 == 'GeneralCommissioning'\n"
                   "          attribute_id: 1 == 'basicCommissioningInfo'\n"
                   "        GeneralCommissioning::basicCommissioningInfo\n"
                   "          failSafeExpiryLengthSeconds: 60\n"
                   "          maxCumulativeFailsafeSeconds: 900\n"
                   "    Anonymous<>\n"
                   "      attribute_data\n"
                   "        data_version: 1414030794\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 48 == 'GeneralCommissioning'\n"
                   "          attribute_id: 0 == 'breadcrumb'\n"
                   "        GeneralCommissioning::breadcrumb: 0\n"
                   "    Anonymous<>\n"
                   "      attribute_data\n"
                   "        data_version: 28559721\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 49 == 'NetworkCommissioning'\n"
                   "          attribute_id: 65532 == 'featureMap'\n"
                   "        NetworkCommissioning::featureMap: 4 == kEthernetNetworkInterface\n"
                   "  suppress_response: true\n"
                   "  interaction_model_revison: 1\n");

    // Different content
    TestSampleData(inSuite, params, im_protocol_report_data_acl,
                   "report_data\n"
                   "  attribute_reports\n"
                   "    Anonymous<>\n"
                   "      attribute_data\n"
                   "        data_version: 3420147058\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 31 == 'AccessControl'\n"
                   "          attribute_id: 0 == 'acl'\n"
                   "        AccessControl::acl\n"
                   "          Anonymous<>\n"
                   "            privilege: 5 == kAdminister\n"
                   "            authMode: 2 == kCASE\n"
                   "            subjects\n"
                   "              Anonymous<>: 112233\n"
                   "            targets: NULL\n"
                   "            fabricIndex: 1\n"
                   "  suppress_response: true\n"
                   "  interaction_model_revison: 1\n");

    TestSampleData(
        inSuite, params, im_protocol_report_data_window_covering,
        "report_data\n"
        "  attribute_reports\n"
        "    Anonymous<>\n"
        "      attribute_data\n"
        "        data_version: 2054986218\n"
        "        path\n"
        "          endpoint_id: 1\n"
        "          cluster_id: 258 == 'WindowCovering'\n"
        "          attribute_id: 7 == 'configStatus'\n"
        "        WindowCovering::configStatus: 27 == kOperational | kOnlineReserved | kLiftPositionAware | kTiltPositionAware\n"
        "  suppress_response: true\n"
        "  interaction_model_revison: 1\n");

    TestSampleData(inSuite, params, im_protocol_invoke_request,
                   "invoke_request\n"
                   "  suppress_response: false\n"
                   "  timed_request: false\n"
                   "  invoke_requests\n"
                   "    Anonymous<>\n"
                   "      path\n"
                   "        endpoint_id: 1\n"
                   "        cluster_id: 6 == 'OnOff'\n"
                   "        command_id: 2 == 'Toggle'\n"
                   "      OnOff::Toggle\n"
                   "  interaction_model_revison: 1\n");

    TestSampleData(inSuite, params, im_protocol_invoke_response,
                   "invoke_response\n"
                   "  suppress_response: false\n"
                   "  invoke_responses\n"
                   "    Anonymous<>\n"
                   "      status\n"
                   "        path\n"
                   "          endpoint_id: 1\n"
                   "          cluster_id: 6 == 'OnOff'\n"
                   "          command_id: 2 == 'Toggle'\n"
                   "        status\n"
                   "          status: 0 == kSuccess\n"
                   "  interaction_model_revison: 1\n");

    TestSampleData(inSuite, params, im_protocol_invoke_request_change_channel,
                   "invoke_request\n"
                   "  suppress_response: false\n"
                   "  timed_request: false\n"
                   "  invoke_requests\n"
                   "    Anonymous<>\n"
                   "      path\n"
                   "        endpoint_id: 1\n"
                   "        cluster_id: 1284 == 'Channel'\n"
                   "        command_id: 0 == 'ChangeChannel'\n"
                   "      Channel::ChangeChannel\n"
                   "        match: \"channel name\"\n"
                   "  interaction_model_revison: 1\n");

    TestSampleData(inSuite, params, im_protocol_event_software_fault,
                   "report_data\n"
                   "  event_reports\n"
                   "    Anonymous<>\n"
                   "      event_data\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 52 == 'SoftwareDiagnostics'\n"
                   "          event_id: 0 == 'SoftwareFault'\n"
                   "        event_number: 196610\n"
                   "        priority: 1\n"
                   "        epoch_timestamp: 1690566548210\n"
                   "        SoftwareDiagnostics::SoftwareFault\n"
                   "          id: 2454952\n"
                   "          name: \"2454952\"\n"
                   "          faultRecording: hex:467269204A756C2032382031333A34393A30382032303233\n"
                   "  suppress_response: true\n"
                   "  interaction_model_revison: 1\n");

    TestSampleData(inSuite, params, im_protocol_event_multipress,
                   "report_data\n"
                   "  event_reports\n"
                   "    Anonymous<>\n"
                   "      event_data\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 59 == 'Switch'\n"
                   "          event_id: 6 == 'MultiPressComplete'\n"
                   "        event_number: 196611\n"
                   "        priority: 1\n"
                   "        epoch_timestamp: 1690566820898\n"
                   "        Switch::MultiPressComplete\n"
                   "          previousPosition: 1\n"
                   "          totalNumberOfPressesCounted: 23\n"
                   "  suppress_response: true\n"
                   "  interaction_model_revison: 1\n");
}

void TestMetaDataOnlyDecoding(nlTestSuite * inSuite, void * inContext)
{
    PayloadDecoderInitParams params;

    // NO CLUSTER DECODE TREE
    params.SetProtocolDecodeTree(chip::TLVMeta::protocols_meta);

    TestSampleData(inSuite, params, secure_channel_mrp_ack, "mrp_ack: EMPTY\n");
    TestSampleData(inSuite, params, secure_channel_pkbdf_param_request,
                   "pbkdf_param_request\n"
                   "  initiator_random: hex:7C8698755B8E9866BB4FFDC27B733F3B6EF7F83D43FBE0CA6AD2B8C52C8F4236\n"
                   "  initiator_session_id: 37677\n"
                   "  passcode_id: 0\n"
                   "  has_pbkdf_parameters: false\n");

    TestSampleData(inSuite, params, im_protocol_read_request,
                   "read_request\n"
                   "  attribute_requests\n"
                   "    Anonymous<>\n"
                   "      cluster_id: 49\n"
                   "      attribute_id: 65532\n"
                   "    Anonymous<>\n"
                   "      endpoint_id: 0\n"
                   "      cluster_id: 48\n"
                   "      attribute_id: 0\n"
                   "    Anonymous<>\n"
                   "      endpoint_id: 0\n"
                   "      cluster_id: 48\n"
                   "      attribute_id: 1\n"
                   "    Anonymous<>\n"
                   "      endpoint_id: 0\n"
                   "      cluster_id: 48\n"
                   "      attribute_id: 2\n"
                   "    Anonymous<>\n"
                   "      endpoint_id: 0\n"
                   "      cluster_id: 48\n"
                   "      attribute_id: 3\n"
                   "    Anonymous<>\n"
                   "      endpoint_id: 0\n"
                   "      cluster_id: 40\n"
                   "      attribute_id: 2\n"
                   "    Anonymous<>\n"
                   "      endpoint_id: 0\n"
                   "      cluster_id: 40\n"
                   "      attribute_id: 4\n"
                   "    Anonymous<>\n"
                   "      cluster_id: 49\n"
                   "      attribute_id: 3\n"
                   "  fabric_filtered: false\n"
                   "  interaction_model_revison: 1\n");
    TestSampleData(inSuite, params, im_protocol_report_data,
                   "report_data\n"
                   "  attribute_reports\n"
                   "    Anonymous<>\n"
                   "      attribute_data\n"
                   "        data_version: 28559721\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 49\n"
                   "          attribute_id: 3\n"
                   "        ATTRIBUTE: 49/3\n"
                   "    Anonymous<>\n"
                   "      attribute_data\n"
                   "        data_version: 664978787\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 40\n"
                   "          attribute_id: 4\n"
                   "        ATTRIBUTE: 40/4\n"
                   "    Anonymous<>\n"
                   "      attribute_data\n"
                   "        data_version: 664978787\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 40\n"
                   "          attribute_id: 2\n"
                   "        ATTRIBUTE: 40/2\n"
                   "    Anonymous<>\n"
                   "      attribute_data\n"
                   "        data_version: 1414030794\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 48\n"
                   "          attribute_id: 3\n"
                   "        ATTRIBUTE: 48/3\n"
                   "    Anonymous<>\n"
                   "      attribute_data\n"
                   "        data_version: 1414030794\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 48\n"
                   "          attribute_id: 2\n"
                   "        ATTRIBUTE: 48/2\n"
                   "    Anonymous<>\n"
                   "      attribute_data\n"
                   "        data_version: 1414030794\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 48\n"
                   "          attribute_id: 1\n"
                   "        ATTRIBUTE: 48/1\n"
                   "    Anonymous<>\n"
                   "      attribute_data\n"
                   "        data_version: 1414030794\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 48\n"
                   "          attribute_id: 0\n"
                   "        ATTRIBUTE: 48/0\n"
                   "    Anonymous<>\n"
                   "      attribute_data\n"
                   "        data_version: 28559721\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 49\n"
                   "          attribute_id: 65532\n"
                   "        ATTRIBUTE: 49/65532\n"
                   "  suppress_response: true\n"
                   "  interaction_model_revison: 1\n");

    // Different content
    TestSampleData(inSuite, params, im_protocol_report_data_acl,
                   "report_data\n"
                   "  attribute_reports\n"
                   "    Anonymous<>\n"
                   "      attribute_data\n"
                   "        data_version: 3420147058\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 31\n"
                   "          attribute_id: 0\n"
                   "        ATTRIBUTE: 31/0\n"
                   "  suppress_response: true\n"
                   "  interaction_model_revison: 1\n");
}

void TestEmptyClusterMetaDataDecode(nlTestSuite * inSuite, void * inContext)
{
    PayloadDecoderInitParams params;

    params.SetProtocolDecodeTree(chip::TLVMeta::protocols_meta).SetClusterDecodeTree(empty_meta);

    TestSampleData(inSuite, params, secure_channel_mrp_ack, "mrp_ack: EMPTY\n");
    TestSampleData(inSuite, params, im_protocol_report_data_acl,
                   "report_data\n"
                   "  attribute_reports\n"
                   "    Anonymous<>\n"
                   "      attribute_data\n"
                   "        data_version: 3420147058\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 31\n"
                   "          attribute_id: 0\n"
                   "        0x1f::ATTR(0x0)\n"              // Cluster 31, attribute 0
                   "          AnonymousTag()\n"             // List entry (acl is a list)
                   "            ContextTag(0x1): 5\n"       // privilege
                   "            ContextTag(0x2): 2\n"       // authMode
                   "            ContextTag(0x3)\n"          // subjects
                   "              AnonymousTag(): 112233\n" // List entry (subjects is a list)
                   "            ContextTag(0x4): NULL\n"    // targets
                   "            ContextTag(0xFE): 1\n"      // fabricIndex
                   "  suppress_response: true\n"
                   "  interaction_model_revison: 1\n");
}

void TestMissingDecodeData(nlTestSuite * inSuite, void * inContext)
{
    PayloadDecoderInitParams params;

    params.SetProtocolDecodeTree(empty_meta).SetClusterDecodeTree(empty_meta);

    TestSampleData(inSuite, params, secure_channel_mrp_ack, "PROTO(0x0, 0x10): UNKNOWN\n");
    TestSampleData(inSuite, params, im_protocol_report_data_acl, "PROTO(0x1, 0x5): UNKNOWN\n");
}

void TestWrongDecodeData(nlTestSuite * inSuite, void * inContext)
{
    PayloadDecoderInitParams params;

    params.SetProtocolDecodeTree(fake_protocols_meta).SetClusterDecodeTree(empty_meta);

    TestSampleData(inSuite, params, secure_channel_mrp_ack, "proto16: EMPTY\n");
    TestSampleData(inSuite, params, im_protocol_report_data_acl,
                   "proto5\n"
                   "  ContextTag(0x1)\n"
                   "    AnonymousTag()\n"
                   "      ContextTag(0x1)\n"
                   "        ContextTag(0x0): 3420147058\n"
                   "        ContextTag(0x1)\n"
                   "          ContextTag(0x2): 0\n"
                   "          ContextTag(0x3): 31\n"
                   "          ContextTag(0x4): 0\n"
                   "        ContextTag(0x2)\n"
                   "          AnonymousTag()\n"
                   "            ContextTag(0x1): 5\n"
                   "            ContextTag(0x2): 2\n"
                   "            ContextTag(0x3)\n"
                   "              AnonymousTag(): 112233\n"
                   "            ContextTag(0x4): NULL\n"
                   "            ContextTag(0xFE): 1\n"
                   "  ContextTag(0x4): true\n"
                   "  ContextTag(0xFF): 1\n");
}

void TestNestingOverflow(nlTestSuite * inSuite, void * inContext)
{
    PayloadDecoderInitParams params;
    params.SetProtocolDecodeTree(fake_protocols_meta).SetClusterDecodeTree(empty_meta);

    uint8_t data_buffer[1024];
    chip::TLV::TLVWriter writer;

    writer.Init(data_buffer, sizeof(data_buffer));

    chip::TLV::TLVType unusedType;

    // Protocols start with an anonymous tagged structure, after which lists can be of any tags
    NL_TEST_ASSERT(inSuite, writer.StartContainer(AnonymousTag(), kTLVType_Structure, unusedType) == CHIP_NO_ERROR);

    // nesting overflow here
    for (uint8_t i = 0; i < 32; i++)
    {
        NL_TEST_ASSERT(inSuite, writer.StartContainer(ContextTag(i), kTLVType_List, unusedType) == CHIP_NO_ERROR);
    }
    // Go back to 24 (still too much nesting)
    for (uint8_t i = 0; i < 8; i++)
    {
        NL_TEST_ASSERT(inSuite, writer.EndContainer(kTLVType_List) == CHIP_NO_ERROR);
    }
    for (uint8_t i = 0; i < 4; i++)
    {
        NL_TEST_ASSERT(
            inSuite, writer.StartContainer(ContextTag(static_cast<uint8_t>(i + 0x10)), kTLVType_List, unusedType) == CHIP_NO_ERROR);
    }
    for (uint8_t i = 0; i < 4; i++)
    {
        NL_TEST_ASSERT(inSuite, writer.EndContainer(kTLVType_List) == CHIP_NO_ERROR);
    }
    // Go back to 8
    for (uint8_t i = 0; i < 16; i++)
    {
        NL_TEST_ASSERT(inSuite, writer.EndContainer(kTLVType_List) == CHIP_NO_ERROR);
    }
    for (uint8_t i = 0; i < 4; i++)
    {
        NL_TEST_ASSERT(
            inSuite, writer.StartContainer(ContextTag(static_cast<uint8_t>(i + 0x20)), kTLVType_List, unusedType) == CHIP_NO_ERROR);
    }
    for (uint8_t i = 0; i < 4; i++)
    {
        NL_TEST_ASSERT(inSuite, writer.EndContainer(kTLVType_List) == CHIP_NO_ERROR);
    }
    // Go back to 4
    for (uint8_t i = 0; i < 4; i++)
    {
        NL_TEST_ASSERT(inSuite, writer.EndContainer(kTLVType_List) == CHIP_NO_ERROR);
    }
    for (uint8_t i = 0; i < 4; i++)
    {
        NL_TEST_ASSERT(
            inSuite, writer.StartContainer(ContextTag(static_cast<uint8_t>(i + 0x30)), kTLVType_List, unusedType) == CHIP_NO_ERROR);
    }
    for (uint8_t i = 0; i < 4; i++)
    {
        NL_TEST_ASSERT(inSuite, writer.EndContainer(kTLVType_List) == CHIP_NO_ERROR);
    }
    // close everything
    for (uint8_t i = 0; i < 4; i++)
    {
        NL_TEST_ASSERT(inSuite, writer.EndContainer(kTLVType_List) == CHIP_NO_ERROR);
    }
    NL_TEST_ASSERT(inSuite, writer.EndContainer(kTLVType_Structure) == CHIP_NO_ERROR);

    SamplePayload fake_payload{ chip::Protocols::InteractionModel::Id, 5, chip::ByteSpan(data_buffer, writer.GetLengthWritten()) };

    TestSampleData(inSuite, params, fake_payload,
                   "proto5\n"
                   "  ContextTag(0x0)\n"
                   "    ContextTag(0x1)\n"
                   "      ContextTag(0x2)\n"
                   "        ContextTag(0x3)\n"
                   "          ContextTag(0x4)\n"
                   "            ContextTag(0x5)\n"
                   "              ContextTag(0x6)\n"
                   "                ContextTag(0x7)\n"
                   "                  ContextTag(0x8)\n"
                   "                    ContextTag(0x9)\n"
                   "                      ContextTag(0xA)\n"
                   "                        ContextTag(0xB)\n"
                   "                          ContextTag(0xC)\n"
                   "                            ContextTag(0xD)\n"
                   "                              ContextTag(0xE)\n"
                   "                                ContextTag(0xF): NESTING DEPTH REACHED\n"
                   "                  ContextTag(0x20)\n"
                   "                    ContextTag(0x21)\n"
                   "                      ContextTag(0x22)\n"
                   "                        ContextTag(0x23)\n"
                   "          ContextTag(0x30)\n"
                   "            ContextTag(0x31)\n"
                   "              ContextTag(0x32)\n"
                   "                ContextTag(0x33)\n");
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestFullDataDecoding", TestFullDataDecoding),                     //
    NL_TEST_DEF("TestMetaDataOnlyDecoding", TestMetaDataOnlyDecoding),             //
    NL_TEST_DEF("TestEmptyClusterMetaDataDecode", TestEmptyClusterMetaDataDecode), //
    NL_TEST_DEF("TestMissingDecodeData", TestMissingDecodeData),                   //
    NL_TEST_DEF("TestWrongDecodeData", TestWrongDecodeData),                       //
    NL_TEST_DEF("TestNestingOverflow", TestNestingOverflow),                       //
    NL_TEST_SENTINEL()                                                             //
};

} // namespace

int TestDecode()
{
    nlTestSuite theSuite = { "TestDecode", sTests, nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestDecode)
