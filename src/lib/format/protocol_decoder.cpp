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

namespace chip {
namespace Decoders {

namespace {

using namespace chip::TLV;

using chip::StringBuilderBase;
using chip::TLVMeta::AttributeTag;
using chip::TLVMeta::ClusterTag;
using chip::TLVMeta::CommandTag;
using chip::TLVMeta::ConstantValueTag;
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

CHIP_ERROR FormatCurrentValue(const TLVReader & reader, chip::StringBuilderBase & out)
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
void PrettyPrintCurrentValue(const TLVReader & reader, chip::StringBuilderBase & out, PayloadDecoderBase::DecodePosition & position)
{
    CHIP_ERROR err = FormatCurrentValue(reader, out);

    if (err != CHIP_NO_ERROR)
    {
        out.AddFormat("ERR: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

    auto data = position.Get();
    if (data == nullptr)
    {
        return;
    }

    // Report enum values in human readable form
    if (data->type == ItemType::kEnum && (reader.GetType() == kTLVType_UnsignedInteger))
    {
        uint64_t value = 0;
        VerifyOrReturn(reader.Get(value) == CHIP_NO_ERROR);

        position.Enter(ByTag(ConstantValueTag(value)));
        auto enum_data = position.Get();
        if (enum_data != nullptr)
        {
            out.Add(" == ").Add(enum_data->name);
        }
        position.Exit();
    }

    if (data->type == ItemType::kBitmap && (reader.GetType() == kTLVType_UnsignedInteger))
    {
        uint64_t value = 0;
        VerifyOrReturn(reader.Get(value) == CHIP_NO_ERROR);

        uint64_t bit = 0x01;
        bool first   = true;
        for (unsigned i = 0; i < 64; i++, bit <<= 1)
        {
            if ((value & bit) == 0)
            {
                continue;
            }

            // NOTE: this only can select individual bits;
            position.Enter(ByTag(ConstantValueTag(bit)));
            auto bitmap_data = position.Get();
            if (bitmap_data == nullptr)
            {
                position.Exit();
                continue;
            }

            // Try to pretty print the value
            if (first)
            {
                out.Add(" == ");
                first = false;
            }
            else
            {
                out.Add(" | ");
            }

            out.Add(bitmap_data->name);
            value = value & (~bit);

            position.Exit();
        }

        if (!first && value)
        {
            // Only append if some constants were found.
            out.AddFormat(" | 0x%08" PRIX64, value);
        }
    }

    out.AddMarkerIfOverflow();
}

} // namespace

PayloadDecoderBase::PayloadDecoderBase(const PayloadDecoderInitParams & params, StringBuilderBase & nameBuilder,
                                       StringBuilderBase & valueBuilder) :
    mProtocol(params.GetProtocol()),
    mMessageType(params.GetMessageType()), mNameBuilder(nameBuilder), mValueBuilder(valueBuilder),

    mPayloadPosition(params.GetProtocolDecodeTree(), params.GetProtocolDecodeTreeSize()),
    mIMContentPosition(params.GetClusterDecodeTree(), params.GetClusterDecodeTreeSize())
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
    case State::kContentRead:
        NextFromContentRead(entry);
        return true;
    case State::kDone:
        return false;
    }
    // should never happen
    return false;
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
        mNameBuilder.Reset().AddFormat("PROTO(0x%" PRIX32 ", 0x%X)", mProtocol.ToFullyQualifiedSpecForm(), mMessageType);
        mValueBuilder.Reset().Add("UNKNOWN");
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
        mValueBuilder.Reset().AddFormat("ERROR getting Anonymous Structure TLV: %" CHIP_ERROR_FORMAT, err.Format());
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
        if (mState == State::kContentRead)
        {
            mIMContentPosition.Exit();
            if (mIMContentPosition.DescendDepth() <= 1)
            {
                // Lever for actual content is 2: cluster::attr/cmd/ev
                mState = State::kValueRead;
                mPayloadPosition.Exit();
            }
        }
        else
        {
            mPayloadPosition.Exit();
        }
        CHIP_ERROR err = mReader.ExitContainer(mNestingEnters[--mCurrentNesting]);
        if (err != CHIP_NO_ERROR)
        {
            mValueBuilder.Reset().AddFormat("ERROR: %" CHIP_ERROR_FORMAT, err.Format());
            mNameBuilder.Reset().AddFormat("END CONTAINER");
            entry  = PayloadEntry::SimpleValue(mNameBuilder.c_str(), mValueBuilder.c_str());
            mState = State::kDone;
            return;
        }
    }

