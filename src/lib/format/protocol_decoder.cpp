/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <lib/format/protocol_decoder.h>

#include <lib/format/FlatTree.h>
#include <lib/format/FlatTreePosition.h>

// FIXME: is this sane?
#include <tlv/meta/clusters_meta.h>
#include <tlv/meta/protocols_meta.h>

namespace chip {
namespace Decoders {

namespace {

using namespace chip::TLV;

using chip::StringBuilderBase;
using chip::TLVMeta::AttributeTag;
using chip::TLVMeta::ClusterTag;
using chip::TLVMeta::CommandTag;
using chip::TLVMeta::EventTag;
using chip::TLVMeta::ItemType;

class ByTag
{
public:
    constexpr ByTag(Tag tag) : mTag(tag) {}
    bool operator()(const chip::TLVMeta::ItemInfo & item) { return item.tag == mTag; }

private:
    const Tag mTag;
};

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

} // namespace

PayloadDecoderBase::PayloadDecoderBase(chip::Protocols::Id protocol, uint8_t messageType, StringBuilderBase & nameBuilder,
                                       StringBuilderBase & valueBuilder) :
    mProtocol(protocol),
    mMessageType(messageType), mNameBuilder(nameBuilder), mValueBuilder(valueBuilder),
    mPayloadPosition(chip::TLVMeta::protocols_meta), mIMContentPosition(chip::TLVMeta::clusters_meta)
{}

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

} // namespace Decoders
} // namespace chip
