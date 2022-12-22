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

#include <app/AppConfig.h>
#include <app/InteractionModelEngine.h>
#include <app/RequiredPrivilege.h>
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
    // Flush out the event buffer synchronously
    ScheduleUrgentEventDeliverySync();

    mNumReportsInFlight = 0;
    mCurReadHandlerIdx  = 0;
    mGlobalDirtySet.ReleaseAll();
}

bool Engine::IsClusterDataVersionMatch(const ObjectList<DataVersionFilter> * aDataVersionFilterList,
                                       const ConcreteReadAttributePath & aPath)
{
    bool existPathMatch       = false;
    bool existVersionMismatch = false;
    for (auto filter = aDataVersionFilterList; filter != nullptr; filter = filter->mpNext)
    {
        if (aPath.mEndpointId == filter->mValue.mEndpointId && aPath.mClusterId == filter->mValue.mClusterId)
        {
            existPathMatch = true;
            if (!IsClusterDataVersionEqual(ConcreteClusterPath(filter->mValue.mEndpointId, filter->mValue.mClusterId),
                                           filter->mValue.mDataVersion.Value()))
            {
                existVersionMismatch = true;
            }
        }
    }
    return existPathMatch && !existVersionMismatch;
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

        ChipLogDetail(DataManagement,
                      "Building Reports for ReadHandler with LastReportGeneration = %" PRIu64 " DirtyGeneration = %" PRIu64,
                      apReadHandler->mPreviousReportsBeginGeneration, apReadHandler->mDirtyGeneration);

        // This ReadHandler is not generating reports, so we reset the iterator for a clean start.
        if (!apReadHandler->IsReporting())
        {
            apReadHandler->ResetPathIterator();
        }

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        uint32_t attributesRead = 0;
#endif

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
                        // We don't need to worry about paths that were already marked dirty before the last time this read handler
                        // started a report that it completed: those paths already got reported.
                        if (dirtyPath->mGeneration > apReadHandler->mPreviousReportsBeginGeneration)
                        {
                            concretePathDirty = true;
                            return Loop::Break;
                        }
                    }
                    return Loop::Continue;
                });

                if (!concretePathDirty)
                {
                    // This attribute is not dirty, we just skip this one.
                    continue;
                }
            }
            else
            {
                if (IsClusterDataVersionMatch(apReadHandler->GetDataVersionFilterList(), readPath))
                {
                    continue;
                }
            }

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
            attributesRead++;
            if (attributesRead > mMaxAttributesPerChunk)
            {
                ExitNow(err = CHIP_ERROR_BUFFER_TOO_SMALL);
            }
#endif

            // If we are processing a read request, or the initial report of a subscription, just regard all paths as dirty
            // paths.
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
        aReportDataBuilder.ResetError();
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

CHIP_ERROR Engine::CheckAccessDeniedEventPaths(TLV::TLVWriter & aWriter, bool & aHasEncodedData, ReadHandler * apReadHandler)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    for (auto current = apReadHandler->mpEventPathList; current != nullptr;)
    {
        if (current->mValue.HasEventWildcard())
        {
            current = current->mpNext;
            continue;
        }

        Access::RequestPath requestPath{ .cluster = current->mValue.mClusterId, .endpoint = current->mValue.mEndpointId };
        ConcreteEventPath path(current->mValue.mEndpointId, current->mValue.mClusterId, current->mValue.mEventId);
        Access::Privilege requestPrivilege = RequiredPrivilege::ForReadEvent(path);

        err = Access::GetAccessControl().Check(apReadHandler->GetSubjectDescriptor(), requestPath, requestPrivilege);
        if (err != CHIP_ERROR_ACCESS_DENIED)
        {
            ReturnErrorOnFailure(err);
        }
        else
        {
            TLV::TLVWriter checkpoint = aWriter;
            err                       = EventReportIB::ConstructEventStatusIB(aWriter, path,
                                                        StatusIB(Protocols::InteractionModel::Status::UnsupportedAccess));
            if (err != CHIP_NO_ERROR)
            {
                aWriter = checkpoint;
                break;
            }
            aHasEncodedData = true;
            ChipLogDetail(InteractionModel, "Acces to event (%u, " ChipLogFormatMEI ", " ChipLogFormatMEI ") denied by ACL",
                          current->mValue.mEndpointId, ChipLogValueMEI(current->mValue.mClusterId),
                          ChipLogValueMEI(current->mValue.mEventId));
        }
        current = current->mpNext;
    }

    return err;
}

