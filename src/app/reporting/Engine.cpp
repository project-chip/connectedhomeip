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

/**
 *    @file
 *      This file implements reporting engine for CHIP
 *      Data Model profile.
 *
 */

#include <app/InteractionModelEngine.h>
#include <app/reporting/Engine.h>

namespace chip {
namespace app {
namespace reporting {
CHIP_ERROR Engine::Init()
{
    mMoreChunkedMessages = false;
    mNumReportsInFlight  = 0;
    mCurReadHandlerIdx   = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR
Engine::RetrieveClusterData(AttributeDataElement::Builder & aAttributeDataElementBuilder, ClusterInfo & aClusterInfo)
{
    CHIP_ERROR err                              = CHIP_NO_ERROR;
    TLV::TLVType type                           = TLV::kTLVType_NotSpecified;
    AttributePath::Builder attributePathBuilder = aAttributeDataElementBuilder.CreateAttributePathBuilder();
    attributePathBuilder.NodeId(aClusterInfo.mAttributePathParams.mNodeId)
        .EndpointId(aClusterInfo.mAttributePathParams.mEndpointId)
        .ClusterId(aClusterInfo.mAttributePathParams.mClusterId)
        .FieldId(aClusterInfo.mAttributePathParams.mFieldId)
        .EndOfAttributePath();
    err = attributePathBuilder.GetError();
    SuccessOrExit(err);

    aAttributeDataElementBuilder.GetWriter()->StartContainer(TLV::ContextTag(AttributeDataElement::kCsTag_Data),
                                                             TLV::kTLVType_Structure, type);
    err = ReadSingleClusterData(aClusterInfo.mAttributePathParams, *(aAttributeDataElementBuilder.GetWriter()));
    SuccessOrExit(err);
    aAttributeDataElementBuilder.GetWriter()->EndContainer(type);
    aAttributeDataElementBuilder.DataVersion(0).MoreClusterData(false).EndOfAttributeDataElement();
    err = aAttributeDataElementBuilder.GetError();
    // TODO: Add DataVersion support

exit:
    aClusterInfo.ClearDirty();

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DataManagement, "Error retrieving data from clusterId: %08x, err = %d",
                     aClusterInfo.mAttributePathParams.mClusterId, err);
    }

    return err;
}

CHIP_ERROR Engine::BuildSingleReportDataAttributeDataList(ReportData::Builder & reportDataBuilder, ReadHandler * apReadHandler)
{
    CHIP_ERROR err                               = CHIP_NO_ERROR;
    ClusterInfo * clusterInfo                    = apReadHandler->GetCluterInfolist();
    AttributeDataList::Builder attributeDataList = reportDataBuilder.CreateAttributeDataListBuilder();
    SuccessOrExit(reportDataBuilder.GetError());
    // TODO: Need to handle multiple chunk of message
    while (clusterInfo != nullptr)
    {
        if (clusterInfo->IsDirty())
        {
            AttributeDataElement::Builder attributeDataElementBuilder = attributeDataList.CreateAttributeDataElementBuilder();
            ChipLogDetail(DataManagement, "<RE:Run> Cluster %u, Field %u is dirty", clusterInfo->mAttributePathParams.mClusterId,
                          clusterInfo->mAttributePathParams.mFieldId);
            // Retrieve data for this cluster instance and clear its dirty flag.
            err = RetrieveClusterData(attributeDataElementBuilder, *clusterInfo);
            VerifyOrExit(err == CHIP_NO_ERROR,
                         ChipLogError(DataManagement, "<RE:Run> Error retrieving data from cluster, aborting"));
        }

        clusterInfo = clusterInfo->mpNext;
    }

exit:
    ChipLogFunctError(err);
    return err;
}

CHIP_ERROR Engine::BuildAndSendSingleReportData(ReadHandler * apReadHandler)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter reportDataWriter;
    ReportData::Builder reportDataBuilder;
    chip::System::PacketBufferHandle bufHandle = System::PacketBufferHandle::New(chip::app::kMaxSecureSduLengthBytes);

