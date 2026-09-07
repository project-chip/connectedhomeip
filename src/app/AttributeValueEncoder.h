/*
 *    Copyright (c) 2021-2024 Project CHIP Authors
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

#include <access/SubjectDescriptor.h>
#include <app/AttributeEncodeState.h>
#include <app/AttributeReportBuilder.h>
#include <app/ConcreteAttributePath.h>
#include <app/MessageDef/AttributeReportIBs.h>
#include <app/data-model/FabricScoped.h>
#include <app/data-model/List.h>
#include <lib/support/BitFlags.h>
#include <lib/support/BitMask.h>
#include <lib/support/TypeTraits.h>

#include <type_traits>

namespace chip {
namespace app {

/**
 * The AttributeValueEncoder is a helper class for filling report payloads into AttributeReportIBs.
 * The attribute value encoder can be initialized with a AttributeEncodeState for saving and recovering its state between encode
 * sessions (chunkings).
 *
 * When Encode returns recoverable errors (e.g. CHIP_ERROR_NO_MEMORY) the state can be used to initialize the AttributeValueEncoder
 * for future use on the same attribute path.
 */
class AttributeValueEncoder
{
private:
    // Attempt to save flash by reducing the number of instantiations of the
    // Encode methods for AttributeValueEncoder and ListEncodeHelper.  The idea
    // here is that some types actually end up encoded as other types anyway
    // (e.g. all integers are encoded are uint64_t or int64_t), so we might as
    // well avoid generating template instantiations of our methods (which have
    // extra logic) for all the different types that end up encoded the same in
    // the end.
    //
    // A type is a "base" type if it can't be treated as any other type for
    // encoding purposes.  Overloads of BaseEncodableValue can be added for
    // "non-base" types to return values of a base type.
    //
    // It's important here to not collapse together types for which
    // DataModel::Encode in fact has different behavior (e.g. enum types).
    template <typename T>
    static constexpr const T & BaseEncodableValue(const T & aArg)
    {
        return aArg;
    }
    template <typename T>
    static constexpr auto BaseEncodableValue(const BitFlags<T> & aArg)
    {
        return BaseEncodableValue(aArg.Raw());
    }
    template <typename T>
    static constexpr auto BaseEncodableValue(const BitMask<T> & aArg)
    {
        return BaseEncodableValue(aArg.Raw());
    }
    static constexpr uint64_t BaseEncodableValue(uint32_t aArg) { return aArg; }
    static constexpr uint64_t BaseEncodableValue(uint16_t aArg) { return aArg; }
    static constexpr uint64_t BaseEncodableValue(uint8_t aArg) { return aArg; }
    static constexpr int64_t BaseEncodableValue(int32_t aArg) { return aArg; }
    static constexpr int64_t BaseEncodableValue(int16_t aArg) { return aArg; }
    static constexpr int64_t BaseEncodableValue(int8_t aArg) { return aArg; }

    // Determines whether a type should be encoded as-is (if IsBaseType<T> is
    // true) or transformed to a different type by calling BaseEncodableValue()
    // on it.
    template <typename T>
    static constexpr bool IsBaseType = std::is_same_v<const T &, decltype(BaseEncodableValue(std::declval<const T &>()))>;

    template <typename T>
    static constexpr bool IsMatterEnum = std::is_enum_v<T> && DataModel::detail::HasUnknownValue<T>;

public:
    class ListEncodeHelper
    {
    public:
        ListEncodeHelper(AttributeValueEncoder & encoder) : mAttributeValueEncoder(encoder) {}

        template <typename T, std::enable_if_t<IsBaseType<T> && DataModel::IsFabricScoped<T>::value, bool> = true>
        CHIP_ERROR Encode(const T & aArg) const
        {
            VerifyOrReturnError(aArg.GetFabricIndex() != kUndefinedFabricIndex, CHIP_ERROR_INVALID_FABRIC_INDEX);

            // If we are encoding for a fabric filtered attribute read and the fabric index does not match that present in the
            // request, skip encoding this list item.
            VerifyOrReturnError(!mAttributeValueEncoder.mIsFabricFiltered ||
                                    aArg.GetFabricIndex() == mAttributeValueEncoder.AccessingFabricIndex(),
                                CHIP_NO_ERROR);
            return mAttributeValueEncoder.EncodeListItem(mCheckpoint, aArg, mAttributeValueEncoder.AccessingFabricIndex());
        }

        template <typename T,
                  std::enable_if_t<IsBaseType<T> && !DataModel::IsFabricScoped<T>::value && !IsMatterEnum<T>, bool> = true>
        CHIP_ERROR Encode(const T & aArg) const
        {
            return mAttributeValueEncoder.EncodeListItem(mCheckpoint, aArg);
        }

