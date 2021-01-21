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
 *      This file defines Reporting Engine for a CHIP Interaction Model
 *
 */

#pragma once

#ifndef _CHIP_DATA_MODEL_REPORTING_ENGINE_H
#define _CHIP_DATA_MODEL_REPORTING_ENGINE_H

#include <app/MessageDef/ReportData.h>
#include <app/ReadHandler.h>
#include <app/reporting/EventLoggingTypes.h>
#include <app/reporting/LoggingManagement.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLVDebug.hpp>
#include <map>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <util/basic-types.h>

#define CHOP_MAX_REPORTS_IN_FLIGHT 1

namespace chip {
namespace app {
namespace reporting {
class ReportingEngine
{
public:
    /**
     * Initializes the reporting engine. Should only be called once.
     *
     * @retval #CHIP_NO_ERROR On success.
     * @retval other           Was unable to retrieve data and write it into the writer.
     */
    CHIP_ERROR Init();

    /**
     * Main work-horse function that executes the run-loop.
     */
    void Run();

    /**
     * Main work-horse function that executes the run-loop asynchronously on the CHIP thread
     */
    void ScheduleRun();

private:
    CHIP_ERROR BuildSingleReportDataEventList(ReportData::Builder & aReportDataBuilder, ReadHandler * aReadHandler);

    CHIP_ERROR BuildAndSendSingleReportData(ReadHandler * aReadHandler);

    CHIP_ERROR SendReport(ReadHandler * apReadHandler, System::PacketBufferHandle && aPayload);

    void OnReportConfirm();

    static void Run(System::Layer * aSystemLayer, void * apAppState, System::Error);

    bool mMoreChunkedMessages;
    uint32_t mNumReportsInFlight;
    EventList::Builder mEventList;
    uint32_t mCurReadHandlerIdx;
};

}; // namespace reporting
}; // namespace app
}; // namespace chip

#endif // _CHIP_DATA_MODEL_REPORTING_ENGINE_H
