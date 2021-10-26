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

void Engine::Shutdown()
{
    mMoreChunkedMessages = false;
    mNumReportsInFlight  = 0;
    mCurReadHandlerIdx   = 0;
    InteractionModelEngine::GetInstance()->ReleaseClusterInfoList(mpGlobalDirtySet);
    mpGlobalDirtySet = nullptr;
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
Engine::RetrieveClusterData(AttributeDataList::Builder & aAttributeDataList, ClusterInfo & aClusterInfo)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ConcreteAttributePath path(aClusterInfo.mEndpointId, aClusterInfo.mClusterId, aClusterInfo.mFieldId);
    AttributeDataElement::Builder attributeDataElementBuilder = aAttributeDataList.CreateAttributeDataElementBuilder();
    AttributePath::Builder attributePathBuilder               = attributeDataElementBuilder.CreateAttributePathBuilder();
    attributePathBuilder.NodeId(aClusterInfo.mNodeId)
        .EndpointId(aClusterInfo.mEndpointId)
        .ClusterId(aClusterInfo.mClusterId)
        .FieldId(aClusterInfo.mFieldId)
        .EndOfAttributePath();
    err = attributePathBuilder.GetError();
    SuccessOrExit(err);

    ChipLogDetail(DataManagement, "<RE:Run> Cluster %" PRIx32 ", Field %" PRIx32 " is dirty", aClusterInfo.mClusterId,
                  aClusterInfo.mFieldId);

    err = ReadSingleClusterData(path, attributeDataElementBuilder.GetWriter(), nullptr /* data exists */);
    SuccessOrExit(err);
    attributeDataElementBuilder.MoreClusterData(false);
    attributeDataElementBuilder.EndOfAttributeDataElement();
    err = attributeDataElementBuilder.GetError();

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DataManagement, "Error retrieving data from clusterId: " ChipLogFormatMEI ", err = %" CHIP_ERROR_FORMAT,
                     ChipLogValueMEI(aClusterInfo.mClusterId), err.Format());
    }

    return err;
}

CHIP_ERROR Engine::BuildSingleReportDataAttributeDataList(ReportData::Builder & aReportDataBuilder, ReadHandler * apReadHandler)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    bool attributeClean = true;
    TLV::TLVWriter backup;
    aReportDataBuilder.Checkpoint(backup);
    AttributeDataList::Builder attributeDataList = aReportDataBuilder.CreateAttributeDataListBuilder();
    SuccessOrExit(err = aReportDataBuilder.GetError());
    // TODO: Need to handle multiple chunk of message
    for (auto clusterInfo = apReadHandler->GetAttributeClusterInfolist(); clusterInfo != nullptr; clusterInfo = clusterInfo->mpNext)
    {
        if (apReadHandler->IsInitialReport())
        {
            // Retrieve data for this cluster instance and clear its dirty flag.
            err = RetrieveClusterData(attributeDataList, *clusterInfo);
            VerifyOrExit(err == CHIP_NO_ERROR,
                         ChipLogError(DataManagement, "<RE:Run> Error retrieving data from cluster, aborting"));
            attributeClean = false;
        }
        else
        {
            for (auto path = mpGlobalDirtySet; path != nullptr; path = path->mpNext)
            {
                if (clusterInfo->IsAttributePathSupersetOf(*path))
                {
                    err = RetrieveClusterData(attributeDataList, *path);
                }
                else if (path->IsAttributePathSupersetOf(*clusterInfo))
                {
                    err = RetrieveClusterData(attributeDataList, *clusterInfo);
                }
                else
                {
                    // partial overlap is not possible, hence the 'continue' here: clusterInfo and path have nothing in
                    // common.
                    continue;
                }
                VerifyOrExit(err == CHIP_NO_ERROR,
                             ChipLogError(DataManagement, "<RE:Run> Error retrieving data from cluster, aborting"));
                attributeClean = false;
            }
        }
    }
    attributeDataList.EndOfAttributeDataList();
    err = attributeDataList.GetError();

exit:
    if (attributeClean || err != CHIP_NO_ERROR)
    {
        aReportDataBuilder.Rollback(backup);
    }
    return err;
}

