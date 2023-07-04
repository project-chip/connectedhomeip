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
#include <lib/core/TLVReader.h>
#include <lib/format/protocol_decoder.h>
#include <lib/support/StringBuilder.h>
#include <lib/support/UnitTestRegistration.h>

#include <tlv/meta/clusters_meta.h>
#include <tlv/meta/protocols_meta.h>

#include <nlunit-test.h>

#include "sample_data.h"

namespace {

using namespace chip::TestData;
using namespace chip::Decoders;

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
            output_builder.AddFormat("%*sATTRIBUTE: %d/%d\n", nesting * 2, "", entry.GetClusterId(), entry.GetAttributeId());
            continue;
        case PayloadEntry::IMPayloadType::kCommand:
            output_builder.AddFormat("%*sCOMMAND: %d/%d\n", nesting * 2, "", entry.GetClusterId(), entry.GetCommandId());
            continue;
        case PayloadEntry::IMPayloadType::kEvent:
            output_builder.AddFormat("%*sEVENT: %d/%d\n", nesting * 2, "", entry.GetClusterId(), entry.GetEventId());
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
                   "pase_pake1\n"
                   "  pA: "
                   "hex:"
                   "0422ABC7A84352850456BD4A510905FE6BB782A0863A9382550E1228020801B22EEC4102C60F80082842B9739705FCD37F134651442A41E"
                   "3723DFFE0278\n");
    TestSampleData(inSuite, params, secure_channel_pase_pake2,
                   "pase_pake2\n"
                   "  pB: "
                   "hex:"
                   "04B6A44A3347C6B77900A3674CA19F40F25F056F8CB344EC1B4FA7888B9E6B570B7010431C5D0BE4021FE74A96C40721765FDA6802BE8DF"

                   "DF5624332275\n"
                   "  cB: hex:40E7452275E38AEBAF0E0F6FAB33A1B0CB5AEB5E824230DD40D0071DC7E55C87\n");
    TestSampleData(inSuite, params, secure_channel_pase_pake3,
                   "pase_pake3\n"
                   "  cA: hex:6008C72EDEC9D25D4A36522F0BF23058F9378EFE38CBBCCE8C6853900169BC38\n");
    TestSampleData(inSuite, params, secure_channel_status_report, "status_report: BINARY DATA\n");
    TestSampleData(inSuite, params, im_protocol_read_request,
                   "read_request\n"
                   "  attribute_requests\n"
                   "    []\n"
                   "      cluster_id: 49 == 'NetworkCommissioning'\n"
                   "      attribute_id: 65532 == 'featureMap'\n"
                   "    []\n"
                   "      endpoint_id: 0\n"
                   "      cluster_id: 48 == 'GeneralCommissioning'\n"
                   "      attribute_id: 0 == 'breadcrumb'\n"
                   "    []\n"
                   "      endpoint_id: 0\n"
                   "      cluster_id: 48 == 'GeneralCommissioning'\n"
                   "      attribute_id: 1 == 'basicCommissioningInfo'\n"
                   "    []\n"
                   "      endpoint_id: 0\n"
                   "      cluster_id: 48 == 'GeneralCommissioning'\n"
                   "      attribute_id: 2 == 'regulatoryConfig'\n"
                   "    []\n"
                   "      endpoint_id: 0\n"
                   "      cluster_id: 48 == 'GeneralCommissioning'\n"
                   "      attribute_id: 3 == 'locationCapability'\n"
                   "    []\n"
                   "      endpoint_id: 0\n"
                   "      cluster_id: 40 == 'BasicInformation'\n"
                   "      attribute_id: 2 == 'vendorID'\n"
                   "    []\n"
                   "      endpoint_id: 0\n"
                   "      cluster_id: 40 == 'BasicInformation'\n"
                   "      attribute_id: 4 == 'productID'\n"
                   "    []\n"
                   "      cluster_id: 49 == 'NetworkCommissioning'\n"
                   "      attribute_id: 3 == 'connectMaxTimeSeconds'\n"
                   "  fabric_filtered: false\n"
                   "  interaction_model_revison: 1\n");
    TestSampleData(inSuite, params, im_protocol_report_data,
                   "report_data\n"
                   "  attribute_reports\n"
                   "    []\n"
                   "      attribute_data\n"
                   "        data_version: 28559721\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 49 == 'NetworkCommissioning'\n"
                   "          attribute_id: 3 == 'connectMaxTimeSeconds'\n"
                   "        NetworkCommissioning::connectMaxTimeSeconds: 0\n"
                   "    []\n"
                   "      attribute_data\n"
                   "        data_version: 664978787\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 40 == 'BasicInformation'\n"
                   "          attribute_id: 4 == 'productID'\n"
                   "        BasicInformation::productID: 32769\n"
                   "    []\n"
                   "      attribute_data\n"
                   "        data_version: 664978787\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 40 == 'BasicInformation'\n"
                   "          attribute_id: 2 == 'vendorID'\n"
                   "        BasicInformation::vendorID: 65521\n"
                   "    []\n"
                   "      attribute_data\n"
                   "        data_version: 1414030794\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 48 == 'GeneralCommissioning'\n"
                   "          attribute_id: 3 == 'locationCapability'\n"
                   "        GeneralCommissioning::locationCapability: 2\n"
                   "    []\n"
                   "      attribute_data\n"
                   "        data_version: 1414030794\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 48 == 'GeneralCommissioning'\n"
                   "          attribute_id: 2 == 'regulatoryConfig'\n"
                   "        GeneralCommissioning::regulatoryConfig: 0\n"
                   "    []\n"
                   "      attribute_data\n"
                   "        data_version: 1414030794\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 48 == 'GeneralCommissioning'\n"
                   "          attribute_id: 1 == 'basicCommissioningInfo'\n"
                   "        GeneralCommissioning::basicCommissioningInfo\n"
                   "          failSafeExpiryLengthSeconds: 60\n"
                   "          maxCumulativeFailsafeSeconds: 900\n"
                   "    []\n"
                   "      attribute_data\n"
                   "        data_version: 1414030794\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 48 == 'GeneralCommissioning'\n"
                   "          attribute_id: 0 == 'breadcrumb'\n"
                   "        GeneralCommissioning::breadcrumb: 0\n"
                   "    []\n"
                   "      attribute_data\n"
                   "        data_version: 28559721\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 49 == 'NetworkCommissioning'\n"
                   "          attribute_id: 65532 == 'featureMap'\n"
                   "        NetworkCommissioning::featureMap: 4\n"
                   "  suppress_response: true\n"
                   "  interaction_model_revison: 1\n");

    // Different content
    TestSampleData(inSuite, params, im_protocol_report_data_acl,
                   "report_data\n"
                   "  attribute_reports\n"
                   "    []\n"
                   "      attribute_data\n"
                   "        data_version: 3420147058\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 31 == 'AccessControl'\n"
                   "          attribute_id: 0 == 'acl'\n"
                   "        AccessControl::acl\n"
                   "          []\n"
                   "            privilege: 5\n"
                   "            authMode: 2\n"
                   "            subjects\n"
                   "              []: 112233\n"
                   "            targets: NULL\n"
                   "            fabricIndex: 1\n"
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
                   "    []\n"
                   "      cluster_id: 49\n"
                   "      attribute_id: 65532\n"
                   "    []\n"
                   "      endpoint_id: 0\n"
                   "      cluster_id: 48\n"
                   "      attribute_id: 0\n"
                   "    []\n"
                   "      endpoint_id: 0\n"
                   "      cluster_id: 48\n"
                   "      attribute_id: 1\n"
                   "    []\n"
                   "      endpoint_id: 0\n"
                   "      cluster_id: 48\n"
                   "      attribute_id: 2\n"
                   "    []\n"
                   "      endpoint_id: 0\n"
                   "      cluster_id: 48\n"
                   "      attribute_id: 3\n"
                   "    []\n"
                   "      endpoint_id: 0\n"
                   "      cluster_id: 40\n"
                   "      attribute_id: 2\n"
                   "    []\n"
                   "      endpoint_id: 0\n"
                   "      cluster_id: 40\n"
                   "      attribute_id: 4\n"
                   "    []\n"
                   "      cluster_id: 49\n"
                   "      attribute_id: 3\n"
                   "  fabric_filtered: false\n"
                   "  interaction_model_revison: 1\n");
    TestSampleData(inSuite, params, im_protocol_report_data,
                   "report_data\n"
                   "  attribute_reports\n"
                   "    []\n"
                   "      attribute_data\n"
                   "        data_version: 28559721\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 49\n"
                   "          attribute_id: 3\n"
                   "        ATTRIBUTE: 49/3\n"
                   "    []\n"
                   "      attribute_data\n"
                   "        data_version: 664978787\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 40\n"
                   "          attribute_id: 4\n"
                   "        ATTRIBUTE: 40/4\n"
                   "    []\n"
                   "      attribute_data\n"
                   "        data_version: 664978787\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 40\n"
                   "          attribute_id: 2\n"
                   "        ATTRIBUTE: 40/2\n"
                   "    []\n"
                   "      attribute_data\n"
                   "        data_version: 1414030794\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 48\n"
                   "          attribute_id: 3\n"
                   "        ATTRIBUTE: 48/3\n"
                   "    []\n"
                   "      attribute_data\n"
                   "        data_version: 1414030794\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 48\n"
                   "          attribute_id: 2\n"
                   "        ATTRIBUTE: 48/2\n"
                   "    []\n"
                   "      attribute_data\n"
                   "        data_version: 1414030794\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 48\n"
                   "          attribute_id: 1\n"
                   "        ATTRIBUTE: 48/1\n"
                   "    []\n"
                   "      attribute_data\n"
                   "        data_version: 1414030794\n"
                   "        path\n"
                   "          endpoint_id: 0\n"
                   "          cluster_id: 48\n"
                   "          attribute_id: 0\n"
                   "        ATTRIBUTE: 48/0\n"
                   "    []\n"
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
                   "    []\n"
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

const nlTest sTests[] = {
    NL_TEST_DEF("TestFullDataDecoding", TestFullDataDecoding),         //
    NL_TEST_DEF("TestMetaDataOnlyDecoding", TestMetaDataOnlyDecoding), //
    NL_TEST_SENTINEL()                                                 //
};

} // namespace

int TestDecode()
{
    nlTestSuite theSuite = { "TestDecode", sTests, nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestDecode)
