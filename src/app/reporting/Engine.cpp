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

#include <app/AppBuildConfig.h>
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

EventNumber Engine::CountEvents(ReadHandler * apReadHandler, EventNumber * apInitialEvents)
{
    EventNumber event_count             = 0;
    EventNumber * vendedEventNumberList = apReadHandler->GetVendedEventNumberList();
    for (size_t index = 0; index < kNumPriorityLevel; index++)
    {
        if (vendedEventNumberList[index] > apInitialEvents[index])
        {
            event_count += vendedEventNumberList[index] - apInitialEvents[index];
        }
    }
    return event_count;
}

CHIP_ERROR
Engine::RetrieveClusterData(AttributeDataElement::Builder & aAttributeDataElementBuilder, ClusterInfo & aClusterInfo)
{
    CHIP_ERROR err                              = CHIP_NO_ERROR;
    AttributePath::Builder attributePathBuilder = aAttributeDataElementBuilder.CreateAttributePathBuilder();
    attributePathBuilder.NodeId(aClusterInfo.mNodeId)
        .EndpointId(aClusterInfo.mEndpointId)
        .ClusterId(aClusterInfo.mClusterId)
        .FieldId(aClusterInfo.mFieldId)
        .EndOfAttributePath();
    err = attributePathBuilder.GetError();
    SuccessOrExit(err);

    err = ReadSingleClusterData(aClusterInfo, aAttributeDataElementBuilder.GetWriter(), nullptr /* data exists */);
    SuccessOrExit(err);
    aAttributeDataElementBuilder.MoreClusterData(false);
    aAttributeDataElementBuilder.EndOfAttributeDataElement();
    err = aAttributeDataElementBuilder.GetError();

exit:
    aClusterInfo.ClearDirty();

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DataManagement, "Error retrieving data from clusterId: %" PRIx32 ", err = %" CHIP_ERROR_FORMAT,
                     aClusterInfo.mClusterId, err);
    }

    return err;
}

CHIP_ERROR Engine::BuildSingleReportDataAttributeDataList(ReportData::Builder & reportDataBuilder, ReadHandler * apReadHandler)
{
    CHIP_ERROR err                               = CHIP_NO_ERROR;
    ClusterInfo * clusterInfo                    = apReadHandler->GetAttributeClusterInfolist();
    AttributeDataList::Builder attributeDataList = reportDataBuilder.CreateAttributeDataListBuilder();
    SuccessOrExit(err = reportDataBuilder.GetError());
    // TODO: Need to handle multiple chunk of message
    while (clusterInfo != nullptr)
    {
        if (clusterInfo->IsDirty())
        {
            AttributeDataElement::Builder attributeDataElementBuilder = attributeDataList.CreateAttributeDataElementBuilder();
            ChipLogDetail(DataManagement, "<RE:Run> Cluster %" PRIx32 ", Field %" PRIx32 " is dirty", clusterInfo->mClusterId,
                          clusterInfo->mFieldId);
            // Retrieve data for this cluster instance and clear its dirty flag.
            err = RetrieveClusterData(attributeDataElementBuilder, *clusterInfo);
            VerifyOrExit(err == CHIP_NO_ERROR,
                         ChipLogError(DataManagement, "<RE:Run> Error retrieving data from cluster, aborting"));
        }

        clusterInfo = clusterInfo->mpNext;
    }
    attributeDataList.EndOfAttributeDataList();
    err = attributeDataList.GetError();

exit:
    ChipLogFunctError(err);
    return err;
}

