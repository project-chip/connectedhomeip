/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
#include <app/ConcreteAttributePath.h>
#include <app/MessageDef/AttributeReportIBs.h>
#include <app/data-model/DecodableList.h>
#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>
#include <app/data-model/FabricScoped.h>
#include <app/data-model/List.h> // So we can encode lists
#include <app/data-model/TagBoundEncoder.h>
#include <app/util/basic-types.h>
#include <lib/core/Optional.h>
#include <lib/core/TLV.h>
#include <lib/support/logging/CHIPLogging.h>

/**
 * Callback class that clusters can implement in order to interpose custom
 * attribute-handling logic.  An AttributeAccessInterface instance is associated
 * with some specific cluster.  A single instance may be used for a specific
 * endpoint or for all endpoints.
 *
 * Instances of AttributeAccessInterface that are registered via
 * registerAttributeAccessOverride will be consulted before taking the normal
 * attribute access codepath and can use that codepath as a fallback if desired.
 */
namespace chip {
namespace app {

/**
 * The AttributeReportBuilder is a helper class for filling a single report in AttributeReportIBs.
 *
 * Possible usage of AttributeReportBuilder might be:
 *
 * AttributeReportBuilder builder;
 * ReturnErrorOnFailure(builder.PrepareAttribute(...));
 * ReturnErrorOnFailure(builder.Encode(...));
 * ReturnErrorOnFailure(builder.FinishAttribute());
 */
class AttributeReportBuilder
{
public:
    /**
     * PrepareAttribute encodes the "header" part of an attribute report including the path and data version.
     * Path will be encoded according to section 10.5.4.3.1 in the spec.
     * Note: Only append is supported currently (encode a null list index), other operations won't encode a list index in the
     * attribute path field.
     * TODO: Add support for encoding a single element in the list (path with a valid list index).
     */
    CHIP_ERROR PrepareAttribute(AttributeReportIBs::Builder & aAttributeReportIBs, const ConcreteDataAttributePath & aPath,
                                DataVersion aDataVersion);

    /**
     * FinishAttribute encodes the "footer" part of an attribute report (it closes the containers opened in PrepareAttribute)
     */
    CHIP_ERROR FinishAttribute(AttributeReportIBs::Builder & aAttributeReportIBs);

    /**
     * EncodeValue encodes the value field of the report, it should be called exactly once.
     */
    template <typename T, std::enable_if_t<!DataModel::IsFabricScoped<T>::value, bool> = true, typename... Ts>
    CHIP_ERROR EncodeValue(AttributeReportIBs::Builder & aAttributeReportIBs, TLV::Tag tag, T && item, Ts &&... aArgs)
    {
        return DataModel::Encode(*(aAttributeReportIBs.GetAttributeReport().GetAttributeData().GetWriter()), tag, item,
                                 std::forward<Ts>(aArgs)...);
    }

    template <typename T, std::enable_if_t<DataModel::IsFabricScoped<T>::value, bool> = true, typename... Ts>
    CHIP_ERROR EncodeValue(AttributeReportIBs::Builder & aAttributeReportIBs, TLV::Tag tag, FabricIndex accessingFabricIndex,
                           T && item, Ts &&... aArgs)
    {
        return DataModel::EncodeForRead(*(aAttributeReportIBs.GetAttributeReport().GetAttributeData().GetWriter()), tag,
                                        accessingFabricIndex, item, std::forward<Ts>(aArgs)...);
    }
};

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
                                    aArg.GetFabricIndex() == mAttributeValueEncoder.mAccessingFabricIndex,
                                CHIP_NO_ERROR);
            return mAttributeValueEncoder.EncodeListItem(mAttributeValueEncoder.mAccessingFabricIndex, std::forward<T>(aArg));
        }

        template <typename T, std::enable_if_t<!DataModel::IsFabricScoped<T>::value, bool> = true>
        CHIP_ERROR Encode(T && aArg) const
        {
            return mAttributeValueEncoder.EncodeListItem(std::forward<T>(aArg));
        }

    private:
        AttributeValueEncoder & mAttributeValueEncoder;
    };

    class AttributeEncodeState
    {
    public:
        AttributeEncodeState() : mAllowPartialData(false), mCurrentEncodingListIndex(kInvalidListIndex) {}
        bool AllowPartialData() const { return mAllowPartialData; }

    private:
        friend class AttributeValueEncoder;
        /**
         * When an attempt to encode an attribute returns an error, the buffer may contain tailing dirty data
         * (since the put was aborted).  The report engine normally rolls back the buffer to right before encoding
         * of the attribute started on errors.
         *
         * When chunking a list, EncodeListItem will atomically encode list items, ensuring that the
         * state of the buffer is valid to send (i.e. contains no trailing garbage), and return an error
         * if the list doesn't entirely fit.  In this situation, mAllowPartialData is set to communicate to the
         * report engine that it should not roll back the list items.
         *
         * TODO: There might be a better name for this variable.
         */
        bool mAllowPartialData = false;
        /**
         * If set to kInvalidListIndex, indicates that we have not encoded any data for the list yet and
         * need to start by encoding an empty list before we start encoding any list items.
         *
         * When set to a valid ListIndex value, indicates the index of the next list item that needs to be
         * encoded (i.e. the count of items encoded so far).
         */
        ListIndex mCurrentEncodingListIndex = kInvalidListIndex;
    };

    AttributeValueEncoder(AttributeReportIBs::Builder & aAttributeReportIBsBuilder, FabricIndex aAccessingFabricIndex,
                          const ConcreteAttributePath & aPath, DataVersion aDataVersion, bool aIsFabricFiltered = false,
                          const AttributeEncodeState & aState = AttributeEncodeState()) :
        mAttributeReportIBsBuilder(aAttributeReportIBsBuilder),
        mAccessingFabricIndex(aAccessingFabricIndex), mPath(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId),
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
            mEncodeState = AttributeEncodeState();
        }
        return err;
    }

    bool TriedEncode() const { return mTriedEncode; }

    /**
     * The accessing fabric index for this read or subscribe interaction.
     */
    FabricIndex AccessingFabricIndex() const { return mAccessingFabricIndex; }

    /**
     * AttributeValueEncoder is a short lived object, and the state is persisted by mEncodeState and restored by constructor.
     */
    const AttributeEncodeState & GetState() const { return mEncodeState; }

