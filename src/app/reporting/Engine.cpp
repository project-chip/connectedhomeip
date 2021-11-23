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
#include <app/util/MatterCallbacks.h>

namespace chip {
namespace app {
namespace reporting {
CHIP_ERROR Engine::Init()
{
    mNumReportsInFlight = 0;
    mCurReadHandlerIdx  = 0;
    return CHIP_NO_ERROR;
}

void Engine::Shutdown()
{
    mNumReportsInFlight = 0;
    mCurReadHandlerIdx  = 0;
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
Engine::RetrieveClusterData(FabricIndex aAccessingFabricIndex, AttributeReportIBs::Builder & aAttributeReportIBs,
                            const ConcreteAttributePath & aPath)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    AttributeReportIB::Builder attributeReport = aAttributeReportIBs.CreateAttributeReport();

    ChipLogDetail(DataManagement, "<RE:Run> Cluster %" PRIx32 ", Attribute %" PRIx32 " is dirty", aPath.mClusterId,
                  aPath.mAttributeId);

    MatterPreAttributeReadCallback(aPath);
    err = ReadSingleClusterData(aAccessingFabricIndex, aPath, attributeReport);
    MatterPostAttributeReadCallback(aPath);
    SuccessOrExit(err);
    attributeReport.EndOfAttributeReportIB();
    err = attributeReport.GetError();

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DataManagement, "Error retrieving data from clusterId: " ChipLogFormatMEI ", err = %" CHIP_ERROR_FORMAT,
                     ChipLogValueMEI(aPath.mClusterId), err.Format());
    }

    return err;
}

CHIP_ERROR Engine::BuildSingleReportDataAttributeReportIBs(ReportDataMessage::Builder & aReportDataBuilder,
                                                           ReadHandler * apReadHandler, bool * apHasMoreChunks)
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    bool attributeDataWritten = false;
    bool hasMoreChunks        = true;
    TLV::TLVWriter backup;
    aReportDataBuilder.Checkpoint(backup);
    auto attributeReportIBs = aReportDataBuilder.CreateAttributeReportIBs();
    SuccessOrExit(err = aReportDataBuilder.GetError());

    {
        ConcreteAttributePath readPath;

        // For each path included in the interested path of the read handler...
        for (; apReadHandler->GetAttributePathExpandIterator()->Get(readPath);
             apReadHandler->GetAttributePathExpandIterator()->Next())
        {
            if (!apReadHandler->IsPriming())
            {
                bool concretePathDirty = false;
                // TODO: Optimize this implementation by making the iterator only emit intersected paths.
                for (auto dirtyPath = mpGlobalDirtySet; dirtyPath != nullptr; dirtyPath = dirtyPath->mpNext)
                {
                    if (dirtyPath->IsAttributePathSupersetOf(readPath))
                    {
                        concretePathDirty = true;
                        break;
                    }
                }

                if (!concretePathDirty)
                {
                    // This attribute is not dirty, we just skip this one.
                    continue;
                }
            }
            // If we are processing a read request, or the initial report of a subscription, just regard all paths as dirty paths.

            TLV::TLVWriter attributeBackup;
            attributeReportIBs.Checkpoint(attributeBackup);
            err = RetrieveClusterData(apReadHandler->GetAccessingFabricIndex(), attributeReportIBs, readPath);
            if (err != CHIP_NO_ERROR)
            {
                // We met a error during writing reports, one common case is we are running out of buffer, rollback the
                // attributeReportIB to avoid any partial data.
                attributeReportIBs.Rollback(attributeBackup);
            }
            SuccessOrExit(err);
            attributeDataWritten = true;
        }
        // We just visited all paths interested by this read handler and did not abort in the middle of iteration, there are no more
        // chunks for this report.
        hasMoreChunks = false;
    }
exit:
    if ((err == CHIP_ERROR_BUFFER_TOO_SMALL) || (err == CHIP_ERROR_NO_MEMORY))
    {
        ChipLogDetail(DataManagement, "<RE:Run> We cannot put more chunks into this report. Enable chunking.");
        err = CHIP_NO_ERROR;
    }

    if (err == CHIP_NO_ERROR)
    {
        attributeReportIBs.EndOfAttributeReportIBs();
        err = attributeReportIBs.GetError();
    }

