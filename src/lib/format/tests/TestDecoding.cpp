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
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVReader.h>
#include <lib/format/FlatTree.h>
#include <lib/format/FlatTreePosition.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/UnitTestRegistration.h>

#include "sample_data.h"
#include <tlv/meta/protocols_meta.h>

#include <vector>

#include <nlunit-test.h>

namespace {

using namespace chip::TestData;
using namespace chip::TLV;

using chip::TLVMeta::AttributeTag;
using chip::TLVMeta::ClusterTag;

class ByTag
{
public:
    constexpr ByTag(Tag tag) : mTag(tag) {}
    bool operator()(const chip::TLVMeta::ItemInfo & item) { return item.tag == mTag; }

private:
    const Tag mTag;
};

void ENFORCE_FORMAT(1, 2) SimpleDumpWriter(const char * aFormat, ...)
{
    va_list args;
    va_start(args, aFormat);
    vprintf(aFormat, args);
    va_end(args);
}

// Returns a null terminated string containing the current reader value
void PrettyPrintCurrentValue(TLVReader & reader, chip::MutableCharSpan out)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (reader.GetType())
    {
    case kTLVType_SignedInteger: {
        int64_t sVal;
        err = reader.Get(sVal);
        if (err != CHIP_NO_ERROR)
        {
            snprintf(out.data(), out.size(), "ERR: %" CHIP_ERROR_FORMAT, err.Format());
        }
        else
        {
            snprintf(out.data(), out.size(), "%" PRIi64, sVal);
        }
        out.data()[out.size() - 1] = '\0';
        break;
    }
    case kTLVType_UnsignedInteger: {
        uint64_t uVal;
        err = reader.Get(uVal);
        if (err != CHIP_NO_ERROR)
        {
            snprintf(out.data(), out.size(), "ERR: %" CHIP_ERROR_FORMAT, err.Format());
        }
        else
        {
            snprintf(out.data(), out.size(), "%" PRIu64, uVal);
        }
        out.data()[out.size() - 1] = '\0';
        break;
    }
    case kTLVType_Boolean: {
        bool bVal;
        err = reader.Get(bVal);
        if (err != CHIP_NO_ERROR)
        {
            snprintf(out.data(), out.size(), "ERR: %" CHIP_ERROR_FORMAT, err.Format());
        }
        else
        {
            chip::Platform::CopyString(out.data(), out.size(), bVal ? "true" : "false");
        }
        break;
    }
    case kTLVType_FloatingPointNumber: {
        double fpVal;
        err = reader.Get(fpVal);
        if (err != CHIP_NO_ERROR)
        {
            snprintf(out.data(), out.size(), "ERR: %" CHIP_ERROR_FORMAT, err.Format());
        }
        else
        {
            snprintf(out.data(), out.size(), "%lf", fpVal);
        }
        out.data()[out.size() - 1] = '\0';
        break;
    }
    case kTLVType_UTF8String: {
        const uint8_t * strbuf = nullptr;
        err                    = reader.GetDataPtr(strbuf);
        if (err != CHIP_NO_ERROR)
        {
            snprintf(out.data(), out.size(), "ERR: %" CHIP_ERROR_FORMAT, err.Format());
        }
        else
        {
            snprintf(out.data(), out.size(), "\"%-.*s\"", static_cast<int>(reader.GetLength()), strbuf);
        }
        out.data()[out.size() - 1] = '\0';
        break;
    }
    case kTLVType_ByteString: {
        const uint8_t * strbuf = nullptr;
        err                    = reader.GetDataPtr(strbuf);
        if (err != CHIP_NO_ERROR)
        {
            snprintf(out.data(), out.size(), "ERR: %" CHIP_ERROR_FORMAT, err.Format());
        }
        else
        {
            chip::Platform::CopyString(out.data(), out.size(), "hex:");
            if (out.size() > 6)
            { // space for at least one byte
                err = chip::Encoding::BytesToUppercaseHexString(strbuf, reader.GetLength(), out.data() + 4, out.size() - 4);
                if (err != CHIP_NO_ERROR)
                {
                    snprintf(out.data(), out.size(), "ERR: %" CHIP_ERROR_FORMAT, err.Format());
                }
            }
        }
        out.data()[out.size() - 1] = '\0';
        break;
    }

    case kTLVType_Null:
        chip::Platform::CopyString(out.data(), out.size(), "NULL");
        break;

    case kTLVType_NotSpecified:
        chip::Platform::CopyString(out.data(), out.size(), "Not Specified");
        break;
    default:
        chip::Platform::CopyString(out.data(), out.size(), "???");
        break;
    }
}

