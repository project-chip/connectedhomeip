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
#include <lib/support/UnitTestRegistration.h>

#include "sample_data.h"

// FIXME: is this sane?
#include <tlv/meta/clusters_meta.h>
#include <tlv/meta/protocols_meta.h>

#include <nlunit-test.h>

namespace {

using namespace chip::TestData;
using namespace chip::Decoders;

void TestSampleData(nlTestSuite * inSuite, void * inContext, const PayloadDecoderInitParams & params, const SamplePayload & data)
{
    chip::Decoders::PayloadDecoder<64, 128> decoder(
        PayloadDecoderInitParams(params).SetProtocol(data.protocolId).SetMessageType(data.messageType));

    decoder.StartDecoding(data.payload);

    printf("*******************************************\n");

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
            printf("%*sATTRIBUTE: %d/%d\n", nesting * 2, "", entry.GetClusterId(), entry.GetAttributeId());
            continue;
        case PayloadEntry::IMPayloadType::kCommand:
            printf("%*sCOMMAND: %d/%d\n", nesting * 2, "", entry.GetClusterId(), entry.GetCommandId());
            continue;
        case PayloadEntry::IMPayloadType::kEvent:
            printf("%*sEVENT: %d/%d\n", nesting * 2, "", entry.GetClusterId(), entry.GetEventId());
            continue;
        default:
            break;
        }

        printf("%*s%s: %s\n", nesting * 2, "", entry.GetName(), entry.GetValueText());

        if (entry.GetType() == PayloadEntry::IMPayloadType::kNestingEnter)
        {
            nesting++;
        }
    }
}

void TestDecode(nlTestSuite * inSuite, void * inContext)
{
    PayloadDecoderInitParams params;

    params.SetProtocolDecodeTree(chip::TLVMeta::protocols_meta).SetClusterDecodeTree(chip::TLVMeta::clusters_meta);

    TestSampleData(inSuite, inContext, params, secure_channel_mrp_ack);
    TestSampleData(inSuite, inContext, params, secure_channel_pkbdf_param_request);
    TestSampleData(inSuite, inContext, params, secure_channel_pkbdf_param_response);
    TestSampleData(inSuite, inContext, params, secure_channel_pase_pake1);
    TestSampleData(inSuite, inContext, params, secure_channel_pase_pake2);
    TestSampleData(inSuite, inContext, params, secure_channel_pase_pake3);
    TestSampleData(inSuite, inContext, params, secure_channel_status_report);
    TestSampleData(inSuite, inContext, params, im_protocol_read_request);
    TestSampleData(inSuite, inContext, params, im_protocol_report_data);
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestDecode", TestDecode), //
    NL_TEST_SENTINEL()                     //
};

} // namespace

int TestDecode()
{
    nlTestSuite theSuite = { "TestDecode", sTests, nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestDecode)