        // Specialization for enums to share as much code as possible in attribute encoding while
        // still doing the "unknown value" checks that might be needed..
        template <typename T, std::enable_if_t<IsBaseType<T> && IsMatterEnum<T>, bool> = true>
        CHIP_ERROR Encode(const T & aArg) const
        {
            static_assert(!DataModel::IsFabricScoped<T>::value, "How do we have a fabric-scoped enum?");
            using UnderlyingType = std::remove_cv_t<std::remove_reference_t<decltype(to_underlying(aArg))>>;
            static_assert(!std::is_same_v<UnderlyingType, T>, "Encode will call itself recursively");

            CHIP_DM_ENCODING_MAYBE_FAIL_UNKNOWN_ENUM_VALUE(aArg);

            return Encode(to_underlying(aArg));
        }

        template <typename T, std::enable_if_t<!IsBaseType<T>, bool> = true>
        CHIP_ERROR Encode(const T & aArg) const
        {
            return Encode(BaseEncodableValue(aArg));
        }

    private:
        AttributeValueEncoder & mAttributeValueEncoder;
        // Avoid calling the TLVWriter constructor for every instantiation of
        // EncodeListItem.  We treat encoding as a const operation, so either
        // have to put this on the stack (in which case it's per-instantiation),
        // or have it as mutable state.
        mutable TLV::TLVWriter mCheckpoint;
    };

    AttributeValueEncoder(AttributeReportIBs::Builder & aAttributeReportIBsBuilder, Access::SubjectDescriptor subjectDescriptor,
                          const ConcreteAttributePath & aPath, DataVersion aDataVersion, bool aIsFabricFiltered = false,
                          const AttributeEncodeState & aState = AttributeEncodeState()) :
        mAttributeReportIBsBuilder(aAttributeReportIBsBuilder),
        mSubjectDescriptor(subjectDescriptor), mPath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId),
        mDataVersion(aDataVersion), mIsFabricFiltered(aIsFabricFiltered), mEncodeState(aState)
    {}

    /**
     * Encode a single value.  This value will not be chunked; it will either be
     * entirely encoded or fail to be encoded.  Consumers are allowed to make
     * either one call to Encode or one call to EncodeList to handle a read.
     */
    template <typename T, std::enable_if_t<IsBaseType<T> && !IsMatterEnum<T>, bool> = true>
    CHIP_ERROR Encode(const T & aArg)
    {
        mTriedEncode = true;
        return EncodeAttributeReportIB(aArg);
    }

    template <typename T, std::enable_if_t<IsBaseType<T> && IsMatterEnum<T>, bool> = true>
    CHIP_ERROR Encode(const T & aArg)
    {
        using UnderlyingType = std::remove_cv_t<std::remove_reference_t<decltype(to_underlying(aArg))>>;
        static_assert(!std::is_same_v<UnderlyingType, T>, "Encode will call itself recursively");

        CHIP_DM_ENCODING_MAYBE_FAIL_UNKNOWN_ENUM_VALUE(aArg);

        return Encode(to_underlying(aArg));
    }

    template <typename T, std::enable_if_t<!IsBaseType<T>, bool> = true>
    CHIP_ERROR Encode(const T & aArg)
    {
        return Encode(BaseEncodableValue(aArg));
    }

    /**
     * Encode an explicit null value.
     */
    CHIP_ERROR EncodeNull()
    {
        // Doesn't matter what type Nullable we use here.
        return Encode(DataModel::Nullable<uint8_t>());
    }

    /**
     * Encode an explicit empty list.
     */
    CHIP_ERROR EncodeEmptyList()
    {
        // Doesn't matter what type List we use here.
        return Encode(DataModel::List<uint8_t>());
    }

    /**
     * aCallback is expected to take a const auto & argument and Encode() on it as many times as needed to encode all the list
     * elements one by one.  If any of those Encode() calls returns failure, aCallback must stop encoding and return failure.  When
     * all items are encoded aCallback is expected to return success.
     *
     * aCallback may not be called.  Consumers must not assume it will be called.
     *
     * When EncodeList returns an error, the consumers must abort the encoding, and return the exact error to the caller.
     *
     * TODO: Can we hold a error state in the AttributeValueEncoder itself?
     *
     * Consumers are allowed to make either one call to EncodeList or one call to Encode to handle a read.
     *
     */
    template <typename ListGenerator>
    CHIP_ERROR EncodeList(ListGenerator aCallback)
    {
        mTriedEncode = true;
        // Spec 10.5.4.3.1, 10.5.4.6 (Replace a list w/ Multiple IBs)
        // EmptyList acts as the beginning of the whole array type attribute report.
        // An empty list is encoded iff both mCurrentEncodingListIndex and mEncodeState.mCurrentEncodingListIndex are invalid
        // values. After encoding the empty list, mEncodeState.mCurrentEncodingListIndex and mCurrentEncodingListIndex are set to 0.
        ReturnErrorOnFailure(EnsureListStarted());
        CHIP_ERROR err = aCallback(ListEncodeHelper(*this));

        // Even if encoding list items failed, make sure we EnsureListEnded().
        // Since we encode list items atomically, in the case when we just
        // didn't fit the next item we want to make sure our list is properly
        // ended before the reporting engine starts chunking.
        EnsureListEnded();
        if (err == CHIP_NO_ERROR)
        {
            // The Encode procedure finished without any error, clear the state.
            mEncodeState.Reset();
        }
        return err;
    }

    bool TriedEncode() const { return mTriedEncode; }

    const Access::SubjectDescriptor & GetSubjectDescriptor() const { return mSubjectDescriptor; }

    /**
     * The accessing fabric index for this read or subscribe interaction.
     */
    FabricIndex AccessingFabricIndex() const { return GetSubjectDescriptor().fabricIndex; }

    /**
     * AttributeValueEncoder is a short lived object, and the state is persisted by mEncodeState and restored by constructor.
     */
    const AttributeEncodeState & GetState() const { return mEncodeState; }

