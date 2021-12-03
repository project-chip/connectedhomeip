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
                            const ConcreteReadAttributePath & aPath)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    AttributeReportIB::Builder attributeReport = aAttributeReportIBs.CreateAttributeReport();
    err                                        = attributeReport.GetError();
    SuccessOrExit(attributeReport.GetError());

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
    const uint32_t kReservedSizeEndOfReportIBs = 1;

    aReportDataBuilder.Checkpoint(backup);
    auto attributeReportIBs = aReportDataBuilder.CreateAttributeReportIBs();
    SuccessOrExit(err = aReportDataBuilder.GetError());

    //
    // Reserve enough space for closing out the Report IB list
    //
    attributeReportIBs.GetWriter()->ReserveBuffer(kReservedSizeEndOfReportIBs);

    {
        // TODO: Figure out how AttributePathExpandIterator should handle read
        // vs write paths.
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
            ConcreteReadAttributePath pathForRetrieval(readPath);
            err = RetrieveClusterData(apReadHandler->GetAccessingFabricIndex(), attributeReportIBs, pathForRetrieval);
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
    //
    // Running out of space is an error that we're expected to handle - the incompletely written DataIB has already been rolled back
    // earlier to ensure only whole and complete DataIBs are present in the stream.
    //
    // We can safely clear out the error so that the rest of the machinery to close out the reports, etc. will function correctly.
    // These are are guaranteed to not fail since we've already reserved memory for the remaining 'close out' TLV operations in this
    // function and its callers.
    //
    if ((err == CHIP_ERROR_BUFFER_TOO_SMALL) || (err == CHIP_ERROR_NO_MEMORY))
    {
        ChipLogDetail(DataManagement, "<RE:Run> We cannot put more chunks into this report. Enable chunking.");

        //
        // Reset the error tracked within the builder. Otherwise, any further attempts to write
        // data through the builder will be blocked by that error.
        //
        attributeReportIBs.ResetError();
        err = CHIP_NO_ERROR;
    }

    //
    // Only close out the report if we haven't hit an error yet so far.
    //
    if (err == CHIP_NO_ERROR)
    {
        attributeReportIBs.GetWriter()->UnreserveBuffer(kReservedSizeEndOfReportIBs);

        attributeReportIBs.EndOfAttributeReportIBs();
        err = attributeReportIBs.GetError();

        //
        // We reserved space for this earlier - consequently, the call to end the ReportIBs should
        // never fail, so assert if we do since that's a logic bug.
        //
        VerifyOrDie(err == CHIP_NO_ERROR);
    }

    //
    // Rollback the the entire ReportIB array if we never wrote any attributes
    // AND never hit an error.
    //
    if (!attributeDataWritten && err == CHIP_NO_ERROR)
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
    EventReportIBs::Builder EventReportIBs;
    bool hasMoreChunks = false;

    aReportDataBuilder.Checkpoint(backup);

    VerifyOrExit(clusterInfoList != nullptr, );
    VerifyOrExit(apReadHandler != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    // If the eventManager is not valid or has not been initialized,
    // skip the rest of processing
    VerifyOrExit(eventManager.IsValid(), ChipLogError(DataManagement, "EventManagement has not yet initialized"));

    EventReportIBs = aReportDataBuilder.CreateEventReports();
    SuccessOrExit(err = EventReportIBs.GetError());

    memcpy(initialEvents, eventNumberList, sizeof(initialEvents));

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
        err = eventManager.FetchEventsSince(*(EventReportIBs.GetWriter()), clusterInfoList, apReadHandler->GetCurrentPriority(),
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

    EventReportIBs.EndOfEventReports();
    SuccessOrExit(err = EventReportIBs.GetError());

    ChipLogDetail(DataManagement, "Fetched %zu events", eventCount);

exit:
    if (err == CHIP_NO_ERROR && (eventCount == 0 || eventClean))
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

    // Reserved size for the MoreChunks boolean flag, which takes up 1 byte for the control tag and 1 byte for the context tag.
    const uint32_t kReservedSizeForMoreChunksFlag = 1 + 1;

    // Reserved size for the end of report message, which is an end-of-container (i.e 1 byte for the control tag).
    const uint32_t kReservedSizeForEndOfReportMessage = 1;

    VerifyOrExit(!bufHandle.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    if (bufHandle->AvailableDataLength() > kMaxSecureSduLengthBytes)
    {
        reservedSize = static_cast<uint16_t>(bufHandle->AvailableDataLength() - kMaxSecureSduLengthBytes);
    }

    reportDataWriter.Init(std::move(bufHandle));

#if CONFIG_IM_BUILD_FOR_UNIT_TEST
    reportDataWriter.ReserveBuffer(mReservedSize);
#endif

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

    SuccessOrExit(err = reportDataWriter.ReserveBuffer(kReservedSizeForMoreChunksFlag + kReservedSizeForEndOfReportMessage));

    err = BuildSingleReportDataAttributeReportIBs(reportDataBuilder, apReadHandler, &hasMoreChunks);
    SuccessOrExit(err);

    err = BuildSingleReportDataEventReports(reportDataBuilder, apReadHandler, &hasMoreChunks);
    SuccessOrExit(err);

    SuccessOrExit(reportDataBuilder.GetError());

    SuccessOrExit(err = reportDataWriter.UnreserveBuffer(kReservedSizeForMoreChunksFlag + kReservedSizeForEndOfReportMessage));
    if (hasMoreChunks)
    {
        reportDataBuilder.MoreChunkedMessages(hasMoreChunks);
    }
    else if (apReadHandler->IsReadType())
    {
        reportDataBuilder.SuppressResponse(true);
    }

    reportDataBuilder.EndOfReportDataMessage();

    //
    // Since we've already reserved space for both the MoreChunked/SuppressResponse flags, as well as
    // the end-of-container flag for the end of the report, we should never hit an error closing out the message.
    //
    VerifyOrDie(reportDataBuilder.GetError() == CHIP_NO_ERROR);

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

CHIP_ERROR Engine::ScheduleUrgentEventDelivery(ConcreteEventPath & aPath)
{
    for (auto & handler : InteractionModelEngine::GetInstance()->mReadHandlers)
    {
        for (auto clusterInfo = handler.GetEventClusterInfolist(); clusterInfo != nullptr; clusterInfo = clusterInfo->mpNext)
        {
            if (clusterInfo->IsEventPathSupersetOf(aPath))
            {
                ChipLogProgress(DataManagement, "<RE> Unblock Urgent Event Delivery for readHandler[%d]",
                                InteractionModelEngine::GetInstance()->GetReadHandlerArrayIndex(&handler));
                handler.UnblockUrgentEventDelivery();
            }
        }
    }
    return ScheduleRun();
}

}; // namespace reporting
} // namespace app
} // namespace chip

void __attribute__((weak)) MatterPreAttributeReadCallback(const chip::app::ConcreteAttributePath & attributePath) {}
void __attribute__((weak)) MatterPostAttributeReadCallback(const chip::app::ConcreteAttributePath & attributePath) {}
