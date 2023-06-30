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
#include <lib/support/StringBuilder.h>
#include <lib/support/UnitTestRegistration.h>

#include "sample_data.h"
#include <tlv/meta/clusters_meta.h>
#include <tlv/meta/protocols_meta.h>

#include <vector>

#include <nlunit-test.h>

namespace {

using namespace chip::TestData;
using namespace chip::TLV;

using chip::StringBuilderBase;
using chip::TLVMeta::AttributeTag;
using chip::TLVMeta::ClusterTag;
using chip::TLVMeta::CommandTag;
using chip::TLVMeta::EventTag;
using chip::TLVMeta::ItemType;

class PayloadDecoder
{
public:
    PayloadDecoder(chip::Protocols::Id protocol, uint8_t messageType);

    /// Outputs the decoded data for this protocol into [out]
    void Decode(TLVReader & reader, StringBuilderBase & out);

private:
    static constexpr size_t kMaxDecodeDepth = 16;

    using DecodePosition = chip::FlatTree::Position<chip::TLVMeta::ItemInfo, kMaxDecodeDepth>;

    const chip::Protocols::Id mProtocol;
    const uint8_t mMessageType;
    DecodePosition mPayloadPosition;
    DecodePosition mIMContentPosition;

    /// incremental state for parsing of paths
    uint32_t mClusterId   = 0;
    uint32_t mAttributeId = 0;
    uint32_t mEventId     = 0;
    uint32_t mCommandId   = 0;
};

class ByTag
{
public:
    constexpr ByTag(Tag tag) : mTag(tag) {}
    bool operator()(const chip::TLVMeta::ItemInfo & item) { return item.tag == mTag; }

private:
    const Tag mTag;
};

PayloadDecoder::PayloadDecoder(chip::Protocols::Id protocol, uint8_t messageType) :
    mProtocol(protocol), mMessageType(messageType), mPayloadPosition(chip::TLVMeta::protocols_meta),
    mIMContentPosition(chip::TLVMeta::clusters_meta)
{

    // Find the right protocol (fake cluster id)
    mPayloadPosition.Enter(ByTag(ClusterTag(0xFFFF0000 | protocol.ToFullyQualifiedSpecForm())));
    mPayloadPosition.Enter(ByTag(AttributeTag(messageType)));
}

const char * DecodeTagControl(const TLVTagControl aTagControl)
{
    switch (aTagControl)
    {
    case TLVTagControl::Anonymous:
        return "Anonymous";
    case TLVTagControl::ContextSpecific:
        return "ContextSpecific";
    case TLVTagControl::CommonProfile_2Bytes:
        return "Common2B";
    case TLVTagControl::CommonProfile_4Bytes:
        return "Common4B";
    case TLVTagControl::ImplicitProfile_2Bytes:
        return "Implicit2B";
    case TLVTagControl::ImplicitProfile_4Bytes:
        return "Implicit4B";
    case TLVTagControl::FullyQualified_6Bytes:
        return "FullyQualified6B";
    case TLVTagControl::FullyQualified_8Bytes:
        return "FullyQualified8";
    default:
        return "???";
    }
}

void FormatCurrentTag(const TLVReader & reader, chip::StringBuilderBase & out)
{
    chip::TLV::TLVTagControl tagControl = static_cast<TLVTagControl>(reader.GetControlByte() & kTLVTagControlMask);
    chip::TLV::Tag tag                  = reader.GetTag();

    if (IsProfileTag(tag))
    {
        out.AddFormat("%s(0x%x::0x%x::0x%x)", DecodeTagControl(tagControl), VendorIdFromTag(tag), ProfileNumFromTag(tag),
                      TagNumFromTag(tag));
    }
    else if (IsContextTag(tag))
    {
        out.AddFormat("%s(0x%x)", DecodeTagControl(tagControl), TagNumFromTag(tag));
    }
    else
    {
        out.AddFormat("tag[unknown]: 0x%" PRIx64, tag.RawValue());
    }
}

CHIP_ERROR FormatCurrentValue(TLVReader & reader, chip::StringBuilderBase & out)
{
    switch (reader.GetType())
    {
    case kTLVType_SignedInteger: {
        int64_t sVal;
        ReturnErrorOnFailure(reader.Get(sVal));
        out.AddFormat("%" PRIi64, sVal);
        break;
    }
    case kTLVType_UnsignedInteger: {
        uint64_t uVal;
        ReturnErrorOnFailure(reader.Get(uVal));
        out.AddFormat("%" PRIu64, uVal);
        break;
    }
    case kTLVType_Boolean: {
        bool bVal;
        ReturnErrorOnFailure(reader.Get(bVal));
        out.Add(bVal ? "true" : "false");
        break;
    }
    case kTLVType_FloatingPointNumber: {
        double fpVal;
        ReturnErrorOnFailure(reader.Get(fpVal));
        out.AddFormat("%lf", fpVal);
        break;
    }
    case kTLVType_UTF8String: {
        const uint8_t * strbuf = nullptr;
        ReturnErrorOnFailure(reader.GetDataPtr(strbuf));
        out.AddFormat("\"%-.*s\"", static_cast<int>(reader.GetLength()), strbuf);
        break;
    }
    case kTLVType_ByteString: {
        const uint8_t * strbuf = nullptr;
        ReturnErrorOnFailure(reader.GetDataPtr(strbuf));
        out.Add("hex:");
        for (uint32_t i = 0; i < reader.GetLength(); i++)
        {
            out.AddFormat("%02X", strbuf[i]);
        }
        break;
    }
    case kTLVType_Null:
        out.Add("NULL");
        break;

    case kTLVType_NotSpecified:
        out.Add("Not Specified");
        break;
    default:
        out.Add("???");
        break;
    }

    return CHIP_NO_ERROR;
}

// Returns a null terminated string containing the current reader value
void PrettyPrintCurrentValue(TLVReader & reader, chip::StringBuilderBase & out)
{
    CHIP_ERROR err = FormatCurrentValue(reader, out);

    if (err != CHIP_NO_ERROR)
    {
        out.AddFormat("ERR: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void PayloadDecoder::Decode(TLVReader & reader, StringBuilderBase & out)
{
    auto data = mPayloadPosition.Get();
    if (data == nullptr)
    {
        // do not try to decode unknown data. assume binary
        out.AddFormat("UNKNOWN PROTOCOL: 0x%X, 0x%X\n", mProtocol.ToFullyQualifiedSpecForm(), mMessageType);
        return;
    }
    out.AddFormat("%s:\n", data->name);

    if (reader.GetTotalLength() == 0)
    {
        return;
    }
    if (data->type == ItemType::kProtocolBinaryData)
    {
        out.Add("  BINARY DATA\n");
        return;
    }
    CHIP_ERROR err = reader.Next(kTLVType_Structure, AnonymousTag());
    if (err != CHIP_NO_ERROR)
    {
        out.AddFormat("  ERROR getting Anonymous Structure TLV: %" CHIP_ERROR_FORMAT "\n", err.Format());
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
            mPayloadPosition.Exit();
            reader.ExitContainer(containers.back());
            containers.pop_back();
            if (containers.empty())
            {
                break;
            }
            continue;
        }

        // we likely have a tag, try to find its data
        mPayloadPosition.Enter(ByTag(reader.GetTag()));

        for (size_t i = 0; i < containers.size(); i++)
        {
            out.Add("  ");
        }

        auto data = mPayloadPosition.Get();
        if (data != nullptr)
        {
            out.AddFormat("%s: ", data->name);
        }
        else
        {
            FormatCurrentTag(reader, out);
            out.Add(": ");
        }

        bool data_skip = false;
        if (data != nullptr)
        {
            switch (data->type)
            {
            case ItemType::kProtocolClusterId:
            case ItemType::kProtocolAttributeId:
            case ItemType::kProtocolCommandId:
            case ItemType::kProtocolEventId:
                break;
            case ItemType::kProtocolBinaryData:
                out.Add(" (binary - not parsed)");
                data_skip = true;
                break;
            case ItemType::kProtocolPayloadAttribute:
                out.Add("TODO(ATTRIBUTE DECODE)");
                data_skip = true;
                break;
            case ItemType::kProtocolPayloadCommand:
                out.Add("TODO(COMMAND DECODE)");
                data_skip = true;
                break;
            case ItemType::kProtocolPayloadEvent:
                out.Add("TODO(EVENT DECODE)");
                data_skip = true;
                break;
            default:
                break;
            }
        }

        if (data_skip)
        {
            mPayloadPosition.Exit();
        }
        else
        {
            if (TLVTypeIsContainer(reader.GetType()))
            {
                reader.EnterContainer(containerType);
                containers.push_back(containerType);
            }
            else
            {
                // assume regular element, no entering
                PrettyPrintCurrentValue(reader, out);
                mPayloadPosition.Exit();
            }
        }
        if (data != nullptr)
        {
            const chip::TLVMeta::ItemInfo * info = nullptr;
            switch (data->type)
            {
            case ItemType::kProtocolClusterId:
                reader.Get(mClusterId);
                mIMContentPosition.ResetToTop();
                mIMContentPosition.Enter(ByTag(ClusterTag(mClusterId)));
                info = mIMContentPosition.Get();
                break;
            case ItemType::kProtocolAttributeId:
                reader.Get(mAttributeId);
                mIMContentPosition.ResetToTop();
                mIMContentPosition.Enter(ByTag(ClusterTag(mClusterId)));
                mIMContentPosition.Enter(ByTag(AttributeTag(mAttributeId)));
                info = mIMContentPosition.Get();
                break;
            case ItemType::kProtocolCommandId:
                reader.Get(mCommandId);
                mIMContentPosition.ResetToTop();
                mIMContentPosition.Enter(ByTag(ClusterTag(mClusterId)));
                mIMContentPosition.Enter(ByTag(CommandTag(mCommandId)));
                info = mIMContentPosition.Get();
                break;
            case ItemType::kProtocolEventId:
                reader.Get(mEventId);
                mIMContentPosition.ResetToTop();
                mIMContentPosition.Enter(ByTag(ClusterTag(mClusterId)));
                mIMContentPosition.Enter(ByTag(EventTag(mEventId)));
                info = mIMContentPosition.Get();
                break;
            default:
                break;
            }
            if (info != nullptr)
            {
                out.AddFormat(" == %s", info->name);
            }
        }
        out.Add("\n");
    }
    out.AddMarkerIfOverflow();
}

void TestSampleData(nlTestSuite * inSuite, void * inContext, const SamplePayload & data)
{
    printf("*******************************************\n");

    TLVReader reader;
    reader.Init(data.payload);

    PayloadDecoder decoder(data.protocolId, data.messageType);

    chip::StringBuilder<4*1024> output;
    decoder.Decode(reader, output);
    printf("%s", output.c_str());
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
