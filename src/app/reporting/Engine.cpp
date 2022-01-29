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

using namespace chip::Access;

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
    mGlobalDirtySet.ReleaseAll();
}

CHIP_ERROR
Engine::RetrieveClusterData(const SubjectDescriptor & aSubjectDescriptor, bool aIsFabricFiltered,
                            AttributeReportIBs::Builder & aAttributeReportIBs, const ConcreteReadAttributePath & aPath,
                            AttributeValueEncoder::AttributeEncodeState * aEncoderState)
{
    ChipLogDetail(DataManagement, "<RE:Run> Cluster %" PRIx32 ", Attribute %" PRIx32 " is dirty", aPath.mClusterId,
                  aPath.mAttributeId);
    MatterPreAttributeReadCallback(aPath);
    ReturnErrorOnFailure(ReadSingleClusterData(aSubjectDescriptor, aIsFabricFiltered, aPath, aAttributeReportIBs, aEncoderState));
    MatterPostAttributeReadCallback(aPath);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Engine::BuildSingleReportDataAttributeReportIBs(ReportDataMessage::Builder & aReportDataBuilder,
                                                           ReadHandler * apReadHandler, bool * apHasMoreChunks,
                                                           bool * apHasEncodedData)
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    bool attributeDataWritten = false;
    bool hasMoreChunks        = true;
    TLV::TLVWriter backup;
    const uint32_t kReservedSizeEndOfReportIBs = 1;

    aReportDataBuilder.Checkpoint(backup);

    AttributeReportIBs::Builder & attributeReportIBs = aReportDataBuilder.CreateAttributeReportIBs();
    size_t emptyReportDataLength                     = 0;

    SuccessOrExit(err = aReportDataBuilder.GetError());

    emptyReportDataLength = attributeReportIBs.GetWriter()->GetLengthWritten();
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
                mGlobalDirtySet.ForEachActiveObject([&](auto * dirtyPath) {
                    if (dirtyPath->IsAttributePathSupersetOf(readPath))
                    {
                        concretePathDirty = true;
                        return Loop::Break;
                    }
                    return Loop::Continue;
                });

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
            // Load the saved state from previous encoding session for chunking of one single attribute (list chunking).
            AttributeValueEncoder::AttributeEncodeState encodeState = apReadHandler->GetAttributeEncodeState();
            err = RetrieveClusterData(apReadHandler->GetSubjectDescriptor(), apReadHandler->IsFabricFiltered(), attributeReportIBs,
                                      pathForRetrieval, &encodeState);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DataManagement,
                             "Error retrieving data from clusterId: " ChipLogFormatMEI ", err = %" CHIP_ERROR_FORMAT,
                             ChipLogValueMEI(pathForRetrieval.mClusterId), err.Format());

                // If error is not CHIP_ERROR_BUFFER_TOO_SMALL and is not CHIP_ERROR_NO_MEMORY, rollback and encode status.
                // Otherwise, if partial data allowed, save the encode state.
                // Otherwise roll back. If we have already encoded some chunks, we are done; otherwise encode status.

                if (encodeState.AllowPartialData() && ((err == CHIP_ERROR_BUFFER_TOO_SMALL) || (err == CHIP_ERROR_NO_MEMORY)))
                {
                    // Encoding is aborted but partial data is allowed, then we don't rollback and save the state for next chunk.
                    apReadHandler->SetAttributeEncodeState(encodeState);
                }
                else
                {
                    // We met a error during writing reports, one common case is we are running out of buffer, rollback the
                    // attributeReportIB to avoid any partial data.
                    attributeReportIBs.Rollback(attributeBackup);
                    apReadHandler->SetAttributeEncodeState(AttributeValueEncoder::AttributeEncodeState());

                    if (err != CHIP_ERROR_NO_MEMORY && err != CHIP_ERROR_BUFFER_TOO_SMALL)
                    {
                        // Try to encode our error as a status response.
                        err = attributeReportIBs.EncodeAttributeStatus(pathForRetrieval, StatusIB(err));
                        if (err != CHIP_NO_ERROR)
                        {
                            // OK, just roll back again and give up.
                            attributeReportIBs.Rollback(attributeBackup);
                        }
                    }
                }
            }
            SuccessOrExit(err);
            // Successfully encoded the attribute, clear the internal state.
            apReadHandler->SetAttributeEncodeState(AttributeValueEncoder::AttributeEncodeState());
        }
        // We just visited all paths interested by this read handler and did not abort in the middle of iteration, there are no more
        // chunks for this report.
        hasMoreChunks = false;
    }
