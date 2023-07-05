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
#pragma once

#include <lib/core/TLVReader.h>
#include <lib/format/FlatTreePosition.h>
#include <lib/format/tlv_meta.h>
#include <lib/support/StringBuilder.h>
#include <protocols/Protocols.h>

namespace chip {
namespace Decoders {

/// Represents an individual decoded entry for IM Payloads
/// Generally a name + value + metadata tuple, where name and value are never NULL.
class PayloadEntry
{
public:
    static constexpr uint32_t kInvalidId = 0xFFFFFFFF;
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
        VerifyOrReturnValue(mType == IMPayloadType::kAttribute, kInvalidId);
        return mSubId;
    }

    /// valid only if payload as a Command ID
    uint32_t GetCommandId() const
    {
        VerifyOrReturnValue(mType == IMPayloadType::kCommand, kInvalidId);
        return mSubId;
    }

    /// valid only if payload as a Command ID
    uint32_t GetEventId() const
    {
        VerifyOrReturnValue(mType == IMPayloadType::kEvent, kInvalidId);
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

/// Sets up decoding of some Matter data payload
class PayloadDecoderInitParams
{
public:
    using DecodeTree           = const FlatTree::Node<chip::TLVMeta::ItemInfo> *;
    PayloadDecoderInitParams() = default;

    PayloadDecoderInitParams & SetProtocol(Protocols::Id value)
    {
        mProtocol = value;
        return *this;
    }

    PayloadDecoderInitParams & SetMessageType(uint8_t value)
    {
        mMessageType = value;
        return *this;
    }

    PayloadDecoderInitParams & SetProtocolDecodeTree(DecodeTree tree, size_t s)
    {
        mProtocolTree     = tree;
        mProtocolTreeSize = s;
        return *this;
    }

    template <size_t N>
    PayloadDecoderInitParams & SetProtocolDecodeTree(const std::array<const FlatTree::Node<chip::TLVMeta::ItemInfo>, N> & a)
    {
        return SetProtocolDecodeTree(a.data(), N);
    }

    PayloadDecoderInitParams & SetClusterDecodeTree(DecodeTree tree, size_t s)
    {
        mClusterTree     = tree;
        mClusterTreeSize = s;
        return *this;
    }

    template <size_t N>
    PayloadDecoderInitParams & SetClusterDecodeTree(const std::array<const FlatTree::Node<chip::TLVMeta::ItemInfo>, N> & a)
    {
        return SetClusterDecodeTree(a.data(), N);
    }

    DecodeTree GetProtocolDecodeTree() const { return mProtocolTree; }
    size_t GetProtocolDecodeTreeSize() const { return mProtocolTreeSize; }
    DecodeTree GetClusterDecodeTree() const { return mClusterTree; }
    size_t GetClusterDecodeTreeSize() const { return mClusterTreeSize; }

    Protocols::Id GetProtocol() const { return mProtocol; }
    uint8_t GetMessageType() const { return mMessageType; }

private:
    DecodeTree mProtocolTree = nullptr;
    size_t mProtocolTreeSize = 0;

    DecodeTree mClusterTree = nullptr;
    size_t mClusterTreeSize = 0;

    Protocols::Id mProtocol = Protocols::NotSpecified;
    uint8_t mMessageType    = 0;
};

class PayloadDecoderBase
{
public:
    static constexpr size_t kMaxDecodeDepth = 16;
    using DecodePosition                    = chip::FlatTree::Position<chip::TLVMeta::ItemInfo, kMaxDecodeDepth>;

    PayloadDecoderBase(const PayloadDecoderInitParams & params, StringBuilderBase & nameBuilder, StringBuilderBase & valueBuilder);

    /// Initialize decoding from the given reader
    /// Will create a copy of the reader, however the copy will contain
    /// pointers in the original reader (so data must stay valid while Next is called)
    void StartDecoding(const TLV::TLVReader & reader);

    void StartDecoding(chip::ByteSpan data)
    {
        TLV::TLVReader reader;
        reader.Init(data);
        StartDecoding(reader);
    }

    /// Get the next decoded entry from the underlying TLV
    ///
    /// If a cluster decoder is set, then kAttribute/kCommand/kEvent are ALWAYS decoded
    /// (even if unknown tags), otherwise they will be returned as separate PayloadEntry values.
    ///
    /// Returns false when decoding finished.
    bool Next(PayloadEntry & entry);

    const TLV::TLVReader & ReadState() const { return mReader; }

private:
    enum class State
    {
        kStarting,
        kValueRead,   // reading value for Payload
        kContentRead, // reading value for IMContent (i.e. decoded attr/cmd/event)
        kDone,
    };

    /// Move to the given attribute/event/command entry.
    ///
    /// [entry] MUST be of type command/attribute/event.
    ///
    /// This call either moves to "ContentDecoding mode" if content tree is available
    /// or leaves entry unchanged if content decoding tree is not available.
    void MoveToContent(PayloadEntry & entry);

    void NextFromStarting(PayloadEntry & entry);
    void NextFromValueRead(PayloadEntry & entry);
    void NextFromContentRead(PayloadEntry & entry);

    /// Enter the container in mReader.
    ///
    /// May change entry/state in case of errors.
    ///
    /// Returns false on error (and entry is changed in that case)
    bool ReaderEnterContainer(PayloadEntry & entry);

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
    TLV::TLVReader mReader;
    DecodePosition mPayloadPosition;
    DecodePosition mIMContentPosition;
    TLV::TLVType mNestingEnters[kMaxDecodeDepth];
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
    PayloadDecoder(const PayloadDecoderInitParams & params) : PayloadDecoderBase(std::move(params), mName, mValue) {}

private:
    chip::StringBuilder<kNameBufferSize> mName;
    chip::StringBuilder<kValueBufferSize> mValue;
};

} // namespace Decoders
} // namespace chip