private:
    // We made EncodeListItem() private, and ListEncoderHelper will expose it by Encode()
    friend class ListEncodeHelper;

    template <typename... Ts>
    CHIP_ERROR EncodeListItem(Ts &&... aArgs)
    {
        // EncodeListItem must be called after EnsureListStarted(), thus mCurrentEncodingListIndex and
        // mEncodeState.mCurrentEncodingListIndex are not invalid values.
        if (mCurrentEncodingListIndex < mEncodeState.mCurrentEncodingListIndex)
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
        mEncodeState.mCurrentEncodingListIndex++;
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

    bool mTriedEncode = false;
    AttributeReportIBs::Builder & mAttributeReportIBsBuilder;
    const FabricIndex mAccessingFabricIndex;
    ConcreteDataAttributePath mPath;
    DataVersion mDataVersion;
    bool mIsFabricFiltered = false;
    // mEncodingInitialList is true if we're encoding a list and we have not
    // started chunking it yet, so we're encoding a single attribute report IB
    // for the whole list, not one per item.
    bool mEncodingInitialList = false;
    AttributeEncodeState mEncodeState;
    ListIndex mCurrentEncodingListIndex = kInvalidListIndex;
};

class AttributeValueDecoder
{
public:
    AttributeValueDecoder(TLV::TLVReader & aReader, const Access::SubjectDescriptor & aSubjectDescriptor) :
        mReader(aReader), mSubjectDescriptor(aSubjectDescriptor)
    {}

    template <typename T, typename std::enable_if_t<!DataModel::IsFabricScoped<T>::value, bool> = true>
    CHIP_ERROR Decode(T & aArg)
    {
        mTriedDecode = true;
        return DataModel::Decode(mReader, aArg);
    }

    template <typename T, typename std::enable_if_t<DataModel::IsFabricScoped<T>::value, bool> = true>
    CHIP_ERROR Decode(T & aArg)
    {
        mTriedDecode = true;
        // The WriteRequest comes with no fabric index, this will happen when receiving a write request on a PASE session before
        // AddNOC.
        VerifyOrReturnError(AccessingFabricIndex() != kUndefinedFabricIndex, CHIP_IM_GLOBAL_STATUS(UnsupportedAccess));
        ReturnErrorOnFailure(DataModel::Decode(mReader, aArg));
        aArg.SetFabricIndex(AccessingFabricIndex());
        return CHIP_NO_ERROR;
    }

    /**
     * Is the next element of the stream NULL.
     */
    bool WillDecodeNull() const { return mReader.GetType() == TLV::kTLVType_Null; }

    bool TriedDecode() const { return mTriedDecode; }

    /**
     * The accessing fabric index for this write interaction.
     */
    FabricIndex AccessingFabricIndex() const { return mSubjectDescriptor.fabricIndex; }

    /**
     * The accessing subject descriptor for this write interaction.
     */
    const Access::SubjectDescriptor & GetSubjectDescriptor() const { return mSubjectDescriptor; }

private:
    TLV::TLVReader & mReader;
    bool mTriedDecode = false;
    const Access::SubjectDescriptor mSubjectDescriptor;
};

