/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/ClusterInfo.h>
#include <app/ConcreteAttributePath.h>
#include <app/MessageDef/AttributeReportIBs.h>
#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>
#include <app/data-model/List.h> // So we can encode lists
#include <app/data-model/TagBoundEncoder.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/Optional.h>

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
 * The AttributeReportEncoder is a helper class for filling a single report in AttributeReportIBs.
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
     * TODO: Add support of encode a single element in the list (path with a valid list index).
     */
    CHIP_ERROR PrepareAttribute(AttributeReportIBs::Builder & aAttributeReportIBs, const ConcreteDataAttributePath & aPath,
                                DataVersion aDataVersion);

    /**
     * FinishAttribute encodes the "footer" part of an attribute report (it closes the containers opened in PrepareAttribute)
     */
    CHIP_ERROR FinishAttribute();

    /**
     * EncodeValue encodes the value field of the report, it should be called exactly once.
     */
    template <typename... Ts>
    CHIP_ERROR EncodeValue(Ts... aArgs)
    {
        return DataModel::Encode(*mAttributeDataIBBuilder.GetWriter(), TLV::ContextTag(to_underlying(AttributeDataIB::Tag::kData)),
                                 std::forward<Ts>(aArgs)...);
    }

private:
    AttributeReportIB::Builder mAttributeReportIBBuilder;
    AttributeDataIB::Builder mAttributeDataIBBuilder;
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

        template <typename... Ts>
        CHIP_ERROR Encode(Ts... aArgs) const
        {
            return mAttributeValueEncoder.EncodeListItem(std::forward<Ts>(aArgs)...);
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
         * When TLVWriter failed to encode some field, the buffer may contain tailing dirty data (since the put was aborted). Thus
         * report engine may revert the buffer.
         * EncodeListItem will encode an AttributeReportIB atomicly, and we do want return a partial list to the client (chunking),
         * in this case, mAllowPartialData is set to tell the report engine not to revert the buffer.
         */
        bool mAllowPartialData = false;
        /**
         * When a encoding session is interrupted by insufficient buffer, mCurrentEncodingListIndex will store the next item to be
         * encoded. By default, an invalid list index means we have not started encoding the list and we need to encode a empty list
         * first.
         */
        ListIndex mCurrentEncodingListIndex = kInvalidListIndex;
    };

    AttributeValueEncoder(AttributeReportIBs::Builder & aAttributeReportIBsBuilder, FabricIndex aAccessingFabricIndex,
                          const ConcreteAttributePath & aPath, DataVersion aDataVersion,
                          const AttributeEncodeState & aState = AttributeEncodeState()) :
        mAttributeReportIBsBuilder(aAttributeReportIBsBuilder),
        mAccessingFabricIndex(aAccessingFabricIndex), mPath(aPath), mDataVersion(aDataVersion), mEncodeState(aState)
    {}

    /**
     * Encode builds a single AttributeReportIB in AttributeReportIBs.
     * When we are enciding a single element in the list, the actual path in the report contains a null list index as "append"
     * operation.
     */
    template <typename... Ts>
    CHIP_ERROR Encode(Ts... aArgs)
    {
        mTriedEncode = true;
        AttributeReportBuilder builder;

        ReturnErrorOnFailure(builder.PrepareAttribute(
            mAttributeReportIBsBuilder,
            ConcreteDataAttributePath(mPath.mEndpointId, mPath.mClusterId, mPath.mAttributeId,
                                      // We only use the two operations here to indicate if we need to encode a null list index
                                      mCurrentEncodingListIndex == kInvalidListIndex
                                          ? ConcreteDataAttributePath::ListOperation::NotList
                                          : ConcreteDataAttributePath::ListOperation::AppendItem,
                                      mCurrentEncodingListIndex),
            mDataVersion));
        ReturnErrorOnFailure(builder.EncodeValue(std::forward<Ts>(aArgs)...));

        return builder.FinishAttribute();
    }

    /**
     * aCallback is expected to take a const TagBoundEncoder& argument and
     * Encode() on it as many times as needed to encode all the list elements
     * one by one.  If any of those Encode() calls returns failure, aCallback
     * must stop encoding and return failure.  When all items are encoded
     * aCallback is expected to return success.
     *
     * aCallback may not be called.  Consumers must not assume it will be
     * called.
     */
    template <typename ListGenerator>
    CHIP_ERROR EncodeList(ListGenerator aCallback)
    {
        mTriedEncode = true;
        // Spec 10.5.4.3.1, 10.5.4.6 (Replace a list w/ Multiple IBs)
        // EmptyList acts as the beginning of the whole array type attribute report.
        // An empty list is encoded iff both mCurrentEncodingListIndex and mEncodeState.mCurrentEncodingListIndex are invalid
        // values. After encoding the empty list, mEncodeState.mCurrentEncodingListIndex and mCurrentEncodingListIndex are set to 0.
        ReturnErrorOnFailure(EncodeEmptyList());
        ReturnErrorOnFailure(aCallback(ListEncodeHelper(*this)));
        // The Encode procedure finished without any error, clear the state.
        mEncodeState = AttributeEncodeState();
        return CHIP_NO_ERROR;
    }

    bool TriedEncode() const { return mTriedEncode; }

    /**
     * The accessing fabric index for this read or subscribe interaction.
     */
    FabricIndex AccessingFabricIndex() const { return mAccessingFabricIndex; }

    /**
     * AttributeValueEncoder is a short lived object, and the state is presisted by mEncodeState and restored by constructor.
     */
    const AttributeEncodeState & GetState() const { return mEncodeState; }

