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
#include <app/AttributeReportBuilder.h>

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

    ReturnErrorOnFailure(attributePathIBBuilder.EndOfAttributePathIB());

    return attributeDataIBBuilder.GetError();
}

CHIP_ERROR AttributeReportBuilder::FinishAttribute(AttributeReportIBs::Builder & aAttributeReportIBsBuilder)
{
    ReturnErrorOnFailure(aAttributeReportIBsBuilder.GetAttributeReport().GetAttributeData().EndOfAttributeDataIB());
    return aAttributeReportIBsBuilder.GetAttributeReport().EndOfAttributeReportIB();
}

} // namespace app
} // namespace chip