using DecodePosition = chip::FlatTree::Position<chip::TLVMeta::ItemInfo, 16>;

void NiceDecode(DecodePosition & position, TLVReader reader)
{
    if (reader.GetTotalLength() == 0)
    {
        return;
    }
    CHIP_ERROR err = reader.Next(kTLVType_Structure, AnonymousTag());
    if (err != CHIP_NO_ERROR)
    {
        printf("UNEXPECTED DATA: %" CHIP_ERROR_FORMAT "\n", err.Format());
        return;
    }

    auto data = position.Get();
    if (data != nullptr)
    {
        printf("%s:\n", data->name);
    }
    else
    {
        printf("UNKNOWN DATA POSITION\n");
        return;
    }

    TLVType containerType;
    std::vector<TLVType> containers;

    reader.EnterContainer(containerType);
    containers.push_back(containerType);

    while (true)
    {
        err = reader.Next();
        if (err == CHIP_END_OF_TLV)
        {
            position.Exit();
            reader.ExitContainer(containers.back());
            containers.pop_back();
            if (containers.empty())
            {
                break;
            }
            continue;
        }

        // we likely have a tag, try to find its data
        position.Enter(ByTag(reader.GetTag()));

        for (size_t i = 0; i < containers.size(); i++)
        {
            printf("  ");
        }

        // FIXME: exit logic may be complex
        auto data = position.Get();
        if (data != nullptr)
        {
            printf("%s: ", data->name);
        }
        else
        {
            printf("TAG NOT FOUND: ");
        }

        if (TLVTypeIsContainer(reader.GetType())) {
            reader.EnterContainer(containerType);
            containers.push_back(containerType);
            printf("\n");
        } else {
            // assume regular element, no entering
            char buffer[256];
            PrettyPrintCurrentValue(reader, chip::MutableCharSpan(buffer));
            printf("%s\n", buffer);
            position.Exit();
        }
    }
}

void TestSampleData(nlTestSuite * inSuite, void * inContext, const SamplePayload & data)
{
    printf("*******************************************\n");
    printf("* message type: %d\n", data.protocolId.ToFullyQualifiedSpecForm());
    printf("* message type: %d\n", data.messageType);
    printf("* DATA:\n");

    TLVReader reader;
    reader.Init(data.payload);

    Debug::Dump(reader, SimpleDumpWriter);
    printf("* NEW DATA:\n");

    DecodePosition position(chip::TLVMeta::protocols_meta.data(), chip::TLVMeta::protocols_meta.size());

    // Find the right protocol (fake cluster id)
    position.Enter(ByTag(ClusterTag(0xFFFF0000 | data.protocolId.ToFullyQualifiedSpecForm())));
    position.Enter(ByTag(AttributeTag(data.messageType)));

    NiceDecode(position, reader);
}

void TestDecode(nlTestSuite * inSuite, void * inContext)
{
    TestSampleData(inSuite, inContext, secure_channel_mrp_ack);
    TestSampleData(inSuite, inContext, secure_channel_pkbdf_param_request);
    TestSampleData(inSuite, inContext, secure_channel_pkbdf_param_response);
    TestSampleData(inSuite, inContext, secure_channel_pase_pake1);
    TestSampleData(inSuite, inContext, secure_channel_pase_pake2);
    TestSampleData(inSuite, inContext, secure_channel_pase_pake3);
    TestSampleData(inSuite, inContext, secure_channel_status_report);
    TestSampleData(inSuite, inContext, im_protocol_read_request);
    TestSampleData(inSuite, inContext, im_protocol_report_data);
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
