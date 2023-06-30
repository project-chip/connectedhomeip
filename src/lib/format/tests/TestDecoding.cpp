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

class PayloadEntry
{
public:
    enum class IMPayloadType
    {
        kNone = 0, // generally should not be used except initial init
        kValue,    // represents a simple value to output

        kNestingEnter, // Nested struct enter. Has name, empty value
        kNestingExit,  // Nested struct exit (has no name/value)

        // Content payloads
        kAttribute,
        kCommand,
        kEvent,
    };
    PayloadEntry(const PayloadEntry &)             = default;
    PayloadEntry & operator=(const PayloadEntry &) = default;

    PayloadEntry() : mType(IMPayloadType::kNone), mName(""), mValue("") {}

    IMPayloadType GetType() const { return mType; }

    const char * GetName() const { return mName; }
    const char * GetValueText() const { return mValue; }

    /// valid only if payload is an IM Payload
    uint32_t GetClusterId() const { return mClusterId; };

    /// valid only if payload as an Attribute ID
    uint32_t GetAttributeId() const
    {
        VerifyOrReturnValue(mType == IMPayloadType::kAttribute, 0xFFFFFFFF);
        return mSubId;
    }

    /// valid only if payload as a Command ID
    uint32_t GetCommandId() const
    {
        VerifyOrReturnValue(mType == IMPayloadType::kCommand, 0xFFFFFFFF);
        return mSubId;
    }

    /// valid only if payload as a Command ID
    uint32_t GetEventId() const
    {
        VerifyOrReturnValue(mType == IMPayloadType::kEvent, 0xFFFFFFFF);
        return mSubId;
    }

    static PayloadEntry SimpleValue(const char * name, const char * value)
    {
        return PayloadEntry(IMPayloadType::kValue, name, value);
    }

    static PayloadEntry NestingEnter(const char * name) { return PayloadEntry(IMPayloadType::kNestingEnter, name, ""); }

    static PayloadEntry NestingExit() { return PayloadEntry(IMPayloadType::kNestingExit, "", ""); }

    static PayloadEntry AttributePayload(uint32_t cluster_id, uint32_t attribute_id)
    {
        PayloadEntry result(IMPayloadType::kAttribute, "ATTR_DATA", "");
        result.mClusterId = cluster_id;
        result.mSubId     = attribute_id;

        return result;
    }

    static PayloadEntry CommandPayload(uint32_t cluster_id, uint32_t command_id)
    {
        PayloadEntry result(IMPayloadType::kCommand, "COMMAND_DATA", "");
        result.mClusterId = cluster_id;
        result.mSubId     = command_id;
        return result;
    }

    static PayloadEntry EventPayload(uint32_t cluster_id, uint32_t event_id)
    {
        PayloadEntry result(IMPayloadType::kEvent, "EVENT_DATA", "");
        result.mClusterId = cluster_id;
        result.mSubId     = event_id;
        return result;
    }

private:
    PayloadEntry(IMPayloadType type, const char * name, const char * value) : mType(type), mName(name), mValue(value) {}

    IMPayloadType mType = IMPayloadType::kValue;
    const char * mName  = nullptr;
    const char * mValue = nullptr;

    uint32_t mClusterId = 0;
    uint32_t mSubId     = 0; // attribute, command or event id
};

class PayloadDecoderBase
{
public:
    PayloadDecoderBase(chip::Protocols::Id protocol, uint8_t messageType, StringBuilderBase & nameBuilder,
                       StringBuilderBase & valueBuilder);

    /// Initialize decoding from the given reader
    /// Will create a copy of the reader, however the copy will contain
    /// pointers in the original reader (so data must stay valid while Next is called)
    void StartDecoding(const TLVReader & reader);

    void StartDecoding(chip::ByteSpan data)
    {
        TLVReader reader;
        reader.Init(data);
        StartDecoding(reader);
    }

    /// Get the next decoded entry from the underlying TLV
    ///
    /// Returns false when decoding finished
    bool Next(PayloadEntry & entry);

    const TLVReader &ReadState() const { return mReader; }

private:
    static constexpr size_t kMaxDecodeDepth = 16;
    using DecodePosition                    = chip::FlatTree::Position<chip::TLVMeta::ItemInfo, kMaxDecodeDepth>;

    enum class State
    {
        kStarting,
        kValueRead,
        kDone,
    };

    void NextFromStarting(PayloadEntry & entry);
    void NextFromValueRead(PayloadEntry & entry);

    /// Returns a "nesting enter" value, making sure that
    /// nesting depth is sufficient.
    void EnterContainer(PayloadEntry & entry);

    /// Returns a "nesting exit" value, making sure that
    /// nesting depth is sufficient.
    void ExitContainer(PayloadEntry & entry);

    const chip::Protocols::Id mProtocol;
    const uint8_t mMessageType;