    VerifyOrExit(!bufHandle.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    reportDataWriter.Init(std::move(bufHandle));

    // Create a report data.
    err = reportDataBuilder.Init(&reportDataWriter);
    SuccessOrExit(err);

    err = BuildSingleReportDataAttributeDataList(reportDataBuilder, apReadHandler);
    SuccessOrExit(err);
    // TODO: Fill in the EventList.
    // err = BuildSingleReportDataEventList(reportDataBuilder, apReadHandler);
    // SuccessOrExit(err);

    // TODO: Add mechanism to set mSuppressResponse to handle status reports for multiple reports
    // TODO: Add more chunk message support, currently mMoreChunkedMessages is always false.
    if (mMoreChunkedMessages)
    {
        reportDataBuilder.MoreChunkedMessages(mMoreChunkedMessages);
    }

    reportDataBuilder.EndOfReportData();
    SuccessOrExit(reportDataBuilder.GetError());

    err = reportDataWriter.Finalize(&bufHandle);
    SuccessOrExit(err);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    {
        ChipLogDetail(DataManagement, "<RE> Dumping report data...");
        chip::System::PacketBufferTLVReader reader;
        ReportData::Parser report;

        reader.Init(bufHandle.Retain());
        reader.Next();

        err = report.Init(reader);
        SuccessOrExit(err);

        err = report.CheckSchemaValidity();
        SuccessOrExit(err);
    }
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

    ChipLogDetail(DataManagement, "<RE> Sending report...");
    err = SendReport(apReadHandler, std::move(bufHandle));
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(DataManagement, "<RE> Error sending out report data with %d!", err));

    ChipLogDetail(DataManagement, "<RE> ReportsInFlight = %u with readHandler %u, RE has %s", mNumReportsInFlight,
                  mCurReadHandlerIdx, mMoreChunkedMessages ? "more messages" : "no more messages");

    if (!mMoreChunkedMessages)
    {
        OnReportConfirm();
    }

exit:
    ChipLogFunctError(err);
    if (!mMoreChunkedMessages || err != CHIP_NO_ERROR)
    {
        apReadHandler->Shutdown();
    }
    return err;
}

void Engine::Run(System::Layer * aSystemLayer, void * apAppState, System::Error)
{
    Engine * const pEngine = reinterpret_cast<Engine *>(apAppState);
    pEngine->Run();
}

CHIP_ERROR Engine::ScheduleRun()
{
    if (InteractionModelEngine::GetInstance()->GetExchangeManager() != nullptr)
    {
        return InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionMgr()->SystemLayer()->ScheduleWork(Run, this);
    }
    else
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
}

void Engine::Run()
{
    uint32_t numReadHandled = 0;

    InteractionModelEngine * imEngine = InteractionModelEngine::GetInstance();
    ReadHandler * readHandler         = imEngine->mReadHandlers + mCurReadHandlerIdx;

    while ((mNumReportsInFlight < CHIP_MAX_REPORTS_IN_FLIGHT) && (numReadHandled < CHIP_MAX_NUM_READ_HANDLER))
    {
        if (readHandler->IsReportable())
        {
            CHIP_ERROR err = BuildAndSendSingleReportData(readHandler);
            ChipLogFunctError(err);
            return;
        }
        numReadHandled++;
        mCurReadHandlerIdx = (mCurReadHandlerIdx + 1) % CHIP_MAX_NUM_READ_HANDLER;
        readHandler        = imEngine->mReadHandlers + mCurReadHandlerIdx;
    }
}

CHIP_ERROR Engine::SendReport(ReadHandler * apReadHandler, System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // We can only have 1 report in flight for any given read - increment and break out.
    mNumReportsInFlight++;

    err = apReadHandler->SendReportData(std::move(aPayload));

    if (err != CHIP_NO_ERROR)
    {
        mNumReportsInFlight--;
    }
    return err;
}

void Engine::OnReportConfirm()
{
    VerifyOrDie(mNumReportsInFlight > 0);

    mNumReportsInFlight--;
    ChipLogDetail(DataManagement, "<RE> OnReportConfirm: NumReports = %d", mNumReportsInFlight);
}

}; // namespace reporting
}; // namespace app
}; // namespace chip