private:
    // We made EncodeListItem() private, and ListEncoderHelper will expose it by Encode()
    friend class ListEncodeHelper;
    friend class TestOnlyAttributeValueEncoderAccessor;

    // Returns true if the list item should be encoded.  If it should, the
    // passed-in TLVWriter will be used to checkpoint the current state of our
    // attribute report list builder.
    bool ShouldEncodeListItem(TLV::TLVWriter & aCheckpoint);

    // Does any cleanup work needed after attempting to encode a list item.
    void PostEncodeListItem(CHIP_ERROR aEncodeStatus, const TLV::TLVWriter & aCheckpoint);

    // EncodeListItem may be given an extra FabricIndex argument as a second
    // arg, or not.  Represent that via a parameter pack (which might be
    // empty). In practice, for any given ItemType the extra arg is either there
    // or not, so we don't get more template explosion due to aExtraArgs.
    template <typename ItemType, typename... ExtraArgTypes>
    CHIP_ERROR EncodeListItem(TLV::TLVWriter & aCheckpoint, const ItemType & aItem, ExtraArgTypes &&... aExtraArgs)
    {
        if (!ShouldEncodeListItem(aCheckpoint))
        {
            return CHIP_NO_ERROR;
        }

        CHIP_ERROR err;
        if (mEncodingInitialList)
        {
            // Just encode a single item, with an anonymous tag.
            AttributeReportBuilder builder;
            err = builder.EncodeValue(mAttributeReportIBsBuilder, TLV::AnonymousTag(), aItem,
                                      std::forward<ExtraArgTypes>(aExtraArgs)...);
        }
        else
        {
            err = EncodeAttributeReportIB(aItem, std::forward<ExtraArgTypes>(aExtraArgs)...);
        }

        PostEncodeListItem(err, aCheckpoint);
        return err;
    }

    /**
     * Builds a single AttributeReportIB in AttributeReportIBs.  The caller is
     * responsible for setting up mPath correctly.
     *
     * In particular, when we are encoding a single element in the list, mPath
     * must indicate a null list index to represent an "append" operation.
     * operation.
     *
     * EncodeAttributeReportIB may be given an extra FabricIndex argument as a second
     * arg, or not.  Represent that via a parameter pack (which might be
     * empty). In practice, for any given ItemType the extra arg is either
     * there or not, so we don't get more template explosion due to aExtraArgs.
     */
    template <typename ItemType, typename... ExtraArgTypes>
    CHIP_ERROR EncodeAttributeReportIB(const ItemType & aItem, ExtraArgTypes &&... aExtraArgs)
    {
        AttributeReportBuilder builder;
        ReturnErrorOnFailure(builder.PrepareAttribute(mAttributeReportIBsBuilder, mPath, mDataVersion));
        ReturnErrorOnFailure(builder.EncodeValue(mAttributeReportIBsBuilder, TLV::ContextTag(AttributeDataIB::Tag::kData), aItem,
                                                 std::forward<ExtraArgTypes>(aExtraArgs)...));

        return builder.FinishAttribute(mAttributeReportIBsBuilder);
    }

    /**
     * EnsureListStarted sets our mCurrentEncodingListIndex to 0, and:
     *
     * * If we are just starting the list, gets us ready to encode list items.
     *
     * * If we are continuing a chunked list, guarantees that mPath.mListOp is
     *   AppendItem after it returns.
     */
    CHIP_ERROR EnsureListStarted();

    /**
     * EnsureListEnded writes out the end of the list and our attribute data IB,
     * if we were encoding our initial list
     */
    void EnsureListEnded();

    AttributeReportIBs::Builder & mAttributeReportIBsBuilder;
    const Access::SubjectDescriptor mSubjectDescriptor;
    ConcreteDataAttributePath mPath;
    DataVersion mDataVersion;
    bool mTriedEncode      = false;
    bool mIsFabricFiltered = false;
    // mEncodingInitialList is true if we're encoding a list and we have not
    // started chunking it yet, so we're encoding a single attribute report IB
    // for the whole list, not one per item.
    bool mEncodingInitialList = false;
    // mEncodedAtLeastOneListItem becomes true once we successfully encode a list item.
    bool mEncodedAtLeastOneListItem     = false;
    ListIndex mCurrentEncodingListIndex = kInvalidListIndex;
    AttributeEncodeState mEncodeState;
};

} // namespace app
} // namespace chip
