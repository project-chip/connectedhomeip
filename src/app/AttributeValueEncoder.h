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
public:
    class ListEncodeHelper
    {
    public:
        ListEncodeHelper(AttributeValueEncoder & encoder) : mAttributeValueEncoder(encoder) {}

        template <typename T, std::enable_if_t<DataModel::IsFabricScoped<T>::value, bool> = true>
        CHIP_ERROR Encode(T && aArg) const
        {
            VerifyOrReturnError(aArg.GetFabricIndex() != kUndefinedFabricIndex, CHIP_ERROR_INVALID_FABRIC_INDEX);

            // If we are encoding for a fabric filtered attribute read and the fabric index does not match that present in the
            // request, skip encoding this list item.
            VerifyOrReturnError(!mAttributeValueEncoder.mIsFabricFiltered ||
                                    aArg.GetFabricIndex() == mAttributeValueEncoder.AccessingFabricIndex(),
                                CHIP_NO_ERROR);
            return mAttributeValueEncoder.EncodeListItem(mAttributeValueEncoder.AccessingFabricIndex(), std::forward<T>(aArg));
        }

        template <typename T, std::enable_if_t<!DataModel::IsFabricScoped<T>::value, bool> = true>
        CHIP_ERROR Encode(T && aArg) const
        {
            return mAttributeValueEncoder.EncodeListItem(std::forward<T>(aArg));
        }

    private:
        AttributeValueEncoder & mAttributeValueEncoder;
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
    template <typename... Ts>
    CHIP_ERROR Encode(Ts &&... aArgs)
    {
        mTriedEncode = true;
        return EncodeAttributeReportIB(std::forward<Ts>(aArgs)...);
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
     * TODO: Can we hold a error state in the AttributeValueEncoder itself so functions in ember-compatibility-functions don't have
     * to rely on the above assumption?
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

    template <typename... Ts>
    CHIP_ERROR EncodeListItem(Ts &&... aArgs)
    {
        // EncodeListItem must be called after EnsureListStarted(), thus mCurrentEncodingListIndex and
        // mEncodeState.mCurrentEncodingListIndex are not invalid values.
        if (mCurrentEncodingListIndex < mEncodeState.CurrentEncodingListIndex())
        {
            // We have encoded this element in previous chunks, skip it.
            mCurrentEncodingListIndex++;
            return CHIP_NO_ERROR;
        }

        TLV::TLVWriter backup;
        mAttributeReportIBsBuilder.Checkpoint(backup);

        CHIP_ERROR err;
        if (mEncodingInitialList)
        {
            // Just encode a single item, with an anonymous tag.
            AttributeReportBuilder builder;
            err = builder.EncodeValue(mAttributeReportIBsBuilder, TLV::AnonymousTag(), std::forward<Ts>(aArgs)...);
        }
        else
        {
            err = EncodeAttributeReportIB(std::forward<Ts>(aArgs)...);
        }
        if (err != CHIP_NO_ERROR)
        {
            // For list chunking, ReportEngine should not rollback the buffer when CHIP_ERROR_NO_MEMORY or similar error occurred.
            // However, the error might be raised in the middle of encoding procedure, then the buffer may contain partial data,
            // unclosed containers etc. This line clears all possible partial data and makes EncodeListItem is atomic.
            mAttributeReportIBsBuilder.Rollback(backup);
            return err;
        }

        mCurrentEncodingListIndex++;
        mEncodeState.SetCurrentEncodingListIndex(mCurrentEncodingListIndex);
        mEncodedAtLeastOneListItem = true;
        return CHIP_NO_ERROR;
    }

    /**
     * Builds a single AttributeReportIB in AttributeReportIBs.  The caller is
     * responsible for setting up mPath correctly.
     *
     * In particular, when we are encoding a single element in the list, mPath
     * must indicate a null list index to represent an "append" operation.
     * operation.
     */
    template <typename... Ts>
    CHIP_ERROR EncodeAttributeReportIB(Ts &&... aArgs)
    {
        AttributeReportBuilder builder;
        ReturnErrorOnFailure(builder.PrepareAttribute(mAttributeReportIBsBuilder, mPath, mDataVersion));
        ReturnErrorOnFailure(builder.EncodeValue(mAttributeReportIBsBuilder, TLV::ContextTag(AttributeDataIB::Tag::kData),
                                                 std::forward<Ts>(aArgs)...));

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