    if (!attributeDataWritten || err != CHIP_NO_ERROR)
    {
        aReportDataBuilder.Rollback(backup);
    }
    else if (apHasMoreChunks != nullptr)
    {
        *apHasMoreChunks = hasMoreChunks;
    }

    return err;
}

CHIP_ERROR Engine::BuildSingleReportDataEventReports(ReportDataMessage::Builder & aReportDataBuilder, ReadHandler * apReadHandler,
                                                     bool * apHasMoreChunks)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    size_t eventCount = 0;
    TLV::TLVWriter backup;
    bool eventClean = true;
    EventNumber initialEvents[kNumPriorityLevel];
    ClusterInfo * clusterInfoList  = apReadHandler->GetEventClusterInfolist();
    EventNumber * eventNumberList  = apReadHandler->GetVendedEventNumberList();
    EventManagement & eventManager = EventManagement::GetInstance();
    EventReports::Builder EventReports;
    bool hasMoreChunks = false;

    aReportDataBuilder.Checkpoint(backup);

    VerifyOrExit(clusterInfoList != nullptr, );
    VerifyOrExit(apReadHandler != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    EventReports = aReportDataBuilder.CreateEventReports();
    SuccessOrExit(err = EventReports.GetError());

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
        err = eventManager.FetchEventsSince(*(EventReports.GetWriter()), clusterInfoList, apReadHandler->GetCurrentPriority(),
                                            eventNumberList[priorityIndex], eventCount);

        if ((err == CHIP_END_OF_TLV) || (err == CHIP_ERROR_TLV_UNDERRUN) || (err == CHIP_NO_ERROR))
        {
            // We have successfully reached the end of the log for
            // the current priority. Advance to the next
            // priority level.
            err = CHIP_NO_ERROR;
            apReadHandler->MoveToNextScheduledDirtyPriority();
            hasMoreChunks = false;
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
            hasMoreChunks = true;
        }
        else
        {
            // All other errors are propagated to higher level.
            // Exiting here and returning an error will lead to
            // abandoning subscription.
            ExitNow();
        }
    }

    EventReports.EndOfEventReports();
    SuccessOrExit(err = EventReports.GetError());

    ChipLogDetail(DataManagement, "Fetched %zu events", eventCount);

exit:
    if (err != CHIP_NO_ERROR || eventCount == 0 || eventClean)
    {
        aReportDataBuilder.Rollback(backup);
    }
    else if (apHasMoreChunks != nullptr)
    {
        *apHasMoreChunks = hasMoreChunks;
    }
    return err;
}

CHIP_ERROR Engine::BuildAndSendSingleReportData(ReadHandler * apReadHandler)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter reportDataWriter;
    ReportDataMessage::Builder reportDataBuilder;
    chip::System::PacketBufferHandle bufHandle = System::PacketBufferHandle::New(chip::app::kMaxSecureSduLengthBytes);
    uint16_t reservedSize                      = 0;
    bool hasMoreChunks                         = false;

    VerifyOrExit(!bufHandle.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    if (bufHandle->AvailableDataLength() > kMaxSecureSduLengthBytes)
    {
        reservedSize = static_cast<uint16_t>(bufHandle->AvailableDataLength() - kMaxSecureSduLengthBytes);
    }

    reportDataWriter.Init(std::move(bufHandle));

    // Always limit the size of the generated packet to fit within kMaxSecureSduLengthBytes regardless of the available buffer
    // capacity.
    // Also, we need to reserve some extra space for the MIC field.
    reportDataWriter.ReserveBuffer(static_cast<uint32_t>(reservedSize + chip::Crypto::CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES));

    // Create a report data.
    err = reportDataBuilder.Init(&reportDataWriter);
    SuccessOrExit(err);

    if (apReadHandler->IsSubscriptionType())
    {
        uint64_t subscriptionId = 0;
        apReadHandler->GetSubscriptionId(subscriptionId);
        reportDataBuilder.SubscriptionId(subscriptionId);
    }

    SuccessOrExit(err = reportDataWriter.ReserveBuffer(Engine::kReservedSizeForMoreChunksFlag));

    err = BuildSingleReportDataAttributeReportIBs(reportDataBuilder, apReadHandler, &hasMoreChunks);
    SuccessOrExit(err);

    err = BuildSingleReportDataEventReports(reportDataBuilder, apReadHandler, &hasMoreChunks);
    SuccessOrExit(err);

    SuccessOrExit(err = reportDataWriter.UnreserveBuffer(Engine::kReservedSizeForMoreChunksFlag));
    if (hasMoreChunks)
    {
        reportDataBuilder.MoreChunkedMessages(hasMoreChunks);
    }
    else if (apReadHandler->IsReadType())
    {
        reportDataBuilder.SuppressResponse(true);
    }

    reportDataBuilder.EndOfReportDataMessage();
    SuccessOrExit(err = reportDataBuilder.GetError());

    err = reportDataWriter.Finalize(&bufHandle);
    SuccessOrExit(err);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    {
        ChipLogDetail(DataManagement, "<RE> Dumping report data...");
        chip::System::PacketBufferTLVReader reader;
        ReportDataMessage::Parser report;

        reader.Init(bufHandle.Retain());
        reader.Next();

        err = report.Init(reader);
        SuccessOrExit(err);

        err = report.CheckSchemaValidity();
        SuccessOrExit(err);
    }
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

    ChipLogDetail(DataManagement, "<RE> Sending report (payload has %" PRIu32 " bytes)...", reportDataWriter.GetLengthWritten());
    err = SendReport(apReadHandler, std::move(bufHandle), hasMoreChunks);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(DataManagement, "<RE> Error sending out report data with %" CHIP_ERROR_FORMAT "!", err.Format()));

    ChipLogDetail(DataManagement, "<RE> ReportsInFlight = %" PRIu32 " with readHandler %" PRIu32 ", RE has %s", mNumReportsInFlight,
                  mCurReadHandlerIdx, hasMoreChunks ? "more messages" : "no more messages");

