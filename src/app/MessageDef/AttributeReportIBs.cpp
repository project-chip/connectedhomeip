/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC.
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
/**
 *    @file
 *      This file defines AttributeReportIBs parser and builder in CHIP interaction model
 *
 */

#include "AttributeReportIBs.h"
#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR AttributeReportIBs::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;

    PRETTY_PRINT("AttributeReportIBs =");
    PRETTY_PRINT("[");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrReturnError(TLV::AnonymousTag() == reader.GetTag(), CHIP_ERROR_INVALID_TLV_TAG);
        {
            AttributeReportIB::Parser AttributeReport;
            ReturnErrorOnFailure(AttributeReport.Init(reader));
            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(AttributeReport.CheckSchemaValidity());
            PRETTY_PRINT_DECDEPTH();
        }
    }

    PRETTY_PRINT("],");
    PRETTY_PRINT_BLANK_LINE();

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

AttributeReportIB::Builder & AttributeReportIBs::Builder::CreateAttributeReport()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mAttributeReport.Init(mpWriter);
    }
    return mAttributeReport;
}

AttributeReportIBs::Builder & AttributeReportIBs::Builder::EndOfAttributeReportIBs()
{
    EndOfContainer();
    return *this;
}

CHIP_ERROR AttributeReportIBs::Builder::EncodeAttributeStatus(const ConcreteReadAttributePath & aPath, const StatusIB & aStatus)
{
    AttributeReportIB::Builder & attributeReport = CreateAttributeReport();
    ReturnErrorOnFailure(GetError());
    AttributeStatusIB::Builder & attributeStatusIBBuilder = attributeReport.CreateAttributeStatus();
    ReturnErrorOnFailure(attributeReport.GetError());
    AttributePathIB::Builder & attributePathIBBuilder = attributeStatusIBBuilder.CreatePath();
    ReturnErrorOnFailure(attributeStatusIBBuilder.GetError());

    attributePathIBBuilder.Endpoint(aPath.mEndpointId)
        .Cluster(aPath.mClusterId)
        .Attribute(aPath.mAttributeId)
        .EndOfAttributePathIB();
    ReturnErrorOnFailure(attributePathIBBuilder.GetError());
    StatusIB::Builder & statusIBBuilder = attributeStatusIBBuilder.CreateErrorStatus();
    ReturnErrorOnFailure(attributeStatusIBBuilder.GetError());
    statusIBBuilder.EncodeStatusIB(aStatus);
    ReturnErrorOnFailure(statusIBBuilder.GetError());

    ReturnErrorOnFailure(attributeStatusIBBuilder.EndOfAttributeStatusIB().GetError());
    return attributeReport.EndOfAttributeReportIB().GetError();
}

} // namespace app
} // namespace chip