CHIP_ERROR Engine::BuildSingleReportDataEventList(ReportData::Builder & aReportDataBuilder, ReadHandler * apReadHandler)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    size_t eventCount = 0;
    TLV::TLVWriter backup;
    bool eventClean = true;
    EventNumber initialEvents[kNumPriorityLevel];
    ClusterInfo * clusterInfoList  = apReadHandler->GetEventClusterInfolist();
    EventNumber * eventNumberList  = apReadHandler->GetVendedEventNumberList();
    EventManagement & eventManager = EventManagement::GetInstance();
    EventList::Builder eventList;

    aReportDataBuilder.Checkpoint(backup);

    VerifyOrExit(clusterInfoList != nullptr, );
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
        ExitNow(); // Read clean, move along
    }

    while (apReadHandler->GetCurrentPriority() != PriorityLevel::Invalid)
    {
        uint8_t priorityIndex = static_cast<uint8_t>(apReadHandler->GetCurrentPriority());
        err = eventManager.FetchEventsSince(*(eventList.GetWriter()), clusterInfoList, apReadHandler->GetCurrentPriority(),
                                            eventNumberList[priorityIndex], eventCount);

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

    ChipLogDetail(DataManagement, "Fetched %zu events", eventCount);

exit:
    if (err != CHIP_NO_ERROR || eventCount == 0 || eventClean)
    {
        aReportDataBuilder.Rollback(backup);
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

    if (apReadHandler->IsSubscriptionType())
    {
        uint64_t subscriptionId = 0;
        apReadHandler->GetSubscriptionId(subscriptionId);
        reportDataBuilder.SubscriptionId(subscriptionId);
    }

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
                 ChipLogError(DataManagement, "<RE> Error sending out report data with %" CHIP_ERROR_FORMAT "!", err.Format()));

    ChipLogDetail(DataManagement, "<RE> ReportsInFlight = %" PRIu32 " with readHandler %" PRIu32 ", RE has %s", mNumReportsInFlight,
                  mCurReadHandlerIdx, mMoreChunkedMessages ? "more messages" : "no more messages");

exit:
    if (err != CHIP_NO_ERROR)
    {
        apReadHandler->Shutdown(ReadHandler::ShutdownOptions::AbortCurrentExchange);
    }
    return err;
}

void Engine::Run(System::Layer * aSystemLayer, void * apAppState)
{
    Engine * const pEngine = reinterpret_cast<Engine *>(apAppState);
    pEngine->Run();
}

CHIP_ERROR Engine::ScheduleRun()
{
    if (InteractionModelEngine::GetInstance()->GetExchangeManager() != nullptr)
    {
        return InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->ScheduleWork(Run,
                                                                                                                             this);
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

    while ((mNumReportsInFlight < CHIP_IM_MAX_REPORTS_IN_FLIGHT) && (numReadHandled < CHIP_IM_MAX_NUM_READ_HANDLER))
    {
        if (readHandler->IsReportable())
        {
            CHIP_ERROR err = BuildAndSendSingleReportData(readHandler);
            if (err != CHIP_NO_ERROR)
            {
                return;
            }
        }
        numReadHandled++;
        mCurReadHandlerIdx = (mCurReadHandlerIdx + 1) % CHIP_IM_MAX_NUM_READ_HANDLER;
        readHandler        = imEngine->mReadHandlers + mCurReadHandlerIdx;
    }

    bool allReadClean = true;
    for (auto & handler : InteractionModelEngine::GetInstance()->mReadHandlers)
    {
        UpdateReadHandlerDirty(handler);
        if (handler.IsDirty())
        {
            allReadClean = false;
            break;
        }
    }

    if (allReadClean)
    {
        InteractionModelEngine::GetInstance()->ReleaseClusterInfoList(mpGlobalDirtySet);
    }
}

CHIP_ERROR Engine::SetDirty(ClusterInfo & aClusterInfo)
{
    for (auto & handler : InteractionModelEngine::GetInstance()->mReadHandlers)
    {
        if (handler.IsSubscriptionType() && (handler.IsGeneratingReports() || handler.IsAwaitingReportResponse()))
        {
            handler.SetDirty();
        }
    }
    if (!InteractionModelEngine::GetInstance()->MergeOverlappedAttributePath(mpGlobalDirtySet, aClusterInfo) &&
        InteractionModelEngine::GetInstance()->IsOverlappedAttributePath(aClusterInfo))
    {
        ReturnLogErrorOnFailure(InteractionModelEngine::GetInstance()->PushFront(mpGlobalDirtySet, aClusterInfo));
    }
    return CHIP_NO_ERROR;
}

void Engine::UpdateReadHandlerDirty(ReadHandler & aReadHandler)
{
    if (!aReadHandler.IsDirty())
    {
        return;
    }
    if (!aReadHandler.IsSubscriptionType())
    {
        return;
    }
    for (auto clusterInfo = aReadHandler.GetAttributeClusterInfolist(); clusterInfo != nullptr; clusterInfo = clusterInfo->mpNext)
    {
        bool intersected = false;
        for (auto path = mpGlobalDirtySet; path != nullptr; path = path->mpNext)
        {
            if (path->IsAttributePathSupersetOf(*clusterInfo) || clusterInfo->IsAttributePathSupersetOf(*path))
            {
                intersected = true;
            }
        }
        if (!intersected)
        {
            aReadHandler.ClearDirty();
        }
    }
}

CHIP_ERROR Engine::SendReport(ReadHandler * apReadHandler, System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // We can only have 1 report in flight for any given read - increment and break out.
    mNumReportsInFlight++;
    err = apReadHandler->SendReportData(std::move(aPayload));
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