CHIP_ERROR Engine::BuildSingleReportDataEventList(ReportData::Builder & aReportDataBuilder, ReadHandler * apReadHandler)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    EventNumber eventCount = 0;
    TLV::TLVWriter backup;
    bool eventClean = true;
    EventNumber initialEvents[kNumPriorityLevel];
    ClusterInfo * clusterInfoList  = apReadHandler->GetEventClusterInfolist();
    EventNumber * eventNumberList  = apReadHandler->GetVendedEventNumberList();
    EventManagement & eventManager = EventManagement::GetInstance();
    EventList::Builder eventList;

    VerifyOrExit(clusterInfoList != nullptr, );

    aReportDataBuilder.Checkpoint(backup);

    VerifyOrExit(apReadHandler != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    eventList = aReportDataBuilder.CreateEventDataListBuilder();
    SuccessOrExit(err = eventList.GetError());

    memcpy(initialEvents, eventNumberList, sizeof(initialEvents));
    // If the eventManager is not valid or has not been initialized,
    // skip the rest of processing
    VerifyOrExit(eventManager.IsValid(), err = CHIP_ERROR_INCORRECT_STATE);

    for (size_t index = 0; index < kNumPriorityLevel; index++)
    {
        EventNumber tmpNumber = eventManager.GetFirstEventNumber(static_cast<PriorityLevel>(index));
        if (tmpNumber > initialEvents[index])
        {
            initialEvents[index] = tmpNumber;
        }
    }

    eventClean = apReadHandler->CheckEventClean(eventManager);

    // proceed only if there are new events.
    if (eventClean)
    {
        aReportDataBuilder.Rollback(backup);
        ExitNow(); // Read clean, move along
    }

    while (apReadHandler->GetCurrentPriority() != PriorityLevel::Invalid)
    {
        uint8_t priorityIndex = static_cast<uint8_t>(apReadHandler->GetCurrentPriority());
        err = eventManager.FetchEventsSince(*(eventList.GetWriter()), clusterInfoList, apReadHandler->GetCurrentPriority(),
                                            eventNumberList[priorityIndex]);

        if ((err == CHIP_END_OF_TLV) || (err == CHIP_ERROR_TLV_UNDERRUN) || (err == CHIP_NO_ERROR))
        {
            // We have successfully reached the end of the log for
            // the current priority. Advance to the next
            // priority level.
            err = CHIP_NO_ERROR;
            apReadHandler->MoveToNextScheduledDirtyPriority();
            mMoreChunkedMessages = false;
        }
        else if ((err == CHIP_ERROR_BUFFER_TOO_SMALL) || (err == CHIP_ERROR_NO_MEMORY))
        {
            eventCount = CountEvents(apReadHandler, initialEvents);

            // when first cluster event is too big to fit in the packet, ignore that cluster event.
            if (eventCount == 0)
            {
                eventNumberList[priorityIndex]++;
                ChipLogDetail(DataManagement, "<RE:Run> first cluster event is too big so that it fails to fit in the packet!");
                err = CHIP_NO_ERROR;
            }
            else
            {
                // `FetchEventsSince` has filled the available space
                // within the allowed buffer before it fit all the
                // available events.  This is an expected condition,
                // so we do not propagate the error to higher levels;
                // instead, we terminate the event processing for now
                // (we will get another chance immediately afterwards,
                // with a ew buffer) and do not advance the processing
                // to the next priority level.
                err = CHIP_NO_ERROR;
                break;
            }
            mMoreChunkedMessages = true;
        }
        else
        {
            // All other errors are propagated to higher level.
            // Exiting here and returning an error will lead to
            // abandoning subscription.
            ExitNow();
        }
    }

    eventList.EndOfEventList();
    SuccessOrExit(err = eventList.GetError());

    eventCount = CountEvents(apReadHandler, initialEvents);
    ChipLogDetail(DataManagement, "Fetched 0x" ChipLogFormatX64 " events", ChipLogValueX64(eventCount));

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DataManagement, "Error retrieving events, err = %" CHIP_ERROR_FORMAT, err);
    }

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

    err = BuildSingleReportDataEventList(reportDataBuilder, apReadHandler);
    SuccessOrExit(err);

    // TODO: Add mechanism to set mSuppressResponse to handle status reports for multiple reports
    // TODO: Add more chunk message support, currently mMoreChunkedMessages is always false.
    if (mMoreChunkedMessages)
    {
        reportDataBuilder.MoreChunkedMessages(mMoreChunkedMessages);
    }

    reportDataBuilder.EndOfReportData();
    SuccessOrExit(err = reportDataBuilder.GetError());

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
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(DataManagement, "<RE> Error sending out report data with %" CHIP_ERROR_FORMAT "!", err));

    ChipLogDetail(DataManagement, "<RE> ReportsInFlight = %" PRIu32 " with readHandler %" PRIu32 ", RE has %s", mNumReportsInFlight,
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

void Engine::Run(System::Layer * aSystemLayer, void * apAppState, CHIP_ERROR)
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
    ChipLogDetail(DataManagement, "<RE> OnReportConfirm: NumReports = %" PRIu32, mNumReportsInFlight);
}

}; // namespace reporting
}; // namespace app
}; // namespace chip