private:
    // We made EncodeListItem() private, and ListEncoderHelper will expose it by Encode()
    friend class ListEncodeHelper;

    template <typename... Ts>
    CHIP_ERROR EncodeListItem(Ts... aArgs)
    {
        // We make Encode atomic here, so need to tell the caller to not rollback when we encounter an error during encoding.
        mEncodeState.mAllowPartialData = true;

        // EncodeListItem must be called after EncodeEmptyList(), thus mCurrentEncodingListIndex and
        // mEncodeState.mCurrentEncodingListIndex are not invalid values.
        if (mCurrentEncodingListIndex < mEncodeState.mCurrentEncodingListIndex)
        {
            // We have encoded this element in previous chunks, skip it.
            mCurrentEncodingListIndex++;
            return CHIP_NO_ERROR;
        }

        TLV::TLVWriter backup;
        mAttributeReportIBsBuilder.Checkpoint(backup);

        CHIP_ERROR err = Encode(std::forward<Ts>(aArgs)...);
        if (err != CHIP_NO_ERROR)
        {
            // For list chunking, ReportEngine should not rollback the buffer when CHIP_NO_MEMORY or similar error occurred.
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
     * EncodeEmptyList encodes the first item of one report with lists (an empty list).
     *
     * If internal state indicates we have already encoded the empty list, this function will encode nothing, set
     * mCurrentEncodingListIndex to 0 and return CHIP_NO_ERROR.
     */
    CHIP_ERROR EncodeEmptyList();

    bool mTriedEncode = false;
    AttributeReportIBs::Builder & mAttributeReportIBsBuilder;
    const FabricIndex mAccessingFabricIndex;
    const ConcreteAttributePath mPath;
    DataVersion mDataVersion;
    AttributeEncodeState mEncodeState;
    ListIndex mCurrentEncodingListIndex = kInvalidListIndex;
};

class AttributeValueDecoder
{
public:
    AttributeValueDecoder(TLV::TLVReader & aReader, FabricIndex aAccessingFabricIndex) :
        mReader(aReader), mAccessingFabricIndex(aAccessingFabricIndex)
    {}

    template <typename T>
    CHIP_ERROR Decode(T & aArg)
    {
        mTriedDecode = true;
        return DataModel::Decode(mReader, aArg);
    }

    bool TriedDecode() const { return mTriedDecode; }

    /**
     * The accessing fabric index for this write interaction.
     */
    FabricIndex AccessingFabricIndex() const { return mAccessingFabricIndex; }

private:
    TLV::TLVReader & mReader;
    bool mTriedDecode = false;
    const FabricIndex mAccessingFabricIndex;
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
     *             data.  If this function returns scucess and no attempt is
     *             made to encode data using aEncoder, the
     *             AttributeAccessInterface did not try to provide any data.  In
     *             this case, normal attribute access will happen for the read.
     *             This may involve reading from the attribute store or external
     *             attribute callbacks.
     */
    virtual CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) = 0;

    /**
     * Callback for writing attributes.
     *
     * @param [in] aPath indicates which exact data is being written.
     * @param [in] aDecoder the AttributeValueDecoder to use for decoding the
     *             data.  If this function returns scucess and no attempt is
     *             made to decode data using aDecoder, the
     *             AttributeAccessInterface did not try to write any data.  In
     *             this case, normal attribute access will happen for the write.
     *             This may involve writing to the attribute store or external
     *             attribute callbacks.
     */
    virtual CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) { return CHIP_NO_ERROR; }

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
