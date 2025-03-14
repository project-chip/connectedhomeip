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

#include <access/AccessRestrictionProvider.h>
#include <access/Privilege.h>
#include <app/AppConfig.h>
#include <app/AttributePathExpandIterator.h>
#include <app/ConcreteEventPath.h>
#include <app/GlobalAttributes.h>
#include <app/InteractionModelEngine.h>
#include <app/RequiredPrivilege.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/MetadataLookup.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/Provider.h>
#include <app/icd/server/ICDServerConfig.h>
#include <app/reporting/Engine.h>
#include <app/reporting/reporting.h>
#include <app/util/MatterCallbacks.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>
#include <protocols/interaction_model/StatusCode.h>

#include <optional>

#if CHIP_CONFIG_ENABLE_ICD_SERVER
#include <app/icd/server/ICDNotifier.h> // nogncheck
#endif

using namespace chip::Access;

namespace chip {
namespace app {
namespace reporting {
namespace {

using Protocols::InteractionModel::Status;

/// Returns the status of ACL validation.
///   If the return value has a status set, that means the ACL check failed,
///   the read must not be performed, and the returned status (which may
///   be success, when dealing with non-concrete paths) should be used
///   as the status for the read.
///
///   If the returned value is std::nullopt, that means the ACL check passed and the
///   read should proceed.
std::optional<CHIP_ERROR> ValidateReadAttributeACL(DataModel::Provider * dataModel, const SubjectDescriptor & subjectDescriptor,
                                                   const ConcreteReadAttributePath & path)
{

    RequestPath requestPath{ .cluster     = path.mClusterId,
                             .endpoint    = path.mEndpointId,
                             .requestType = RequestType::kAttributeReadRequest,
                             .entityId    = path.mAttributeId };

    DataModel::AttributeFinder finder(dataModel);

    std::optional<DataModel::AttributeEntry> info = finder.Find(path);

    // If the attribute exists, we know whether it is readable (readPrivilege has value)
    // and what the required access privilege is. However for attributes missing from the metatada
    // (e.g. global attributes) or completely missing attributes we do not actually know of a required
    // privilege and default to kView (this is correct for global attributes and a reasonable check
    // for others)
    Privilege requiredPrivilege = Privilege::kView;
    if (info.has_value() && info->readPrivilege.has_value())
    {
        // attribute exists and is readable, set the correct read privilege
        requiredPrivilege = *info->readPrivilege;
    }

    CHIP_ERROR err = GetAccessControl().Check(subjectDescriptor, requestPath, requiredPrivilege);
    if (err == CHIP_NO_ERROR)
    {
        if (IsSupportedGlobalAttributeNotInMetadata(path.mAttributeId))
        {
            // Global attributes passing a kView check is ok
            return std::nullopt;
        }

        // We want to return "success" (i.e. nulopt) IF AND ONLY IF the attribute exists and is readable (has read privilege).
        // Since the Access control check above may have passed with kView, we do another check here:
        //    - Attribute exists (info has value)
        //    - Attribute is readable (readProvilege has value) and not "write only"
        // If the attribute exists and is not readable, we will return UnsupportedRead (spec 8.4.3.2: "Else if the path indicates
        // attribute data that is not readable, an AttributeStatusIB SHALL be generated with the UNSUPPORTED_READ Status Code.")
        //
        // TODO:: https://github.com/CHIP-Specifications/connectedhomeip-spec/pull/9024 requires interleaved ordering that
        //        is NOT implemented here. Spec requires:
        //           - check cluster access check (done here as kView at least)
        //           - unsupported endpoint/cluster/attribute check (NOT done here) when the attribute is missing.
        //             this SHOULD be done here when info does not have a value. This was not done as a first pass to
        //             minimize amount of delta in the initial PR.
        //           - "write-only" attributes should return UNSUPPORTED_READ (this is done here)
        if (info.has_value() && !info->readPrivilege.has_value())
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedRead);
        }

