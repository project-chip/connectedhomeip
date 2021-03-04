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
#include <app/reporting/LoggingManagement.h>
#include <app/reporting/ReportingEngine.h>

namespace chip {
namespace app {
namespace reporting {
CHIP_ERROR ReportingEngine::Init()
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    mMoreChunkedMessages = false;
    mNumReportsInFlight  = 0;
    mCurReadHandlerIdx   = 0;
    return err;
}

CHIP_ERROR ReportingEngine::BuildSingleReportDataEventList(ReportData::Builder & aReportDataBuilder, ReadHandler * apReadHandler)
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    chip::EventNumber event_count = 0;
    bool dataClean;
    chip::TLV::TLVWriter backup;
    chip::EventNumber initialEvents[CHIP_NUM_PRIORITY_LEVEL];

    aReportDataBuilder.Checkpoint(backup);

    memcpy(initialEvents, apReadHandler->mSelfVendedEvents, sizeof(initialEvents));

    EventList::Builder eventList = aReportDataBuilder.CreateEventDataListBuilder();

    // TODO:: Add mechanism to get particular set of interested clusters, currently it retrieve all events
    if (apReadHandler->mGetToAllEvents)
    {
        // Verify that we have events to transmit
        LoggingManagement & logger = LoggingManagement::GetInstance();

        // If the logger is not valid or has not been initialized,
        // skip the rest of processing
        VerifyOrExit(logger.IsValid(), );

        for (int i = 0; i < CHIP_NUM_PRIORITY_LEVEL; i++)
        {
            chip::EventNumber tmp_id = logger.GetFirstEventNumber(static_cast<PriorityLevel>(i));
            if (tmp_id > initialEvents[i])
            {
                initialEvents[i] = tmp_id;
            }
        }

        // Check whether we are in a middle of an upload
        if (apReadHandler->mCurrentPriority == PriorityLevel::Invalid)
        {
            // Upload is not underway.  Check for new events, and set a checkpoint
            dataClean = apReadHandler->CheckEventUpToDate(logger);
            if (!dataClean)
            {
                // We have more events. snapshot last event IDs
                apReadHandler->SetEventLogEndpoint(logger);
            }

            // initialize the next priority level to transfer
            apReadHandler->mCurrentPriority = apReadHandler->FindNextPriorityForTransfer();
        }
        else
        {
            apReadHandler->mCurrentPriority = apReadHandler->FindNextPriorityForTransfer();
            dataClean                       = (apReadHandler->mCurrentPriority == PriorityLevel::Invalid);
        }

        // proceed only if there are new events.
        if (dataClean)
        {
            aReportDataBuilder.Rollback(backup);
            ExitNow(); // Read clean, move along
        }

        while (apReadHandler->mCurrentPriority != PriorityLevel::Invalid)
        {
            size_t i = static_cast<size_t>(apReadHandler->mCurrentPriority) - static_cast<size_t>(PriorityLevel::First);
            err      = logger.FetchEventsSince(*(eventList.GetWriter()), apReadHandler->mCurrentPriority,
                                          apReadHandler->mSelfVendedEvents[i]);

            if ((err == CHIP_END_OF_TLV) || (err == CHIP_ERROR_TLV_UNDERRUN) || (err == CHIP_NO_ERROR))
            {
                // We have successfully reached the end of the log for
                // the current priority. Advance to the next
                // priority level.
                err                             = CHIP_NO_ERROR;
                apReadHandler->mCurrentPriority = apReadHandler->FindNextPriorityForTransfer();
                mMoreChunkedMessages            = false;
            }
            else if ((err == CHIP_ERROR_BUFFER_TOO_SMALL) || (err == CHIP_ERROR_NO_MEMORY))
            {
                event_count = CountEvents(apReadHandler, initialEvents);

                // when first cluster event is too big to fit in the packet, ignore that cluster event.
                if (event_count == 0)
                {
                    apReadHandler->mSelfVendedEvents[i]++;
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
    }

    eventList.EndOfEventList();
    SuccessOrExit(eventList.GetError() == CHIP_NO_ERROR);

    event_count = CountEvents(apReadHandler, initialEvents);
    ChipLogDetail(DataManagement, "Fetched %d events", event_count);
exit:

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DataManagement, "Error retrieving events, err = %d", err);
    }

    return err;
}

CHIP_ERROR ReportingEngine::BuildAndSendSingleReportData(ReadHandler * apReadHandler)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter reportDataWriter;
    ReportData::Builder reportDataBuilder;
    chip::System::PacketBufferHandle bufHandle = System::PacketBufferHandle::New(chip::app::kMaxSecureSduLength);

