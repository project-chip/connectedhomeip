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

#include <app/AttributeAccessInterface.h>

namespace chip {
namespace app {

CHIP_ERROR AttributeReportBuilder::PrepareAttribute(AttributeReportIBs::Builder & aAttributeReportIBsBuilder,
                                                    const ConcreteDataAttributePath & aPath, DataVersion aDataVersion)
{
    AttributeReportIB::Builder & attributeReportIBBuilder = aAttributeReportIBsBuilder.CreateAttributeReport();
    ReturnErrorOnFailure(aAttributeReportIBsBuilder.GetError());

    AttributeDataIB::Builder & attributeDataIBBuilder = attributeReportIBBuilder.CreateAttributeData();
    ReturnErrorOnFailure(attributeReportIBBuilder.GetError());

    attributeDataIBBuilder.DataVersion(aDataVersion);

    AttributePathIB::Builder & attributePathIBBuilder = attributeDataIBBuilder.CreatePath();
    ReturnErrorOnFailure(attributeDataIBBuilder.GetError());

    attributePathIBBuilder.Endpoint(aPath.mEndpointId).Cluster(aPath.mClusterId).Attribute(aPath.mAttributeId);

    if (aPath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        // An append to a list (or a data chunk consisting just one list entry that's part of a bigger list) is represented by a
        // null list index in the path.
        attributePathIBBuilder.ListIndex(DataModel::Nullable<ListIndex>());
    }

    ReturnErrorOnFailure(attributePathIBBuilder.EndOfAttributePathIB().GetError());

    return attributeDataIBBuilder.GetError();
}

CHIP_ERROR AttributeReportBuilder::FinishAttribute(AttributeReportIBs::Builder & aAttributeReportIBsBuilder)
{
    ReturnErrorOnFailure(aAttributeReportIBsBuilder.GetAttributeReport().GetAttributeData().EndOfAttributeDataIB().GetError());
    return aAttributeReportIBsBuilder.GetAttributeReport().EndOfAttributeReportIB().GetError();
}

namespace {

constexpr uint32_t kEndOfListByteCount = 1;
// 2 bytes: one to end the AttributeDataIB and one to end the AttributeReportIB.
constexpr uint32_t kEndOfAttributeReportIBByteCount = 2;
constexpr TLV::TLVType kAttributeDataIBType         = TLV::kTLVType_Structure;

} // anonymous namespace

CHIP_ERROR AttributeValueEncoder::EnsureListStarted()
{
    VerifyOrDie(mCurrentEncodingListIndex == kInvalidListIndex);

    mEncodingInitialList = (mEncodeState.mCurrentEncodingListIndex == kInvalidListIndex);
    if (mEncodingInitialList)
    {
        // Clear mAllowPartialData flag here since this encode procedure is not atomic.
        // The most common error in this function is CHIP_ERROR_NO_MEMORY / CHIP_ERROR_BUFFER_TOO_SMALL, just revert and try
        // next time is ok.
        mEncodeState.mAllowPartialData = false;

        AttributeReportBuilder builder;

        mPath.mListOp = ConcreteDataAttributePath::ListOperation::ReplaceAll;
        ReturnErrorOnFailure(builder.PrepareAttribute(mAttributeReportIBsBuilder, mPath, mDataVersion));

        auto * attributeDataWriter = mAttributeReportIBsBuilder.GetAttributeReport().GetAttributeData().GetWriter();
        TLV::TLVType outerType;
        ReturnErrorOnFailure(
            attributeDataWriter->StartContainer(TLV::ContextTag(AttributeDataIB::Tag::kData), TLV::kTLVType_Array, outerType));
        VerifyOrDie(outerType == kAttributeDataIBType);

        // Instead of reserving hardcoded amounts, we could checkpoint the
        // writer, encode array end and FinishAttribute, check that this fits,
        // measure how much the writer advanced, then restore the checkpoint,
        // reserve the measured value, and save it.  But that's probably more
        // cycles than just reserving this known constant.
        ReturnErrorOnFailure(
            mAttributeReportIBsBuilder.GetWriter()->ReserveBuffer(kEndOfAttributeReportIBByteCount + kEndOfListByteCount));

        mEncodeState.mCurrentEncodingListIndex = 0;
    }
    else
    {
        // For all elements in the list, a report with append operation will be generated. This will not be changed during encoding
        // of each report since the users cannot access mPath.
        mPath.mListOp = ConcreteDataAttributePath::ListOperation::AppendItem;
    }

    mCurrentEncodingListIndex = 0;

    // After encoding the initial list start, the remaining items are atomically encoded into the buffer. Tell report engine to not
    // revert partial data.
    mEncodeState.mAllowPartialData = true;

    return CHIP_NO_ERROR;
}

void AttributeValueEncoder::EnsureListEnded()
{
    if (!mEncodingInitialList)
    {
        // Nothing to do.
        return;
    }

    // Unreserve the space we reserved just for this.  Crash if anything here
    // fails, because that would mean that we've corrupted our data, and since
    // mEncodeState.mAllowPartialData is true nothing will clean up for us here.
    auto * attributeDataWriter = mAttributeReportIBsBuilder.GetAttributeReport().GetAttributeData().GetWriter();
    VerifyOrDie(attributeDataWriter->UnreserveBuffer(kEndOfListByteCount + kEndOfAttributeReportIBByteCount) == CHIP_NO_ERROR);
    VerifyOrDie(attributeDataWriter->EndContainer(kAttributeDataIBType) == CHIP_NO_ERROR);

    AttributeReportBuilder builder;
    VerifyOrDie(builder.FinishAttribute(mAttributeReportIBsBuilder) == CHIP_NO_ERROR);
}

} // namespace app
} // namespace chip
