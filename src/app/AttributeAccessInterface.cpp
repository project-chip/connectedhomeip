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

CHIP_ERROR AttributeValueEncoder::EnsureListStarted()
{
    if (mCurrentEncodingListIndex == kInvalidListIndex)
    {
        if (mEncodeState.mCurrentEncodingListIndex == kInvalidListIndex)
        {
            // Clear mAllowPartialData flag here since this encode procedure is not atomic.
            // The most common error in this function is CHIP_ERROR_NO_MEMORY / CHIP_ERROR_BUFFER_TOO_SMALL, just revert and try
            // next time is ok.
            mEncodeState.mAllowPartialData = false;
            // Spec 10.5.4.3.1, 10.5.4.6 (Replace a list w/ Multiple IBs)
            // Put an empty array before encoding the first array element for list chunking.
            AttributeReportBuilder builder;

            mPath.mListOp = ConcreteDataAttributePath::ListOperation::ReplaceAll;
            ReturnErrorOnFailure(builder.PrepareAttribute(mAttributeReportIBsBuilder, mPath, mDataVersion));
            ReturnErrorOnFailure(builder.EncodeValue(mAttributeReportIBsBuilder, DataModel::List<uint8_t>()));

            ReturnErrorOnFailure(builder.FinishAttribute(mAttributeReportIBsBuilder));
            mEncodeState.mCurrentEncodingListIndex = 0;
        }
        mCurrentEncodingListIndex = 0;
    }

    // After encoding the empty list, the remaining items are atomically encoded into the buffer. Tell report engine to not
    // revert partial data.
    mEncodeState.mAllowPartialData = true;

    // For all elements in the list, a report with append operation will be generated. This will not be changed during encoding
    // of each report since the users cannot access mPath.
    mPath.mListOp = ConcreteDataAttributePath::ListOperation::AppendItem;
    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
