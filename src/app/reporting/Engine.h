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

#include <app/MessageDef/ReportData.h>
#include <app/ReadHandler.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/Protocols.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>

namespace chip {
namespace app {
namespace reporting {
/*
 *  @class Engine
 *
 *  @brief The reporting engine is responsible for generating reports to reader. It is able to find the intersection
 * between the path interest set of each reader with what has changed in the publisher data store and generate tailored
 * reports for each reader.
 *
 *         At its core, it  tries to gather and pack as much relevant attributes changes and/or events as possible into a report
 * message before sending that to the reader. It continues to do so until it has no more work to do.
 */
class Engine
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
     * Should be invoked when the device receives a Status report, or when the Report data request times out.
     * This allows the engine to do some clean-up.
     *
     */
    void OnReportConfirm();

    /**
     * Main work-horse function that executes the run-loop asynchronously on the CHIP thread
     */
    CHIP_ERROR ScheduleRun();

private:
    friend class TestReportingEngine;
    /**
     * Build Single Report Data including attribute changes and event data stream, and send out
     *
     */
    CHIP_ERROR BuildAndSendSingleReportData(ReadHandler * apReadHandler);

    CHIP_ERROR BuildSingleReportDataAttributeDataList(ReportData::Builder & reportDataBuilder, ReadHandler * apReadHandler);
    CHIP_ERROR BuildSingleReportDataEventList(ReportData::Builder & reportDataBuilder, ReadHandler * apReadHandler);
    CHIP_ERROR RetrieveClusterData(AttributeDataList::Builder & aAttributeDataList, ClusterInfo & aClusterInfo);
    EventNumber CountEvents(ReadHandler * apReadHandler, EventNumber * apInitialEvents);

    /**
     * Send Report via ReadHandler
     *
     */
    CHIP_ERROR SendReport(ReadHandler * apReadHandler, System::PacketBufferHandle && aPayload);

    /**
     * Generate and send the report data request when there exists subscription or read request
     *
     */
    static void Run(System::Layer * aSystemLayer, void * apAppState);

    /**
     * Boolean to show if more chunk message on the way
     *
     */
    bool mMoreChunkedMessages = false;

    /**
     * The number of report date request in flight
     *
     */
    uint32_t mNumReportsInFlight = 0;

    /**
     *  Current read handler index
     *
     */
    uint32_t mCurReadHandlerIdx = 0;
};

}; // namespace reporting
}; // namespace app
}; // namespace chip