    StringBuilderBase & mNameBuilder;
    StringBuilderBase & mValueBuilder;

    State mState = State::kStarting;
    TLVReader mReader;
    DecodePosition mPayloadPosition;
    DecodePosition mIMContentPosition;
    TLVType mNestingEnters[kMaxDecodeDepth];
    size_t mCurrentNesting = 0;

    /// incremental state for parsing of paths
    uint32_t mClusterId   = 0;
    uint32_t mAttributeId = 0;
    uint32_t mEventId     = 0;
    uint32_t mCommandId   = 0;
};

template <size_t kNameBufferSize, size_t kValueBufferSize>
class PayloadDecoder : public PayloadDecoderBase
{
public:
    PayloadDecoder(chip::Protocols::Id protocol, uint8_t messageType) : PayloadDecoderBase(protocol, messageType, mName, mValue) {}

private:
    chip::StringBuilder<kNameBufferSize> mName;
    chip::StringBuilder<kValueBufferSize> mValue;
};

class ByTag
{
public:
    constexpr ByTag(Tag tag) : mTag(tag) {}
    bool operator()(const chip::TLVMeta::ItemInfo & item) { return item.tag == mTag; }

private:
    const Tag mTag;
};

PayloadDecoderBase::PayloadDecoderBase(chip::Protocols::Id protocol, uint8_t messageType, StringBuilderBase & nameBuilder,
                                       StringBuilderBase & valueBuilder) :
    mProtocol(protocol),
    mMessageType(messageType), mNameBuilder(nameBuilder), mValueBuilder(valueBuilder),
    mPayloadPosition(chip::TLVMeta::protocols_meta), mIMContentPosition(chip::TLVMeta::clusters_meta)
{}

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

void PayloadDecoderBase::StartDecoding(const TLVReader & reader)
{
    mReader = reader;
    mPayloadPosition.ResetToTop();
    mIMContentPosition.ResetToTop();
    mCurrentNesting = 0;
    mClusterId      = 0;
    mAttributeId    = 0;
    mEventId        = 0;
    mCommandId      = 0;
    mState          = State::kStarting;
}

bool PayloadDecoderBase::Next(PayloadEntry & entry)
{
    switch (mState)
    {
    case State::kStarting:
        NextFromStarting(entry);
        return true;
    case State::kValueRead:
        NextFromValueRead(entry);
        return true;
    case State::kDone:
        return false;
    }
}

void PayloadDecoderBase::NextFromStarting(PayloadEntry & entry)
{
    // Find the right protocol (fake cluster id)
    mPayloadPosition.Enter(ByTag(ClusterTag(0xFFFF0000 | mProtocol.ToFullyQualifiedSpecForm())));
    mPayloadPosition.Enter(ByTag(AttributeTag(mMessageType)));

    auto data = mPayloadPosition.Get();
    if (data == nullptr)
    {
        // do not try to decode unknown data. assume binary
        mNameBuilder.Reset().AddFormat("0x%X, 0x%X\n", mProtocol.ToFullyQualifiedSpecForm(), mMessageType);
        mValueBuilder.Reset().Add("UNKNOWN PROTOCOL");
        entry  = PayloadEntry::SimpleValue(mNameBuilder.c_str(), mValueBuilder.c_str());
        mState = State::kDone;
        return;
    }

    // name is known (so valid protocol)
    if (mReader.GetTotalLength() == 0)
    {
        mState = State::kDone;
        entry  = PayloadEntry::SimpleValue(data->name, "");
        return;
    }

    if (data->type == ItemType::kProtocolBinaryData)
    {
        mState = State::kDone;
        entry  = PayloadEntry::SimpleValue(data->name, "BINARY DATA");
        return;
    }

    CHIP_ERROR err = mReader.Next(kTLVType_Structure, AnonymousTag());
    if (err != CHIP_NO_ERROR)
    {
        mValueBuilder.Reset().AddFormat("ERROR getting Anonymous Structure TLV: %" CHIP_ERROR_FORMAT "\n", err.Format());
        mState = State::kDone;
        entry  = PayloadEntry::SimpleValue(data->name, mValueBuilder.c_str());
        return;
    }

    EnterContainer(entry);
}

void PayloadDecoderBase::ExitContainer(PayloadEntry & entry)
{
    entry = PayloadEntry::NestingExit();

    if (mCurrentNesting > 0)
    {
        mPayloadPosition.Exit();
        mReader.ExitContainer(mNestingEnters[--mCurrentNesting]);
    }

    if (mCurrentNesting == 0)
    {
        mState = State::kDone;
    }
}