CHIP_ERROR Engine::BuildSingleReportDataEventReports(ReportDataMessage::Builder & aReportDataBuilder, ReadHandler * apReadHandler,
                                                     bool aBufferIsUsed, bool * apHasMoreChunks, bool * apHasEncodedData)
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    size_t eventCount     = 0;
    bool hasEncodedStatus = false;
    TLV::TLVWriter backup;
    bool eventClean                = true;
    auto & eventMin                = apReadHandler->GetEventMin();
    EventManagement & eventManager = EventManagement::GetInstance();
    bool hasMoreChunks             = false;

    aReportDataBuilder.Checkpoint(backup);

    VerifyOrExit(apReadHandler->GetEventPathList() != nullptr, );

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
        // Just like what we do in BuildSingleReportDataAttributeReportIBs(), we need to reserve one byte for end of container tag
        // when encoding events to ensure we can close the container successfully.
        const uint32_t kReservedSizeEndOfReportIBs = 1;
        EventReportIBs::Builder & eventReportIBs   = aReportDataBuilder.CreateEventReports();
        SuccessOrExit(err = aReportDataBuilder.GetError());
        VerifyOrExit(eventReportIBs.GetWriter() != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        SuccessOrExit(err = eventReportIBs.GetWriter()->ReserveBuffer(kReservedSizeEndOfReportIBs));

        err = CheckAccessDeniedEventPaths(*(eventReportIBs.GetWriter()), hasEncodedStatus, apReadHandler);
        SuccessOrExit(err);

        err = eventManager.FetchEventsSince(*(eventReportIBs.GetWriter()), apReadHandler->GetEventPathList(), eventMin, eventCount,
                                            apReadHandler->GetSubjectDescriptor());

        if ((err == CHIP_END_OF_TLV) || (err == CHIP_ERROR_TLV_UNDERRUN) || (err == CHIP_NO_ERROR))
        {
            err           = CHIP_NO_ERROR;
            hasMoreChunks = false;
        }
        else if ((err == CHIP_ERROR_BUFFER_TOO_SMALL) || (err == CHIP_ERROR_NO_MEMORY))
        {
            // when first cluster event is too big to fit in the packet, ignore that cluster event.
            // However, we may have encoded some attributes before, we don't skip it in that case.
            if (eventCount == 0)
            {
                if (!aBufferIsUsed)
                {
                    eventMin++;
                }
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

        SuccessOrExit(err = eventReportIBs.GetWriter()->UnreserveBuffer(kReservedSizeEndOfReportIBs));
        eventReportIBs.EndOfEventReports();
        SuccessOrExit(err = eventReportIBs.GetError());
    }
    ChipLogDetail(DataManagement, "Fetched %u events", static_cast<unsigned int>(eventCount));

exit:
    if (apHasEncodedData != nullptr)
    {
        *apHasEncodedData = hasEncodedStatus || (eventCount != 0);
    }

    // Maybe encoding the attributes has already used up all space.
    if ((err == CHIP_NO_ERROR || err == CHIP_ERROR_NO_MEMORY || err == CHIP_ERROR_BUFFER_TOO_SMALL) &&
        !(hasEncodedStatus || (eventCount != 0)))
    {
        aReportDataBuilder.Rollback(backup);
        aReportDataBuilder.ResetError();
        err = CHIP_NO_ERROR;
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
    bool needCloseReadHandler                  = false;

    // Reserved size for the MoreChunks boolean flag, which takes up 1 byte for the control tag and 1 byte for the context tag.
    const uint32_t kReservedSizeForMoreChunksFlag = 1 + 1;

    // Reserved size for the uint8_t InteractionModelRevision flag, which takes up 1 byte for the control tag and 1 byte for the
    // context tag, 1 byte for value
    const uint32_t kReservedSizeForIMRevision = 1 + 1 + 1;

    // Reserved size for the end of report message, which is an end-of-container (i.e 1 byte for the control tag).
    const uint32_t kReservedSizeForEndOfReportMessage = 1;

    // Reserved size for an empty EventReportIBs, so we can at least check if there are any events need to be reported.
    const uint32_t kReservedSizeForEventReportIBs = 3; // type, tag, end of container

    VerifyOrExit(apReadHandler != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(apReadHandler->GetSession() != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(!bufHandle.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    if (bufHandle->AvailableDataLength() > kMaxSecureSduLengthBytes)
    {
        reservedSize = static_cast<uint16_t>(bufHandle->AvailableDataLength() - kMaxSecureSduLengthBytes);
    }

    reportDataWriter.Init(std::move(bufHandle));

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
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
        SubscriptionId subscriptionId = 0;
        apReadHandler->GetSubscriptionId(subscriptionId);
        reportDataBuilder.SubscriptionId(subscriptionId);
    }

    SuccessOrExit(err = reportDataWriter.ReserveBuffer(kReservedSizeForMoreChunksFlag + kReservedSizeForIMRevision +
                                                       kReservedSizeForEndOfReportMessage + kReservedSizeForEventReportIBs));

    {
        bool hasMoreChunksForAttributes = false;
        bool hasMoreChunksForEvents     = false;
        bool hasEncodedAttributes       = false;
        bool hasEncodedEvents           = false;

        err = BuildSingleReportDataAttributeReportIBs(reportDataBuilder, apReadHandler, &hasMoreChunksForAttributes,
                                                      &hasEncodedAttributes);
        SuccessOrExit(err);
        SuccessOrExit(err = reportDataWriter.UnreserveBuffer(kReservedSizeForEventReportIBs));
        err = BuildSingleReportDataEventReports(reportDataBuilder, apReadHandler, hasEncodedAttributes, &hasMoreChunksForEvents,
                                                &hasEncodedEvents);
        SuccessOrExit(err);

        hasMoreChunks = hasMoreChunksForAttributes || hasMoreChunksForEvents;

        if (!hasEncodedAttributes && !hasEncodedEvents && hasMoreChunks)
        {
            ChipLogError(DataManagement,
                         "No data actually encoded but hasMoreChunks flag is set, close read handler! (attribute too big?)");
            err = apReadHandler->SendStatusReport(Protocols::InteractionModel::Status::ResourceExhausted);
            if (err == CHIP_NO_ERROR)
            {
                needCloseReadHandler = true;
            }
            ExitNow();
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
    if (err != CHIP_NO_ERROR || (apReadHandler->IsType(ReadHandler::InteractionType::Read) && !hasMoreChunks) ||
        needCloseReadHandler)
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
    pEngine->mRunScheduled = false;
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

    // We may be deallocating read handlers as we go.  Track how many we had
    // initially, so we make sure to go through all of them.
    size_t initialAllocated = imEngine->mReadHandlers.Allocated();
    while ((mNumReportsInFlight < CHIP_IM_MAX_REPORTS_IN_FLIGHT) && (numReadHandled < initialAllocated))
    {
        ReadHandler * readHandler = imEngine->ActiveHandlerAt(mCurReadHandlerIdx % (uint32_t) imEngine->mReadHandlers.Allocated());
        VerifyOrDie(readHandler != nullptr);

        if (readHandler->IsReportable())
        {
            mRunningReadHandler = readHandler;
            CHIP_ERROR err      = BuildAndSendSingleReportData(readHandler);
            mRunningReadHandler = nullptr;
            if (err != CHIP_NO_ERROR)
            {
                return;
            }
        }

        numReadHandled++;
        // If readHandler removed itself from our list, we also decremented
        // mCurReadHandlerIdx to account for that removal, so it's safe to
        // increment here.
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

    imEngine->mReadHandlers.ForEachActiveObject([&allReadClean](ReadHandler * handler) {
        if (handler->IsDirty())
        {
            allReadClean = false;
            return Loop::Break;
        }

        return Loop::Continue;
    });

    if (allReadClean)
    {
        ChipLogDetail(DataManagement, "All ReadHandler-s are clean, clear GlobalDirtySet");

        mGlobalDirtySet.ReleaseAll();
    }
}

bool Engine::MergeOverlappedAttributePath(const AttributePathParams & aAttributePath)
{
    return Loop::Break == mGlobalDirtySet.ForEachActiveObject([&](auto * path) {
        if (path->IsAttributePathSupersetOf(aAttributePath))
        {
            path->mGeneration = GetDirtySetGeneration();
            return Loop::Break;
        }
        if (aAttributePath.IsAttributePathSupersetOf(*path))
        {
            // TODO: the wildcard input path may be superset of next paths in globalDirtySet, it is fine at this moment, since
            // when building report, it would use the first path of globalDirtySet to compare against interested paths read clients
            // want.
            // It is better to eliminate the duplicate wildcard paths in follow-up
            path->mGeneration  = GetDirtySetGeneration();
            path->mEndpointId  = aAttributePath.mEndpointId;
            path->mClusterId   = aAttributePath.mClusterId;
            path->mListIndex   = aAttributePath.mListIndex;
            path->mAttributeId = aAttributePath.mAttributeId;
            return Loop::Break;
        }
        return Loop::Continue;
    });
}

bool Engine::ClearTombPaths()
{
    bool pathReleased = false;
    mGlobalDirtySet.ForEachActiveObject([&](auto * path) {
        if (path->mGeneration == 0)
        {
            mGlobalDirtySet.ReleaseObject(path);
            pathReleased = true;
        }
        return Loop::Continue;
    });
    return pathReleased;
}

bool Engine::MergeDirtyPathsUnderSameCluster()
{
    mGlobalDirtySet.ForEachActiveObject([&](auto * outerPath) {
        if (outerPath->HasWildcardClusterId() || outerPath->mGeneration == 0)
        {
            return Loop::Continue;
        }
        mGlobalDirtySet.ForEachActiveObject([&](auto * innerPath) {
            if (innerPath == outerPath)
            {
                return Loop::Continue;
            }
            // We don't support paths with a wildcard endpoint + a concrete cluster in global dirty set, so we do a simple == check
            // here.
            if (innerPath->mEndpointId != outerPath->mEndpointId || innerPath->mClusterId != outerPath->mClusterId)
            {
                return Loop::Continue;
            }
            if (innerPath->mGeneration > outerPath->mGeneration)
            {
                outerPath->mGeneration = innerPath->mGeneration;
            }
            outerPath->SetWildcardAttributeId();

            // The object pool does not allow us to release objects in a nested iteration, mark the path as a tomb by setting its
            // generation to 0 and then clear it later.
            innerPath->mGeneration = 0;
            return Loop::Continue;
        });
        return Loop::Continue;
    });

    return ClearTombPaths();
}

bool Engine::MergeDirtyPathsUnderSameEndpoint()
{
    mGlobalDirtySet.ForEachActiveObject([&](auto * outerPath) {
        if (outerPath->HasWildcardEndpointId() || outerPath->mGeneration == 0)
        {
            return Loop::Continue;
        }
        mGlobalDirtySet.ForEachActiveObject([&](auto * innerPath) {
            if (innerPath == outerPath)
            {
                return Loop::Continue;
            }
            if (innerPath->mEndpointId != outerPath->mEndpointId)
            {
                return Loop::Continue;
            }
            if (innerPath->mGeneration > outerPath->mGeneration)
            {
                outerPath->mGeneration = innerPath->mGeneration;
            }
            outerPath->SetWildcardClusterId();
            outerPath->SetWildcardAttributeId();

            // The object pool does not allow us to release objects in a nested iteration, mark the path as a tomb by setting its
            // generation to 0 and then clear it later.
            innerPath->mGeneration = 0;
            return Loop::Continue;
        });
        return Loop::Continue;
    });
    return ClearTombPaths();
}

CHIP_ERROR Engine::InsertPathIntoDirtySet(const AttributePathParams & aAttributePath)
{
    ReturnErrorCodeIf(MergeOverlappedAttributePath(aAttributePath), CHIP_NO_ERROR);

    if (mGlobalDirtySet.Exhausted() && !MergeDirtyPathsUnderSameCluster() && !MergeDirtyPathsUnderSameEndpoint())
    {
        ChipLogDetail(DataManagement, "Global dirty set pool exhausted, merge all paths.");
        mGlobalDirtySet.ReleaseAll();
        auto object         = mGlobalDirtySet.CreateObject();
        object->mGeneration = GetDirtySetGeneration();
    }

    ReturnErrorCodeIf(MergeOverlappedAttributePath(aAttributePath), CHIP_NO_ERROR);
    ChipLogDetail(DataManagement, "Cannot merge the new path into any existing path, create one.");

    auto object = mGlobalDirtySet.CreateObject();
    if (object == nullptr)
    {
        // This should not happen, this path should be merged into the wildcard endpoint at least.
        ChipLogError(DataManagement, "mGlobalDirtySet pool full, cannot handle more entries!");
        return CHIP_ERROR_NO_MEMORY;
    }
    *object             = aAttributePath;
    object->mGeneration = GetDirtySetGeneration();

    return CHIP_NO_ERROR;
}

CHIP_ERROR Engine::SetDirty(AttributePathParams & aAttributePath)
{
    BumpDirtySetGeneration();

    bool intersectsInterestPath = false;
    InteractionModelEngine::GetInstance()->mReadHandlers.ForEachActiveObject(
        [&aAttributePath, &intersectsInterestPath](ReadHandler * handler) {
            // We call SetDirty for both read interactions and subscribe interactions, since we may send inconsistent attribute data
            // between two chunks. SetDirty will be ignored automatically by read handlers which are waiting for a response to the
            // last message chunk for read interactions.
            if (handler->IsGeneratingReports() || handler->IsAwaitingReportResponse())
            {
                for (auto object = handler->GetAttributePathList(); object != nullptr; object = object->mpNext)
                {
                    if (object->mValue.Intersects(aAttributePath))
                    {
                        handler->SetDirty(aAttributePath);
                        intersectsInterestPath = true;
                        break;
                    }
                }
            }

            return Loop::Continue;
        });

    if (!intersectsInterestPath)
    {
        return CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(InsertPathIntoDirtySet(aAttributePath));

    return CHIP_NO_ERROR;
}

CHIP_ERROR Engine::SendReport(ReadHandler * apReadHandler, System::PacketBufferHandle && aPayload, bool aHasMoreChunks)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // We can only have 1 report in flight for any given read - increment and break out.
    mNumReportsInFlight++;
    err = apReadHandler->SendReportData(std::move(aPayload), aHasMoreChunks);
    if (err != CHIP_NO_ERROR)
    {
        --mNumReportsInFlight;
    }
    return err;
}

void Engine::OnReportConfirm()
{
    VerifyOrDie(mNumReportsInFlight > 0);

    if (mNumReportsInFlight == CHIP_IM_MAX_REPORTS_IN_FLIGHT)
    {
        // We could have other things waiting to go now that this report is no
        // longer in flight.
        ScheduleRun();
    }
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
        ChipLogDetail(DataManagement, "<RE> Buffer overfilled CHIP_CONFIG_EVENT_LOGGING_BYTE_THRESHOLD %d, schedule engine run",
                      CHIP_CONFIG_EVENT_LOGGING_BYTE_THRESHOLD);
        return ScheduleRun();
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Engine::ScheduleEventDelivery(ConcreteEventPath & aPath, uint32_t aBytesWritten)
{
    // If we literally have no read handlers right now that care about any events,
    // we don't need to call schedule run for event.
    // If schedule run is called, actually we would not delivery events as well.
    // Just wanna save one schedule run here
    if (InteractionModelEngine::GetInstance()->mEventPathPool.Allocated() == 0)
    {
        return CHIP_NO_ERROR;
    }

    bool isUrgentEvent = false;
    InteractionModelEngine::GetInstance()->mReadHandlers.ForEachActiveObject([&aPath, &isUrgentEvent](ReadHandler * handler) {
        if (handler->IsType(ReadHandler::InteractionType::Read))
        {
            return Loop::Continue;
        }

        for (auto * interestedPath = handler->GetEventPathList(); interestedPath != nullptr;
             interestedPath        = interestedPath->mpNext)
        {
            if (interestedPath->mValue.IsEventPathSupersetOf(aPath) && interestedPath->mValue.mIsUrgentEvent)
            {
                isUrgentEvent = true;
                handler->UnblockUrgentEventDelivery();
                break;
            }
        }

        return Loop::Continue;
    });

    if (isUrgentEvent)
    {
        ChipLogDetail(DataManagement, "Urgent event will be sent once reporting is not blocked by the min interval");
        return CHIP_NO_ERROR;
    }

    return ScheduleBufferPressureEventDelivery(aBytesWritten);
}

void Engine::ScheduleUrgentEventDeliverySync(Optional<FabricIndex> fabricIndex)
{
    InteractionModelEngine::GetInstance()->mReadHandlers.ForEachActiveObject([fabricIndex](ReadHandler * handler) {
        if (handler->IsType(ReadHandler::InteractionType::Read))
        {
            return Loop::Continue;
        }

        if (fabricIndex.HasValue() && fabricIndex.Value() != handler->GetAccessingFabricIndex())
        {
            return Loop::Continue;
        }

        handler->UnblockUrgentEventDelivery();

        return Loop::Continue;
    });

    Run();
}

}; // namespace reporting
} // namespace app
} // namespace chip

void __attribute__((weak)) MatterPreAttributeReadCallback(const chip::app::ConcreteAttributePath & attributePath) {}
void __attribute__((weak)) MatterPostAttributeReadCallback(const chip::app::ConcreteAttributePath & attributePath) {}

// TODO: MatterReportingAttributeChangeCallback should just live in libCHIP,
// instead of being in ember-compatibility-functions.  It does not depend on any
// app-specific generated bits.
void __attribute__((weak))
MatterReportingAttributeChangeCallback(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId)
{}
