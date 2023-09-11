/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Google LLC.
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 * SPDX-License-Identifier: Apache-2.0
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
#if CHIP_CONFIG_IM_PRETTY_PRINT
CHIP_ERROR AttributeReportIBs::Parser::PrettyPrint() const
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
            ReturnErrorOnFailure(AttributeReport.PrettyPrint());
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
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

AttributeReportIB::Builder & AttributeReportIBs::Builder::CreateAttributeReport()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mAttributeReport.Init(mpWriter);
    }
    return mAttributeReport;
}

CHIP_ERROR AttributeReportIBs::Builder::EndOfAttributeReportIBs()
{
    EndOfContainer();
    return GetError();
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

    ReturnErrorOnFailure(attributeStatusIBBuilder.EndOfAttributeStatusIB());
    return attributeReport.EndOfAttributeReportIB();
}

} // namespace app
} // namespace chip