void PayloadDecoderBase::EnterContainer(PayloadEntry & entry)
{
    if (mCurrentNesting >= kMaxDecodeDepth)
    {
        mValueBuilder.AddFormat("NESTING DEPTH REACHED");
        entry = PayloadEntry::SimpleValue(mNameBuilder.c_str(), mValueBuilder.c_str());
        return;
    }

    TLVType containerType;
    CHIP_ERROR err = mReader.EnterContainer(containerType);
    if (err != CHIP_NO_ERROR)
    {
        mValueBuilder.AddFormat("ERROR entering container: %" CHIP_ERROR_FORMAT "\n", err.Format());
        entry  = PayloadEntry::SimpleValue(mNameBuilder.c_str(), mValueBuilder.c_str());
        mState = State::kDone;
        return;
    }

    mState                            = State::kValueRead;
    mNestingEnters[mCurrentNesting++] = containerType;

    auto data = mPayloadPosition.Get();
    if (data == nullptr)
    {
        FormatCurrentTag(mReader, mNameBuilder.Reset());
        entry = PayloadEntry::NestingEnter(mNameBuilder.c_str());
    }
    else
    {
        entry = PayloadEntry::NestingEnter(data->name);
    }
}

void PayloadDecoderBase::NextFromValueRead(PayloadEntry & entry)
{
    CHIP_ERROR err = mReader.Next();
    if (err == CHIP_END_OF_TLV)
    {
        ExitContainer(entry);
        return;
    }

    // Attempt to find information about the current tag
    mPayloadPosition.Enter(ByTag(mReader.GetTag()));
    auto data = mPayloadPosition.Get();

    // handle special types
    if (data != nullptr) {
        if (data->type == ItemType::kProtocolBinaryData)
        {
            mPayloadPosition.Exit();
            entry = PayloadEntry::SimpleValue(data->name, "BINARY DATA");
            return;
        }

        if (data->type == ItemType::kProtocolPayloadAttribute)
        {
            mPayloadPosition.Exit();
            entry = PayloadEntry::AttributePayload(mClusterId, mAttributeId);
            return;
        }

        if (data->type == ItemType::kProtocolPayloadCommand)
        {
            mPayloadPosition.Exit();
            entry = PayloadEntry::CommandPayload(mClusterId, mCommandId);
            return;
        }

        if (data->type == ItemType::kProtocolPayloadEvent)
        {
            mPayloadPosition.Exit();
            entry = PayloadEntry::EventPayload(mClusterId, mEventId);
            return;
        }
    }

    if (TLVTypeIsContainer(mReader.GetType()))
    {
        EnterContainer(entry);
        return;
    }

    if (data == nullptr)
    {
        FormatCurrentTag(mReader, mNameBuilder.Reset());
        PrettyPrintCurrentValue(mReader, mValueBuilder.Reset());
        entry = PayloadEntry::SimpleValue(mNameBuilder.c_str(), mValueBuilder.c_str());
        mPayloadPosition.Exit();
        return;
    }

    // at this point, data is "simple data" or "simple data with meaning"

    const chip::TLVMeta::ItemInfo * info = nullptr;
    switch (data->type)
    {
    case ItemType::kProtocolClusterId:
        mReader.Get(mClusterId);
        mIMContentPosition.ResetToTop();
        mIMContentPosition.Enter(ByTag(ClusterTag(mClusterId)));
        info = mIMContentPosition.Get();
        break;
    case ItemType::kProtocolAttributeId:
        mReader.Get(mAttributeId);
        mIMContentPosition.ResetToTop();
        mIMContentPosition.Enter(ByTag(ClusterTag(mClusterId)));
        mIMContentPosition.Enter(ByTag(AttributeTag(mAttributeId)));
        info = mIMContentPosition.Get();
        break;
    case ItemType::kProtocolCommandId:
        mReader.Get(mCommandId);
        mIMContentPosition.ResetToTop();
        mIMContentPosition.Enter(ByTag(ClusterTag(mClusterId)));
        mIMContentPosition.Enter(ByTag(CommandTag(mCommandId)));
        info = mIMContentPosition.Get();
        break;
    case ItemType::kProtocolEventId:
        mReader.Get(mEventId);
        mIMContentPosition.ResetToTop();
        mIMContentPosition.Enter(ByTag(ClusterTag(mClusterId)));
        mIMContentPosition.Enter(ByTag(EventTag(mEventId)));
        info = mIMContentPosition.Get();
        break;
    default:
        break;
    }

    PrettyPrintCurrentValue(mReader, mValueBuilder.Reset());
    if (info != nullptr)
    {
        mValueBuilder.Add(" == '").Add(info->name).Add("'");
    }

    mPayloadPosition.Exit();
    entry = PayloadEntry::SimpleValue(data->name, mValueBuilder.c_str());
}

void TestSampleData(nlTestSuite * inSuite, void * inContext, const SamplePayload & data)
{
    printf("*******************************************\n");

    TLVReader reader;
    reader.Init(data.payload);

    PayloadDecoder<64, 128> decoder(data.protocolId, data.messageType);

    decoder.StartDecoding(data.payload);

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
