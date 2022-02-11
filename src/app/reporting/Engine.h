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

#include <access/AccessControl.h>
#include <app/MessageDef/ReportDataMessage.h>
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

    void Shutdown();

#if CONFIG_IM_BUILD_FOR_UNIT_TEST
    void SetWriterReserved(uint32_t aReservedSize) { mReservedSize = aReservedSize; }
#endif

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

    /**
     * Application marks mutated change path and would be sent out in later report.
     */
    CHIP_ERROR SetDirty(ClusterInfo & aClusterInfo);

    /**
     * @brief
     *  Schedule the event delivery
     *
     */
    CHIP_ERROR ScheduleEventDelivery(ConcreteEventPath & aPath, EventOptions::Type aUrgent, uint32_t aBytesWritten);

    /*
     * Resets the tracker that tracks the currently serviced read handler.
     */
    void ResetReadHandlerTracker() { mCurReadHandlerIdx = 0; }

private:
    friend class TestReportingEngine;
    /**
     * Build Single Report Data including attribute changes and event data stream, and send out
     *
     */
    CHIP_ERROR BuildAndSendSingleReportData(ReadHandler * apReadHandler);

    CHIP_ERROR BuildSingleReportDataAttributeReportIBs(ReportDataMessage::Builder & reportDataBuilder, ReadHandler * apReadHandler,
                                                       bool * apHasMoreChunks, bool * apHasEncodedData);
    CHIP_ERROR BuildSingleReportDataEventReports(ReportDataMessage::Builder & reportDataBuilder, ReadHandler * apReadHandler,
                                                 bool * apHasMoreChunks, bool * apHasEncodedData);
    CHIP_ERROR RetrieveClusterData(const Access::SubjectDescriptor & aSubjectDescriptor, bool aIsFabricFiltered,
                                   AttributeReportIBs::Builder & aAttributeReportIBs,
                                   const ConcreteReadAttributePath & aClusterInfo,
                                   AttributeValueEncoder::AttributeEncodeState * apEncoderState);

    // If version match, it means don't send, if version mismatch, it means send.
    // If client sends the same path with multiple data versions, client will get the data back per the spec, because at least one
    // of those will fail to match.  This function should return false if either nothing in the list matches the given
    // endpoint+cluster in the path or there is an entry in the list that matches the endpoint+cluster in the path but does not
    // match the current data version of that cluster.
    bool IsClusterDataVersionMatch(ClusterInfo * aDataVersionFilterList, const ConcreteReadAttributePath & aPath);

    /**
     * Check all active subscription, if the subscription has no paths that intersect with global dirty set,
     * it would clear dirty flag for that subscription
     *
     */
    void UpdateReadHandlerDirty(ReadHandler & aReadHandler);
    /**
     * Send Report via ReadHandler
     *
     */
    CHIP_ERROR SendReport(ReadHandler * apReadHandler, System::PacketBufferHandle && aPayload, bool aHasMoreChunks);

    /**
     * Generate and send the report data request when there exists subscription or read request
     *
     */
    static void Run(System::Layer * aSystemLayer, void * apAppState);

    CHIP_ERROR ScheduleUrgentEventDelivery(ConcreteEventPath & aPath);
    CHIP_ERROR ScheduleBufferPressureEventDelivery(uint32_t aBytesWritten);
    void GetMinEventLogPosition(uint32_t & aMinLogPosition);

    /**
     * If the provided path is a superset of our of our existing paths, update that existing path to match the
     * provided path.
     *
     * Return whether one of our paths is now a superset of the provided path.
     */
    bool MergeOverlappedAttributePath(ClusterInfo & aAttributePath);

    /**
     * Boolean to indicate if ScheduleRun is pending. This flag is used to prevent calling ScheduleRun multiple times
     * within the same execution context to avoid applying too much pressure on platforms that use small, fixed size event queues.
     *
     */
    bool mRunScheduled = false;

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

    /**
     *  mGlobalDirtySet is used to track the set of attribute/event paths marked dirty for reporting purposes.
     *
     */
    ObjectPool<ClusterInfo, CHIP_IM_SERVER_MAX_NUM_DIRTY_SET> mGlobalDirtySet;

#if CONFIG_IM_BUILD_FOR_UNIT_TEST
    uint32_t mReservedSize = 0;
#endif
};

}; // namespace reporting
}; // namespace app
}; // namespace chip