class AttributeAccessInterface
{
public:
    /**
     * aEndpointId can be Missing to indicate that this object is meant to be
     * used with all endpoints.
     */
    AttributeAccessInterface(Optional<EndpointId> aEndpointId, ClusterId aClusterId) :
        mEndpointId(aEndpointId), mClusterId(aClusterId)
    {}
    virtual ~AttributeAccessInterface() {}

    /**
     * Callback for reading attributes.
     *
     * @param [in] aPath indicates which exact data is being read.
     * @param [in] aEncoder the AttributeValueEncoder to use for encoding the
     *             data.
     *
     * The implementation can do one of three things:
     *
     * 1) Return a failure.  This is treated as a failed read and the error is
     *    returned to the client, by converting it to a StatusIB.
     * 2) Return success and attempt to encode data using aEncoder.  The data is
     *    returned to the client.
     * 3) Return success and not attempt to encode any data using aEncoder.  In
     *    this case, Ember attribute access will happen for the read. This may
     *    involve reading from the attribute store or external attribute
     *    callbacks.
     */
    virtual CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) = 0;

    /**
     * Callback for writing attributes.
     *
     * @param [in] aPath indicates which exact data is being written.
     * @param [in] aDecoder the AttributeValueDecoder to use for decoding the
     *             data.
     *
     * The implementation can do one of three things:
     *
     * 1) Return a failure.  This is treated as a failed write and the error is
     *    sent to the client, by converting it to a StatusIB.
     * 2) Return success and attempt to decode from aDecoder.  This is
     *    treated as a successful write.
     * 3) Return success and not attempt to decode from aDecoder.  In
     *    this case, Ember attribute access will happen for the write. This may
     *    involve writing to the attribute store or external attribute
     *    callbacks.
     */
    virtual CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) { return CHIP_NO_ERROR; }

    /**
     * Indicates the start of a series of list operations. This function will be called before the first Write operation of a series
     * of consequence attribute data of the same attribute.
     *
     * 1) This function will be called if the client tries to set a nullable list attribute to null.
     * 2) This function will only be called once for a series of consequent attribute data (regardless the kind of list operation)
     * of the same attribute.
     *
     * @param [in] aPath indicates the path of the modified list.
     */
    virtual void OnListWriteBegin(const ConcreteAttributePath & aPath) {}

    /**
     * Indicates the end of a series of list operations. This function will be called after the last Write operation of a series
     * of consequence attribute data of the same attribute.
     *
     * 1) This function will be called if the client tries to set a nullable list attribute to null.
     * 2) This function will only be called once for a series of consequent attribute data (regardless the kind of list operation)
     * of the same attribute.
     * 3) When aWriteWasSuccessful is true, the data written must be consistent or the list is untouched.
     *
     * @param [in] aPath indicates the path of the modified list
     * @param [in] aWriteWasSuccessful indicates whether the delivered list is complete.
     *
     */
    virtual void OnListWriteEnd(const ConcreteAttributePath & aPath, bool aWriteWasSuccessful) {}

    /**
     * Mechanism for keeping track of a chain of AttributeAccessInterfaces.
     */
    void SetNext(AttributeAccessInterface * aNext) { mNext = aNext; }
    AttributeAccessInterface * GetNext() const { return mNext; }

    /**
     * Check whether a this AttributeAccessInterface is relevant for a
     * particular endpoint+cluster.  An AttributeAccessInterface will be used
     * for a read from a particular cluster only when this function returns
     * true.
     */
    bool Matches(EndpointId aEndpointId, ClusterId aClusterId) const
    {
        return (!mEndpointId.HasValue() || mEndpointId.Value() == aEndpointId) && mClusterId == aClusterId;
    }

    /**
     * Check whether an AttributeAccessInterface is relevant for a particular
     * specific endpoint.  This is used to clean up overrides registered for an
     * endpoint that becomes disabled.
     */
    bool MatchesEndpoint(EndpointId aEndpointId) const { return mEndpointId.HasValue() && mEndpointId.Value() == aEndpointId; }

    /**
     * Check whether another AttributeAccessInterface wants to handle the same set of
     * attributes as we do.
     */
    bool Matches(const AttributeAccessInterface & aOther) const
    {
        return mClusterId == aOther.mClusterId &&
            (!mEndpointId.HasValue() || !aOther.mEndpointId.HasValue() || mEndpointId.Value() == aOther.mEndpointId.Value());
    }

private:
    Optional<EndpointId> mEndpointId;
    ClusterId mClusterId;
    AttributeAccessInterface * mNext = nullptr;
};

} // namespace app
} // namespace chip