        return std::nullopt;
    }
    VerifyOrReturnError((err == CHIP_ERROR_ACCESS_DENIED) || (err == CHIP_ERROR_ACCESS_RESTRICTED_BY_ARL), err);

    // Implementation of 8.4.3.2 of the spec for path expansion
    if (path.mExpanded)
    {
        return CHIP_NO_ERROR;
    }

    // access denied and access restricted have specific codes for IM
    return err == CHIP_ERROR_ACCESS_DENIED ? CHIP_IM_GLOBAL_STATUS(UnsupportedAccess) : CHIP_IM_GLOBAL_STATUS(AccessRestricted);
}

DataModel::ActionReturnStatus RetrieveClusterData(DataModel::Provider * dataModel, const SubjectDescriptor & subjectDescriptor,
                                                  bool isFabricFiltered, AttributeReportIBs::Builder & reportBuilder,
                                                  const ConcreteReadAttributePath & path, AttributeEncodeState * encoderState)
{
    ChipLogDetail(DataManagement, "<RE:Run> Cluster %" PRIx32 ", Attribute %" PRIx32 " is dirty", path.mClusterId,
                  path.mAttributeId);
    DataModelCallbacks::GetInstance()->AttributeOperation(DataModelCallbacks::OperationType::Read,
                                                          DataModelCallbacks::OperationOrder::Pre, path);

    DataModel::ReadAttributeRequest readRequest;

    readRequest.readFlags.Set(DataModel::ReadFlags::kFabricFiltered, isFabricFiltered);
    readRequest.subjectDescriptor = &subjectDescriptor;
    readRequest.path              = path;

    DataModel::ServerClusterFinder serverClusterFinder(dataModel);

    DataVersion version = 0;
    if (auto clusterInfo = serverClusterFinder.Find(path); clusterInfo.has_value())
    {
        version = clusterInfo->dataVersion;
    }
    else
    {
        ChipLogError(DataManagement, "Read request on unknown cluster - no data version available");
    }

    TLV::TLVWriter checkpoint;
    reportBuilder.Checkpoint(checkpoint);

    DataModel::ActionReturnStatus status(CHIP_NO_ERROR);
    AttributeValueEncoder attributeValueEncoder(reportBuilder, subjectDescriptor, path, version, isFabricFiltered, encoderState);

    // TODO: we explicitly DO NOT validate that path is a valid cluster path (even more, above serverClusterFinder
    //       explicitly ignores that case). This means that global attribute reads as well as ReadAttribute
    //       can be passed invalid paths when an invalid Read is detected and must handle them.
    //
    //       See https://github.com/project-chip/connectedhomeip/issues/37410

    if (auto access_status = ValidateReadAttributeACL(dataModel, subjectDescriptor, path); access_status.has_value())
    {
        status = *access_status;
    }
    else if (IsSupportedGlobalAttributeNotInMetadata(readRequest.path.mAttributeId))
    {
        // Global attributes are NOT directly handled by data model providers, instead
        // the are routed through metadata.
        status = ReadGlobalAttributeFromMetadata(dataModel, readRequest.path, attributeValueEncoder);
    }
    else
    {
        status = dataModel->ReadAttribute(readRequest, attributeValueEncoder);
    }

    if (status.IsSuccess())
    {
        // TODO: this callback being only executed on success is awkward. The Write callback is always done
        //       for both read and write.
        //
        //       For now this preserves existing/previous code logic, however we should consider to ALWAYS
        //       call this.
        DataModelCallbacks::GetInstance()->AttributeOperation(DataModelCallbacks::OperationType::Read,
                                                              DataModelCallbacks::OperationOrder::Post, path);
        return status;
    }

    // Encoder state is relevant for errors in case they are retryable.
    //
    // Generally only out of space encoding errors would be retryable, however we save the state
    // for all errors in case this is information that is useful (retry or error position).
    if (encoderState != nullptr)
    {
        *encoderState = attributeValueEncoder.GetState();
    }

#if CHIP_CONFIG_DATA_MODEL_EXTRA_LOGGING
    // Out of space errors may be chunked data, reporting those cases would be very confusing
    // as they are not fully errors. Report only others (which presumably are not recoverable
    // and will be sent to the client as well).
    if (!status.IsOutOfSpaceEncodingResponse())
    {
        DataModel::ActionReturnStatus::StringStorage storage;
        ChipLogError(DataManagement, "Failed to read attribute: %s", status.c_str(storage));
    }
#endif
    return status;
}

bool IsClusterDataVersionEqualTo(DataModel::Provider * dataModel, const ConcreteClusterPath & path, DataVersion dataVersion)
{
    DataModel::ServerClusterFinder serverClusterFinder(dataModel);
    auto info = serverClusterFinder.Find(path);

    return info.has_value() && (info->dataVersion == dataVersion);
}

} // namespace

Engine::Engine(InteractionModelEngine * apImEngine) : mpImEngine(apImEngine) {}

CHIP_ERROR Engine::Init(EventManagement * apEventManagement)
{
    VerifyOrReturnError(apEventManagement != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mNumReportsInFlight = 0;
    mCurReadHandlerIdx  = 0;
    mpEventManagement   = apEventManagement;

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

bool Engine::IsClusterDataVersionMatch(const SingleLinkedListNode<DataVersionFilter> * aDataVersionFilterList,
                                       const ConcreteReadAttributePath & aPath)
{
    bool existPathMatch       = false;
    bool existVersionMismatch = false;
    for (auto filter = aDataVersionFilterList; filter != nullptr; filter = filter->mpNext)
    {
        if (aPath.mEndpointId == filter->mValue.mEndpointId && aPath.mClusterId == filter->mValue.mClusterId)
        {
            existPathMatch = true;

            if (!IsClusterDataVersionEqualTo(mpImEngine->GetDataModelProvider(),
                                             ConcreteClusterPath(filter->mValue.mEndpointId, filter->mValue.mClusterId),
                                             filter->mValue.mDataVersion.Value()))
            {
                existVersionMismatch = true;
            }
        }
    }
    return existPathMatch && !existVersionMismatch;
}

static bool IsOutOfWriterSpaceError(CHIP_ERROR err)
{
    return err == CHIP_ERROR_NO_MEMORY || err == CHIP_ERROR_BUFFER_TOO_SMALL;
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
    bool reservedEndOfReportIBs                = false;

    aReportDataBuilder.Checkpoint(backup);

    AttributeReportIBs::Builder & attributeReportIBs = aReportDataBuilder.CreateAttributeReportIBs();
    size_t emptyReportDataLength                     = 0;

    SuccessOrExit(err = aReportDataBuilder.GetError());

    emptyReportDataLength = attributeReportIBs.GetWriter()->GetLengthWritten();
    //
    // Reserve enough space for closing out the Report IB list
    //
    SuccessOrExit(err = attributeReportIBs.GetWriter()->ReserveBuffer(kReservedSizeEndOfReportIBs));
    reservedEndOfReportIBs = true;

    {
        // TODO: Figure out how AttributePathExpandIterator should handle read
        // vs write paths.
        ConcreteAttributePath readPath;

        ChipLogDetail(DataManagement,
                      "Building Reports for ReadHandler with LastReportGeneration = 0x" ChipLogFormatX64
                      " DirtyGeneration = 0x" ChipLogFormatX64,
                      ChipLogValueX64(apReadHandler->mPreviousReportsBeginGeneration),
                      ChipLogValueX64(apReadHandler->mDirtyGeneration));

        // This ReadHandler is not generating reports, so we reset the iterator for a clean start.
        if (!apReadHandler->IsReporting())
        {
            apReadHandler->ResetPathIterator();
        }

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        uint32_t attributesRead = 0;
#endif

        // For each path included in the interested path of the read handler...
        for (RollbackAttributePathExpandIterator iterator(mpImEngine->GetDataModelProvider(),
                                                          apReadHandler->AttributeIterationPosition());
             iterator.Next(readPath); iterator.MarkCompleted())
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
            AttributeEncodeState encodeState = apReadHandler->GetAttributeEncodeState();
            DataModel::ActionReturnStatus status =
                RetrieveClusterData(mpImEngine->GetDataModelProvider(), apReadHandler->GetSubjectDescriptor(),
                                    apReadHandler->IsFabricFiltered(), attributeReportIBs, pathForRetrieval, &encodeState);
            if (status.IsError())
            {
                // Operation error set, since this will affect early return or override on status encoding
                // it will also be used for error reporting below.
                err = status.GetUnderlyingError();

                // If error is not an "out of writer space" error, rollback and encode status.
                // Otherwise, if partial data allowed, save the encode state.
                // Otherwise roll back. If we have already encoded some chunks, we are done; otherwise encode status.

                if (encodeState.AllowPartialData() && status.IsOutOfSpaceEncodingResponse())
                {
                    ChipLogDetail(DataManagement,
                                  "List does not fit in packet, chunk between list items for clusterId: " ChipLogFormatMEI
                                  ", attributeId: " ChipLogFormatMEI,
                                  ChipLogValueMEI(pathForRetrieval.mClusterId), ChipLogValueMEI(pathForRetrieval.mAttributeId));
                    // Encoding is aborted but partial data is allowed, then we don't rollback and save the state for next chunk.
                    // The expectation is that RetrieveClusterData has already reset attributeReportIBs to a good state (rolled
                    // back any partially-written AttributeReportIB instances, reset its error status).  Since AllowPartialData()
                    // is true, we may not have encoded a complete attribute value, but we did, if we encoded anything, encode a
                    // set of complete AttributeReportIB instances that represent part of the attribute value.
                    apReadHandler->SetAttributeEncodeState(encodeState);
                }
                else
                {
                    // We met a error during writing reports, one common case is we are running out of buffer, rollback the
                    // attributeReportIB to avoid any partial data.
                    attributeReportIBs.Rollback(attributeBackup);
                    apReadHandler->SetAttributeEncodeState(AttributeEncodeState());

                    if (!status.IsOutOfSpaceEncodingResponse())
                    {
                        ChipLogError(DataManagement,
                                     "Fail to retrieve data, roll back and encode status on clusterId: " ChipLogFormatMEI
                                     ", attributeId: " ChipLogFormatMEI "err = %" CHIP_ERROR_FORMAT,
                                     ChipLogValueMEI(pathForRetrieval.mClusterId), ChipLogValueMEI(pathForRetrieval.mAttributeId),
                                     err.Format());
                        // Try to encode our error as a status response.
                        err = attributeReportIBs.EncodeAttributeStatus(pathForRetrieval, StatusIB(status.GetStatusCode()));
                        if (err != CHIP_NO_ERROR)
                        {
                            // OK, just roll back again and give up; if we still ran out of space we
                            // will send this status response in the next chunk.
                            attributeReportIBs.Rollback(attributeBackup);
                        }
                    }
                    else
                    {
                        ChipLogDetail(DataManagement,
                                      "Next attribute value does not fit in packet, roll back on clusterId: " ChipLogFormatMEI
                                      ", attributeId: " ChipLogFormatMEI ", err = %" CHIP_ERROR_FORMAT,
                                      ChipLogValueMEI(pathForRetrieval.mClusterId), ChipLogValueMEI(pathForRetrieval.mAttributeId),
                                      err.Format());
                    }
                }
            }
            SuccessOrExit(err);
            // Successfully encoded the attribute, clear the internal state.
            apReadHandler->SetAttributeEncodeState(AttributeEncodeState());
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
    if (IsOutOfWriterSpaceError(err) && reservedEndOfReportIBs)
    {
        ChipLogDetail(DataManagement, "<RE:Run> We cannot put more chunks into this report. Enable chunking.");
        err = CHIP_NO_ERROR;
    }

    //
    // Only close out the report if we haven't hit an error yet so far.
    //
    if (err == CHIP_NO_ERROR)
    {
        attributeReportIBs.GetWriter()->UnreserveBuffer(kReservedSizeEndOfReportIBs);

        err = attributeReportIBs.EndOfAttributeReportIBs();

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

CHIP_ERROR Engine::CheckAccessDeniedEventPaths(TLV::TLVWriter & aWriter, bool & aHasEncodedData, ReadHandler * apReadHandler)
{
    using Protocols::InteractionModel::Status;

    CHIP_ERROR err = CHIP_NO_ERROR;
    for (auto current = apReadHandler->mpEventPathList; current != nullptr;)
    {
        if (current->mValue.IsWildcardPath())
        {
            current = current->mpNext;
            continue;
        }

        ConcreteEventPath path(current->mValue.mEndpointId, current->mValue.mClusterId, current->mValue.mEventId);

        // A event path is valid only if the cluster is valid
        Status status = DataModel::ValidateClusterPath(mpImEngine->GetDataModelProvider(), path, Status::Success);

        if (status != Status::Success)
        {
            TLV::TLVWriter checkpoint = aWriter;
            err                       = EventReportIB::ConstructEventStatusIB(aWriter, path, StatusIB(status));
            if (err != CHIP_NO_ERROR)
            {
                aWriter = checkpoint;
                break;
            }
            aHasEncodedData = true;
        }

        RequestPath requestPath{ .cluster     = current->mValue.mClusterId,
                                 .endpoint    = current->mValue.mEndpointId,
                                 .requestType = RequestType::kEventReadRequest,
                                 .entityId    = current->mValue.mEventId };
        Privilege requestPrivilege = RequiredPrivilege::ForReadEvent(path);

        err = GetAccessControl().Check(apReadHandler->GetSubjectDescriptor(), requestPath, requestPrivilege);
        if ((err != CHIP_ERROR_ACCESS_DENIED) && (err != CHIP_ERROR_ACCESS_RESTRICTED_BY_ARL))
        {
            ReturnErrorOnFailure(err);
        }
        else
        {
            TLV::TLVWriter checkpoint = aWriter;
            err                       = EventReportIB::ConstructEventStatusIB(aWriter, path,
                                                        err == CHIP_ERROR_ACCESS_DENIED ? StatusIB(Status::UnsupportedAccess)
                                                                                                              : StatusIB(Status::AccessRestricted));

            if (err != CHIP_NO_ERROR)
            {
                aWriter = checkpoint;
                break;
            }
            aHasEncodedData = true;
            ChipLogDetail(InteractionModel, "Access to event (%u, " ChipLogFormatMEI ", " ChipLogFormatMEI ") denied by %s",
                          current->mValue.mEndpointId, ChipLogValueMEI(current->mValue.mClusterId),
                          ChipLogValueMEI(current->mValue.mEventId), err == CHIP_ERROR_ACCESS_DENIED ? "ACL" : "ARL");
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
    bool eventClean    = true;
    auto & eventMin    = apReadHandler->GetEventMin();
    bool hasMoreChunks = false;

    aReportDataBuilder.Checkpoint(backup);

    VerifyOrExit(apReadHandler->GetEventPathList() != nullptr, );

    // If the mpEventManagement is not valid or has not been initialized,
    // skip the rest of processing
    VerifyOrExit(mpEventManagement != nullptr && mpEventManagement->IsValid(),
                 ChipLogError(DataManagement, "EventManagement has not yet initialized"));

    eventClean = apReadHandler->CheckEventClean(*mpEventManagement);

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

        err = mpEventManagement->FetchEventsSince(*(eventReportIBs.GetWriter()), apReadHandler->GetEventPathList(), eventMin,
                                                  eventCount, apReadHandler->GetSubjectDescriptor());

        if ((err == CHIP_END_OF_TLV) || (err == CHIP_ERROR_TLV_UNDERRUN) || (err == CHIP_NO_ERROR))
        {
            err           = CHIP_NO_ERROR;
            hasMoreChunks = false;
        }
        else if (IsOutOfWriterSpaceError(err))
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
        SuccessOrExit(err = eventReportIBs.EndOfEventReports());
    }
    ChipLogDetail(DataManagement, "Fetched %u events", static_cast<unsigned int>(eventCount));

exit:
    if (apHasEncodedData != nullptr)
    {
        *apHasEncodedData = hasEncodedStatus || (eventCount != 0);
    }

    // Maybe encoding the attributes has already used up all space.
    if ((err == CHIP_NO_ERROR || IsOutOfWriterSpaceError(err)) && !(hasEncodedStatus || (eventCount != 0)))
    {
        aReportDataBuilder.Rollback(backup);
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
    System::PacketBufferTLVWriter reportDataWriter;
    ReportDataMessage::Builder reportDataBuilder;
    System::PacketBufferHandle bufHandle = nullptr;
    uint16_t reservedSize                = 0;
    bool hasMoreChunks                   = false;
    bool needCloseReadHandler            = false;
    size_t reportBufferMaxSize           = 0;

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

    reportBufferMaxSize = apReadHandler->GetReportBufferMaxSize();

    bufHandle = System::PacketBufferHandle::New(reportBufferMaxSize);
    VerifyOrExit(!bufHandle.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    if (bufHandle->AvailableDataLength() > reportBufferMaxSize)
    {
        reservedSize = static_cast<uint16_t>(bufHandle->AvailableDataLength() - reportBufferMaxSize);
    }

    reportDataWriter.Init(std::move(bufHandle));

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    reportDataWriter.ReserveBuffer(mReservedSize);
#endif

    // Always limit the size of the generated packet to fit within the max size returned by the ReadHandler regardless
    // of the available buffer capacity.
    // Also, we need to reserve some extra space for the MIC field.
    reportDataWriter.ReserveBuffer(static_cast<uint32_t>(reservedSize + Crypto::CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES));

    // Create a report data.
    err = reportDataBuilder.Init(&reportDataWriter);
    SuccessOrExit(err);

    if (apReadHandler->IsType(ReadHandler::InteractionType::Subscribe))
    {
#if CHIP_CONFIG_ENABLE_ICD_SERVER
        // Notify the ICDManager that we are about to send a subscription report before we prepare the Report payload.
        // This allows the ICDManager to trigger any necessary updates and have the information in the report about to be sent.
        app::ICDNotifier::GetInstance().NotifySubscriptionReport();
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

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

    SuccessOrExit(err = reportDataBuilder.GetError());
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
    if (IsRunScheduled())
    {
        return CHIP_NO_ERROR;
    }

    Messaging::ExchangeManager * exchangeManager = mpImEngine->GetExchangeManager();
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

    // We may be deallocating read handlers as we go.  Track how many we had
    // initially, so we make sure to go through all of them.
    size_t initialAllocated = mpImEngine->mReadHandlers.Allocated();
    while ((mNumReportsInFlight < CHIP_IM_MAX_REPORTS_IN_FLIGHT) && (numReadHandled < initialAllocated))
    {
        ReadHandler * readHandler =
            mpImEngine->ActiveHandlerAt(mCurReadHandlerIdx % (uint32_t) mpImEngine->mReadHandlers.Allocated());
        VerifyOrDie(readHandler != nullptr);

        if (readHandler->ShouldReportUnscheduled() || mpImEngine->GetReportScheduler()->IsReportableNow(readHandler))
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
    if (mCurReadHandlerIdx >= mpImEngine->mReadHandlers.Allocated())
    {
        mCurReadHandlerIdx = 0;
    }

    bool allReadClean = true;

    mpImEngine->mReadHandlers.ForEachActiveObject([&allReadClean](ReadHandler * handler) {
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
    VerifyOrReturnError(!MergeOverlappedAttributePath(aAttributePath), CHIP_NO_ERROR);

    if (mGlobalDirtySet.Exhausted() && !MergeDirtyPathsUnderSameCluster() && !MergeDirtyPathsUnderSameEndpoint())
    {
        ChipLogDetail(DataManagement, "Global dirty set pool exhausted, merge all paths.");
        mGlobalDirtySet.ReleaseAll();
        auto object         = mGlobalDirtySet.CreateObject();
        object->mGeneration = GetDirtySetGeneration();
    }

    VerifyOrReturnError(!MergeOverlappedAttributePath(aAttributePath), CHIP_NO_ERROR);
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

CHIP_ERROR Engine::SetDirty(const AttributePathParams & aAttributePath)
{
    BumpDirtySetGeneration();

    bool intersectsInterestPath     = false;
    DataModel::Provider * dataModel = mpImEngine->GetDataModelProvider();
    mpImEngine->mReadHandlers.ForEachActiveObject([&dataModel, &aAttributePath, &intersectsInterestPath](ReadHandler * handler) {
        // We call AttributePathIsDirty for both read interactions and subscribe interactions, since we may send inconsistent
        // attribute data between two chunks. AttributePathIsDirty will not schedule a new run for read handlers which are
        // waiting for a response to the last message chunk for read interactions.
        if (handler->CanStartReporting() || handler->IsAwaitingReportResponse())
        {
            for (auto object = handler->GetAttributePathList(); object != nullptr; object = object->mpNext)
            {
                if (object->mValue.Intersects(aAttributePath))
                {
                    handler->AttributePathIsDirty(dataModel, aAttributePath);
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
    mpImEngine->mReadHandlers.ForEachActiveObject([&aMinLogPosition](ReadHandler * handler) {
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

CHIP_ERROR Engine::NewEventGenerated(ConcreteEventPath & aPath, uint32_t aBytesConsumed)
{
    // If we literally have no read handlers right now that care about any events,
    // we don't need to call schedule run for event.
    // If schedule run is called, actually we would not delivery events as well.
    // Just wanna save one schedule run here
    if (mpImEngine->mEventPathPool.Allocated() == 0)
    {
        return CHIP_NO_ERROR;
    }

    bool isUrgentEvent = false;
    mpImEngine->mReadHandlers.ForEachActiveObject([&aPath, &isUrgentEvent](ReadHandler * handler) {
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
                handler->ForceDirtyState();
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

    return ScheduleBufferPressureEventDelivery(aBytesConsumed);
}

void Engine::ScheduleUrgentEventDeliverySync(Optional<FabricIndex> fabricIndex)
{
    mpImEngine->mReadHandlers.ForEachActiveObject([fabricIndex](ReadHandler * handler) {
        if (handler->IsType(ReadHandler::InteractionType::Read))
        {
            return Loop::Continue;
        }

        if (fabricIndex.HasValue() && fabricIndex.Value() != handler->GetAccessingFabricIndex())
        {
            return Loop::Continue;
        }

        handler->ForceDirtyState();

        return Loop::Continue;
    });

    Run();
}

void Engine::MarkDirty(const AttributePathParams & path)
{
    CHIP_ERROR err = SetDirty(path);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DataManagement, "Failed to set path dirty: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

} // namespace reporting
} // namespace app
} // namespace chip

// TODO: MatterReportingAttributeChangeCallback should just live in libCHIP, It does not depend on any
// app-specific generated bits.
void __attribute__((weak))
MatterReportingAttributeChangeCallback(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId)
{}