    VerifyOrExit(!bufHandle.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    reportDataWriter.Init(std::move(bufHandle));

    // Create a report data.
    err = reportDataBuilder.Init(&reportDataWriter);
    SuccessOrExit(err);

    // Fill in the EventList.
    err = BuildSingleReportDataEventList(reportDataBuilder, apReadHandler);
    SuccessOrExit(err);

    // TODO: Add mechanism to set mSuppressResponse to handle status reports for multiple reports
    if (apReadHandler->mSuppressResponse)
    {
        ChipLogDetail(DataManagement, "<RE> suppress response..");
        reportDataBuilder.SuppressResponse(apReadHandler->mSuppressResponse);
    }

    if (mMoreChunkedMessages)
    {
        reportDataBuilder.MoreChunkedMessages(mMoreChunkedMessages);
    }

    reportDataBuilder.EndOfReportData();
    SuccessOrExit(reportDataBuilder.GetError());

    err = reportDataWriter.Finalize(&bufHandle);
    SuccessOrExit(err);

    VerifyOrExit(bufHandle->EnsureReservedSize(System::PacketBuffer::kDefaultHeaderReserve), err = CHIP_ERROR_BUFFER_TOO_SMALL);

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
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(DataManagement, "<RE> Error sending out report data!"));

exit:
    ChipLogFunctError(err);
    if (err != CHIP_NO_ERROR)
    {
        apReadHandler->Shutdown();
    }
    return err;
}

void ReportingEngine::Run(System::Layer * aSystemLayer, void * apAppState, System::Error)
{
    ReportingEngine * const pEngine = reinterpret_cast<ReportingEngine *>(apAppState);
    pEngine->Run();
}

void ReportingEngine::ScheduleRun()
{
    InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionMgr()->SystemLayer()->ScheduleWork(Run, this);
}

void ReportingEngine::Run()
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    uint32_t numReadHandled = 0;

    InteractionModelEngine * imEngine = InteractionModelEngine::GetInstance();
    ReadHandler * readHandler         = imEngine->mReadHandlers + mCurReadHandlerIdx;

    while ((mNumReportsInFlight < CHIP_MAX_REPORTS_IN_FLIGHT) && (numReadHandled < CHIP_MAX_NUM_READ_HANDLER))
    {
        if (readHandler->IsReportable())
        {
            err = BuildAndSendSingleReportData(readHandler);
            SuccessOrExit(err);

            ChipLogDetail(DataManagement, "<RE:Run> ReportsInFlight = %u with readHandler %u, RE has %s", mNumReportsInFlight,
                          mCurReadHandlerIdx, mMoreChunkedMessages ? "MoreChunkedMessages" : "no more messages");
            if (!mMoreChunkedMessages)
            {
                if (readHandler->mSuppressResponse)
                    OnReportConfirm();
                readHandler->OnReportProcessingComplete();
            }
        }
        numReadHandled++;
        mCurReadHandlerIdx = (mCurReadHandlerIdx + 1) % CHIP_MAX_NUM_READ_HANDLER;
        readHandler        = imEngine->mReadHandlers + mCurReadHandlerIdx;
    }

exit:
    ChipLogFunctError(err);
}

CHIP_ERROR ReportingEngine::SendReport(ReadHandler * apReadHandler, System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // We can only have 1 report in flight for any given read/subscription - increment and break out.
    mNumReportsInFlight++;

    err = apReadHandler->SendReportData(std::move(aPayload));

    if (err != CHIP_NO_ERROR)
    {
        mNumReportsInFlight--;
    }
    return err;
}

void ReportingEngine::OnReportConfirm()
{
    VerifyOrDie(mNumReportsInFlight > 0);

    ChipLogDetail(DataManagement, "<RE> OnReportConfirm: NumReports-- = %d", mNumReportsInFlight - 1);
    mNumReportsInFlight--;
    // TODO: Add more chunk message support when status report feature is ready
}

chip::EventNumber ReportingEngine::CountEvents(ReadHandler * apReadHandler, chip::EventNumber * apInitialEvents)
{
    chip::EventNumber event_count = 0;
    for (int t = 0; t < CHIP_NUM_PRIORITY_LEVEL; t++)
    {
        if (apReadHandler->mSelfVendedEvents[t] > apInitialEvents[t])
        {
            event_count += apReadHandler->mSelfVendedEvents[t] - apInitialEvents[t];
        }
    }
    return event_count;
}

}; // namespace reporting
}; // namespace app
}; // namespace chip