exit:
    if (attributeReportIBs.GetWriter()->GetLengthWritten() != emptyReportDataLength)
    {
        // We may encounter BUFFER_TOO_SMALL with nothing actually written for the case of list chunking, so we check if we have
        // actually
        attributeDataWritten = true;
    }

    if (apHasEncodedData != nullptr)
    {
        *apHasEncodedData = attributeDataWritten;
    }
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

    // hasMoreChunks + no data encoded is a flag that we have encountered some trouble when processing the attribute.
    // BuildAndSendSingleReportData will abort the read transaction if we encoded no attribute and no events but hasMoreChunks is
    // set.
    if (apHasMoreChunks != nullptr)
    {
        *apHasMoreChunks = hasMoreChunks;
    }

    return err;
}

CHIP_ERROR Engine::BuildSingleReportDataEventReports(ReportDataMessage::Builder & aReportDataBuilder, ReadHandler * apReadHandler,
                                                     bool * apHasMoreChunks, bool * apHasEncodedData)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    size_t eventCount = 0;
    TLV::TLVWriter backup;
    bool eventClean                = true;
    ClusterInfo * clusterInfoList  = apReadHandler->GetEventClusterInfolist();
    EventNumber & eventMin         = apReadHandler->GetEventMin();
    EventManagement & eventManager = EventManagement::GetInstance();
    bool hasMoreChunks             = false;

    aReportDataBuilder.Checkpoint(backup);

    VerifyOrExit(clusterInfoList != nullptr, );

    // If the eventManager is not valid or has not been initialized,
    // skip the rest of processing
    VerifyOrExit(eventManager.IsValid(), ChipLogError(DataManagement, "EventManagement has not yet initialized"));

    eventClean = apReadHandler->CheckEventClean(eventManager);

    // proceed only if there are new events.
    if (eventClean)
    {
        ExitNow(); // Read clean, move along
    }

    {
        EventReportIBs::Builder & eventReportIBs = aReportDataBuilder.CreateEventReports();
        SuccessOrExit(err = aReportDataBuilder.GetError());
        err = eventManager.FetchEventsSince(*(eventReportIBs.GetWriter()), clusterInfoList, eventMin, eventCount,
                                            apReadHandler->GetAccessingFabricIndex());

        if ((err == CHIP_END_OF_TLV) || (err == CHIP_ERROR_TLV_UNDERRUN) || (err == CHIP_NO_ERROR))
        {
            err           = CHIP_NO_ERROR;
            hasMoreChunks = false;
        }
        else if ((err == CHIP_ERROR_BUFFER_TOO_SMALL) || (err == CHIP_ERROR_NO_MEMORY))
        {
            // when first cluster event is too big to fit in the packet, ignore that cluster event.
            if (eventCount == 0)
            {
                eventMin++;
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
                err = CHIP_NO_ERROR;
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

        eventReportIBs.EndOfEventReports();
        SuccessOrExit(err = eventReportIBs.GetError());
    }
    ChipLogDetail(DataManagement, "Fetched %zu events", eventCount);

exit:
    if (apHasEncodedData != nullptr)
    {
        *apHasEncodedData = !(eventCount == 0 || eventClean);
    }

    if (err == CHIP_NO_ERROR && (eventCount == 0 || eventClean))
    {
        aReportDataBuilder.Rollback(backup);
    }

    // hasMoreChunks + no data encoded is a flag that we have encountered some trouble when processing the attribute.
    // BuildAndSendSingleReportData will abort the read transaction if we encoded no attribute and no events but hasMoreChunks is
    // set.
    if (apHasMoreChunks != nullptr)
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

    // Reserved size for the uint8_t InteractionModelRevision flag, which takes up 1 byte for the control tag and 1 byte for the
    // context tag, 1 byte for value
    const uint32_t kReservedSizeForIMRevision = 1 + 1 + 1;

    // Reserved size for the end of report message, which is an end-of-container (i.e 1 byte for the control tag).
    const uint32_t kReservedSizeForEndOfReportMessage = 1;

    VerifyOrExit(apReadHandler != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
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

    if (apReadHandler->IsType(ReadHandler::InteractionType::Subscribe))
    {
        uint64_t subscriptionId = 0;
        apReadHandler->GetSubscriptionId(subscriptionId);
        reportDataBuilder.SubscriptionId(subscriptionId);
    }

    SuccessOrExit(err = reportDataWriter.ReserveBuffer(kReservedSizeForMoreChunksFlag + kReservedSizeForIMRevision +
                                                       kReservedSizeForEndOfReportMessage));

    {
        bool hasMoreChunksForAttributes = false;
        bool hasMoreChunksForEvents     = false;
        bool hasEncodedAttributes       = false;
        bool hasEncodedEvents           = false;

        err = BuildSingleReportDataAttributeReportIBs(reportDataBuilder, apReadHandler, &hasMoreChunksForAttributes,
                                                      &hasEncodedAttributes);
        SuccessOrExit(err);

        err = BuildSingleReportDataEventReports(reportDataBuilder, apReadHandler, &hasMoreChunksForEvents, &hasEncodedEvents);
        SuccessOrExit(err);

        hasMoreChunks = hasMoreChunksForAttributes || hasMoreChunksForEvents;

        if (!hasEncodedAttributes && !hasEncodedEvents && hasMoreChunks)
        {
            ChipLogError(DataManagement,
                         "No data actually encoded but hasMoreChunks flag is set, abort report! (attribute too big?)");
            ExitNow(err = CHIP_ERROR_INCORRECT_STATE);
        }
    }

    SuccessOrExit(reportDataBuilder.GetError());
    SuccessOrExit(err = reportDataWriter.UnreserveBuffer(kReservedSizeForMoreChunksFlag + kReservedSizeForIMRevision +
                                                         kReservedSizeForEndOfReportMessage));
    if (hasMoreChunks)
    {
        reportDataBuilder.MoreChunkedMessages(true);
    }
    else if (apReadHandler->IsType(ReadHandler::InteractionType::Read))
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

    ChipLogDetail(DataManagement, "<RE> Sending report (payload has %" PRIu32 " bytes)...", reportDataWriter.GetLengthWritten());
    err = SendReport(apReadHandler, std::move(bufHandle), hasMoreChunks);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(DataManagement, "<RE> Error sending out report data with %" CHIP_ERROR_FORMAT "!", err.Format()));

    ChipLogDetail(DataManagement, "<RE> ReportsInFlight = %" PRIu32 " with readHandler %" PRIu32 ", RE has %s", mNumReportsInFlight,
                  mCurReadHandlerIdx, hasMoreChunks ? "more messages" : "no more messages");

exit:
    if (err != CHIP_NO_ERROR)
    {
        //
        // WillSendMessage() was called on this EC well before it got here (since there was an intention to generate reports, which
        // occurs asynchronously. Consequently, if any error occurs, it's on us to close down the exchange.
        //
        apReadHandler->Abort();
    }
    else if (apReadHandler->IsType(ReadHandler::InteractionType::Read) && !hasMoreChunks)
    {
        //
        // In the case of successful report generation and we're on the last chunk of a read, we don't expect
        // any further activity on this exchange. The EC layer will automatically close our EC, so shutdown the ReadHandler
        // gracefully.
        //
        apReadHandler->Close();
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

    Messaging::ExchangeManager * exchangeManager = InteractionModelEngine::GetInstance()->GetExchangeManager();
    if (exchangeManager == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    SessionManager * sessionManager = exchangeManager->GetSessionManager();
    if (sessionManager == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    System::Layer * systemLayer = sessionManager->SystemLayer();
    if (systemLayer == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    ReturnErrorOnFailure(systemLayer->ScheduleWork(Run, this));
    mRunScheduled = true;
    return CHIP_NO_ERROR;
}

void Engine::Run()
{
    uint32_t numReadHandled = 0;

    InteractionModelEngine * imEngine = InteractionModelEngine::GetInstance();

    mRunScheduled = false;

    while ((mNumReportsInFlight < CHIP_IM_MAX_REPORTS_IN_FLIGHT) && (numReadHandled < imEngine->mReadHandlers.Allocated()))
    {
        ReadHandler * readHandler = imEngine->ActiveHandlerAt(mCurReadHandlerIdx % (uint32_t) imEngine->mReadHandlers.Allocated());
        VerifyOrDie(readHandler != nullptr);

        if (readHandler->IsReportable())
        {
            CHIP_ERROR err = BuildAndSendSingleReportData(readHandler);
            if (err != CHIP_NO_ERROR)
            {
                return;
            }
        }

        numReadHandled++;
        mCurReadHandlerIdx++;
    }

    //
    // If our tracker has exceeded the bounds of the handler list, reset it back to 0.
    // This isn't strictly necessary, but does make it easier to debug issues in this code if they
    // do arise.
    //
    if (mCurReadHandlerIdx >= imEngine->mReadHandlers.Allocated())
    {
        mCurReadHandlerIdx = 0;
    }

    bool allReadClean = true;

    imEngine->mReadHandlers.ForEachActiveObject([this, &allReadClean](ReadHandler * handler) {
        UpdateReadHandlerDirty(*handler);
        if (handler->IsDirty())
        {
            allReadClean = false;
            return Loop::Break;
        }

        return Loop::Continue;
    });

    if (allReadClean)
    {
        mGlobalDirtySet.ReleaseAll();
    }
}

bool Engine::MergeOverlappedAttributePath(ClusterInfo & aAttributePath)
{
    return Loop::Break == mGlobalDirtySet.ForEachActiveObject([&](auto * path) {
        if (path->IsAttributePathSupersetOf(aAttributePath))
        {
            return Loop::Break;
        }
        if (aAttributePath.IsAttributePathSupersetOf(*path))
        {
            path->mListIndex   = aAttributePath.mListIndex;
            path->mAttributeId = aAttributePath.mAttributeId;
            return Loop::Break;
        }
        return Loop::Continue;
    });
}

CHIP_ERROR Engine::SetDirty(ClusterInfo & aClusterInfo)
{
    InteractionModelEngine::GetInstance()->mReadHandlers.ForEachActiveObject([&aClusterInfo](ReadHandler * handler) {
        // We call SetDirty for both read interactions and subscribe interactions, since we may sent inconsistent attribute data
        // between two chunks. SetDirty will be ignored automatically by read handlers which is waiting for response to last message
        // chunk for read interactions.
        if (handler->IsGeneratingReports() || handler->IsAwaitingReportResponse())
        {
            for (auto clusterInfo = handler->GetAttributeClusterInfolist(); clusterInfo != nullptr;
                 clusterInfo      = clusterInfo->mpNext)
            {
                if (aClusterInfo.IsAttributePathSupersetOf(*clusterInfo) || clusterInfo->IsAttributePathSupersetOf(aClusterInfo))
                {
                    handler->SetDirty();
                    break;
                }
            }
        }

        return Loop::Continue;
    });

    if (!MergeOverlappedAttributePath(aClusterInfo) &&
        InteractionModelEngine::GetInstance()->IsOverlappedAttributePath(aClusterInfo))
    {
        ClusterInfo * clusterInfo = mGlobalDirtySet.CreateObject();
        if (clusterInfo == nullptr)
        {
            ChipLogError(DataManagement, "mGlobalDirtySet pool full, cannot handle more entries!");
            return CHIP_ERROR_NO_MEMORY;
        }
        *clusterInfo = aClusterInfo;
    }

    return CHIP_NO_ERROR;
}

void Engine::UpdateReadHandlerDirty(ReadHandler & aReadHandler)
{
    if (!aReadHandler.IsDirty())
    {
        return;
    }

    if (!aReadHandler.IsType(ReadHandler::InteractionType::Subscribe))
    {
        return;
    }

    bool intersected = false;
    for (auto clusterInfo = aReadHandler.GetAttributeClusterInfolist(); clusterInfo != nullptr; clusterInfo = clusterInfo->mpNext)
    {
        mGlobalDirtySet.ForEachActiveObject([&](auto * path) {
            if (path->IsAttributePathSupersetOf(*clusterInfo) || clusterInfo->IsAttributePathSupersetOf(*path))
            {
                intersected = true;
                return Loop::Break;
            }
            return Loop::Continue;
        });
        if (intersected)
        {
            break;
        }
    }
    if (!intersected)
    {
        ChipLogDetail(InteractionModel, "clear read handler dirty in UpdateReadHandlerDirty!");
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

void Engine::GetMinEventLogPosition(uint32_t & aMinLogPosition)
{
    InteractionModelEngine::GetInstance()->mReadHandlers.ForEachActiveObject([&aMinLogPosition](ReadHandler * handler) {
        if (handler->IsType(ReadHandler::InteractionType::Read))
        {
            return Loop::Continue;
        }

        uint32_t initialWrittenEventsBytes = handler->GetLastWrittenEventsBytes();
        if (initialWrittenEventsBytes < aMinLogPosition)
        {
            aMinLogPosition = initialWrittenEventsBytes;
        }

        return Loop::Continue;
    });
}

CHIP_ERROR Engine::ScheduleBufferPressureEventDelivery(uint32_t aBytesWritten)
{
    uint32_t minEventLogPosition = aBytesWritten;
    GetMinEventLogPosition(minEventLogPosition);
    if (aBytesWritten - minEventLogPosition > CHIP_CONFIG_EVENT_LOGGING_BYTE_THRESHOLD)
    {
        ChipLogProgress(DataManagement, "<RE> Buffer overfilled CHIP_CONFIG_EVENT_LOGGING_BYTE_THRESHOLD %d, schedule engine run",
                        CHIP_CONFIG_EVENT_LOGGING_BYTE_THRESHOLD);
        return ScheduleRun();
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Engine::ScheduleUrgentEventDelivery(ConcreteEventPath & aPath)
{
    InteractionModelEngine::GetInstance()->mReadHandlers.ForEachActiveObject([&aPath](ReadHandler * handler) {
        if (handler->IsType(ReadHandler::InteractionType::Read))
        {
            return Loop::Continue;
        }

        for (auto clusterInfo = handler->GetEventClusterInfolist(); clusterInfo != nullptr; clusterInfo = clusterInfo->mpNext)
        {
            if (clusterInfo->IsEventPathSupersetOf(aPath))
            {
                handler->UnblockUrgentEventDelivery();
                break;
            }
        }

        return Loop::Continue;
    });

    return ScheduleRun();
}

CHIP_ERROR Engine::ScheduleEventDelivery(ConcreteEventPath & aPath, EventOptions::Type aUrgent, uint32_t aBytesWritten)
{
    if (aUrgent != EventOptions::Type::kUrgent)
    {
        return ScheduleBufferPressureEventDelivery(aBytesWritten);
    }
    else
    {
        return ScheduleUrgentEventDelivery(aPath);
    }
    return CHIP_NO_ERROR;
}

}; // namespace reporting
} // namespace app
} // namespace chip

void __attribute__((weak)) MatterPreAttributeReadCallback(const chip::app::ConcreteAttributePath & attributePath) {}
void __attribute__((weak)) MatterPostAttributeReadCallback(const chip::app::ConcreteAttributePath & attributePath) {}