exit:
    if (err != CHIP_NO_ERROR)
    {
        apReadHandler->Shutdown(ReadHandler::ShutdownOptions::AbortCurrentExchange);
    }
    else if (apReadHandler->IsReadType() && !hasMoreChunks)
    {
        apReadHandler->Shutdown();
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
    if (mRunScheduled)
    {
        return CHIP_NO_ERROR;
    }

    if (InteractionModelEngine::GetInstance()->GetExchangeManager() != nullptr)
    {
        mRunScheduled = true;
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

    mRunScheduled = false;

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
        // We call SetDirty for both read interactions and subscribe interactions, since we may sent inconsistent attribute data
        // between two chunks. SetDirty will be ignored automatically by read handlers which is waiting for response to last message
        // chunk for read interactions.
        if (handler.IsGeneratingReports() || handler.IsAwaitingReportResponse())
        {
            for (auto clusterInfo = handler.GetAttributeClusterInfolist(); clusterInfo != nullptr;
                 clusterInfo      = clusterInfo->mpNext)
            {
                if (aClusterInfo.IsAttributePathSupersetOf(*clusterInfo) || clusterInfo->IsAttributePathSupersetOf(aClusterInfo))
                {
                    handler.SetDirty();
                    break;
                }
            }
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

    bool intersected = false;
    for (auto clusterInfo = aReadHandler.GetAttributeClusterInfolist(); clusterInfo != nullptr; clusterInfo = clusterInfo->mpNext)
    {
        for (auto path = mpGlobalDirtySet; path != nullptr; path = path->mpNext)
        {
            if (path->IsAttributePathSupersetOf(*clusterInfo) || clusterInfo->IsAttributePathSupersetOf(*path))
            {
                intersected = true;
                break;
            }
        }
    }
    if (!intersected)
    {
        aReadHandler.ClearDirty();
    }
}

CHIP_ERROR Engine::SendReport(ReadHandler * apReadHandler, System::PacketBufferHandle && aPayload, bool aHasMoreChunks)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // We can only have 1 report in flight for any given read - increment and break out.
    mNumReportsInFlight++;
    err = apReadHandler->SendReportData(std::move(aPayload), aHasMoreChunks);
    return err;
}

void Engine::OnReportConfirm()
{
    VerifyOrDie(mNumReportsInFlight > 0);

    mNumReportsInFlight--;
    ChipLogDetail(DataManagement, "<RE> OnReportConfirm: NumReports = %" PRIu32, mNumReportsInFlight);
}

}; // namespace reporting
} // namespace app
} // namespace chip

void __attribute__((weak)) MatterPreAttributeReadCallback(const chip::app::ConcreteAttributePath & attributePath) {}
void __attribute__((weak)) MatterPostAttributeReadCallback(const chip::app::ConcreteAttributePath & attributePath) {}