    if (mCurrentNesting == 0)
    {
        mState = State::kDone;
    }
}

bool PayloadDecoderBase::ReaderEnterContainer(PayloadEntry & entry)
{
    if (mCurrentNesting >= kMaxDecodeDepth)
    {
        mValueBuilder.AddFormat("NESTING DEPTH REACHED");
        mReader.GetTag().AppendTo(mNameBuilder.Reset());
        entry = PayloadEntry::SimpleValue(mNameBuilder.c_str(), mValueBuilder.c_str());
        return false;
    }

    TLVType containerType;
    CHIP_ERROR err = mReader.EnterContainer(containerType);
    if (err != CHIP_NO_ERROR)
    {
        mValueBuilder.AddFormat("ERROR entering container: %" CHIP_ERROR_FORMAT, err.Format());
        mReader.GetTag().AppendTo(mNameBuilder.Reset()); // assume enter is not done, so tag is correct
        entry  = PayloadEntry::SimpleValue(mNameBuilder.c_str(), mValueBuilder.c_str());
        mState = State::kDone;
        return false;
    }

    mNestingEnters[mCurrentNesting++] = containerType;

    return true;
}

void PayloadDecoderBase::EnterContainer(PayloadEntry & entry)
{
    // Tag fetch must be done BEFORE entering container
    chip::TLV::Tag tag = mReader.GetTag();

    VerifyOrReturn(ReaderEnterContainer(entry));

    const chip::TLVMeta::ItemInfo * data = nullptr;

    if (mState == State::kContentRead)
    {
        data = mIMContentPosition.Get();
    }
    else
    {
        mState = State::kValueRead;
        data   = mPayloadPosition.Get();
    }

    if (data == nullptr)
    {
        tag.AppendTo(mNameBuilder.Reset());
        entry = PayloadEntry::NestingEnter(mNameBuilder.c_str());
    }
    else
    {
        entry = PayloadEntry::NestingEnter(data->name);
    }
}

void PayloadDecoderBase::NextFromContentRead(PayloadEntry & entry)
{
    CHIP_ERROR err = mReader.Next();
    if (err == CHIP_END_OF_TLV)
    {
        ExitContainer(entry);
        return;
    }

    if (err != CHIP_NO_ERROR)
    {
        mValueBuilder.Reset().AddFormat("ERROR on TLV Next: %" CHIP_ERROR_FORMAT, err.Format());
        entry  = PayloadEntry::SimpleValue("TLV_ERR", mValueBuilder.c_str());
        mState = State::kDone;
        return;
    }

    if (mCurrentNesting > 0 && mNestingEnters[mCurrentNesting - 1] == kTLVType_List)
    {
        // Spec A5.3: `The members of a list may be encoded with any form of tag, including an anonymous tag.`
        // TLVMeta always uses Anonymous
        mIMContentPosition.Enter(ByTag(AnonymousTag()));
    }
    else
    {
        mIMContentPosition.Enter(ByTag(mReader.GetTag()));
    }
    auto data = mIMContentPosition.Get();

    if (data != nullptr)
    {
        if (data->type == ItemType::kProtocolBinaryData)
        {
            mIMContentPosition.Exit();
            entry = PayloadEntry::SimpleValue(data->name, "BINARY DATA");
            return;
        }
    }

    if (TLVTypeIsContainer(mReader.GetType()))
    {
        EnterContainer(entry);
        return;
    }

    PrettyPrintCurrentValue(mReader, mValueBuilder.Reset(), mIMContentPosition);
    mIMContentPosition.Exit();

    if (data == nullptr)
    {
        mReader.GetTag().AppendTo(mNameBuilder.Reset());
        entry = PayloadEntry::SimpleValue(mNameBuilder.c_str(), mValueBuilder.c_str());
        return;
    }

    entry = PayloadEntry::SimpleValue(data->name, mValueBuilder.c_str());
}

void PayloadDecoderBase::MoveToContent(PayloadEntry & entry)
{
    if (!mIMContentPosition.HasValidTree())
    {
        mPayloadPosition.Exit();
        return;
    }

    VerifyOrDie((entry.GetType() == PayloadEntry::IMPayloadType::kAttribute) ||
                (entry.GetType() == PayloadEntry::IMPayloadType::kCommand) ||
                (entry.GetType() == PayloadEntry::IMPayloadType::kEvent));

    mNameBuilder.Reset();

    mIMContentPosition.ResetToTop();
    mIMContentPosition.Enter(ByTag(ClusterTag(entry.GetClusterId())));
    auto data = mIMContentPosition.Get();
    if (data != nullptr)
    {
        mNameBuilder.AddFormat("%s::", data->name);
    }
    else
    {
        mNameBuilder.AddFormat("0x%" PRIx32 "::", entry.GetClusterId());
    }

    uint32_t id          = 0;
    const char * id_type = "UNKNOWN";

    switch (entry.GetType())
    {
    case PayloadEntry::IMPayloadType::kAttribute:
        mIMContentPosition.Enter(ByTag(AttributeTag(entry.GetAttributeId())));
        id      = entry.GetAttributeId();
        id_type = "ATTR";
        break;
    case PayloadEntry::IMPayloadType::kCommand:
        mIMContentPosition.Enter(ByTag(CommandTag(entry.GetCommandId())));
        id      = entry.GetCommandId();
        id_type = "CMD";
        break;
    case PayloadEntry::IMPayloadType::kEvent:
        mIMContentPosition.Enter(ByTag(EventTag(entry.GetEventId())));
        id      = entry.GetEventId();
        id_type = "EV";
        break;
    default:
        // never happens: verified all case above covered.
        break;
    }

    data = mIMContentPosition.Get();
    if (data != nullptr)
    {
        mNameBuilder.AddFormat("%s", data->name);
    }
    else
    {
        mNameBuilder.AddFormat("%s(0x%" PRIx32 ")", id_type, id);
    }

    if (TLVTypeIsContainer(mReader.GetType()))
    {
        mState = State::kContentRead;
        entry  = PayloadEntry::NestingEnter(mNameBuilder.c_str());
        ReaderEnterContainer(entry);
    }
    else
    {
        PrettyPrintCurrentValue(mReader, mValueBuilder.Reset(), mIMContentPosition);
        entry = PayloadEntry::SimpleValue(mNameBuilder.c_str(), mValueBuilder.c_str());

        // Can simply exit, only one value to return
        mPayloadPosition.Exit();
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

    if (err != CHIP_NO_ERROR)
    {
        mValueBuilder.Reset().AddFormat("ERROR on TLV Next: %" CHIP_ERROR_FORMAT, err.Format());
        entry  = PayloadEntry::SimpleValue("TLV_ERR", mValueBuilder.c_str());
        mState = State::kDone;
        return;
    }

    // Attempt to find information about the current tag
    mPayloadPosition.Enter(ByTag(mReader.GetTag()));
    auto data = mPayloadPosition.Get();

    // handle special types
    if (data != nullptr)
    {
        if (data->type == ItemType::kProtocolBinaryData)
        {
            mPayloadPosition.Exit();
            entry = PayloadEntry::SimpleValue(data->name, "BINARY DATA");
            return;
        }

        if (data->type == ItemType::kProtocolPayloadAttribute)
        {
            entry = PayloadEntry::AttributePayload(mClusterId, mAttributeId);
            MoveToContent(entry);
            return;
        }

        if (data->type == ItemType::kProtocolPayloadCommand)
        {
            entry = PayloadEntry::CommandPayload(mClusterId, mCommandId);
            MoveToContent(entry);
            return;
        }

        if (data->type == ItemType::kProtocolPayloadEvent)
        {
            entry = PayloadEntry::EventPayload(mClusterId, mEventId);
            MoveToContent(entry);
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
        mReader.GetTag().AppendTo(mNameBuilder.Reset());
        PrettyPrintCurrentValue(mReader, mValueBuilder.Reset(), mPayloadPosition);
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

    PrettyPrintCurrentValue(mReader, mValueBuilder.Reset(), mPayloadPosition);
    if (info != nullptr)
    {
        mValueBuilder.Add(" == '").Add(info->name).Add("'");
    }

    mPayloadPosition.Exit();
    entry = PayloadEntry::SimpleValue(data->name, mValueBuilder.c_str());
}

} // namespace Decoders
} // namespace chip
