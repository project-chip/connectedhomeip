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
#include <app/data-model-provider/ProviderChangeListener.h>
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

class InteractionModelEngine;
class TestReadInteraction;

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
class Engine : public DataModel::ProviderChangeListener
{
public:
    /**
     *  Constructor Engine with a valid InteractionModelEngine pointer.
     */
    Engine(InteractionModelEngine * apImEngine);

    /**
     * Initializes the reporting engine. Should only be called once.
     *
     * @retval #CHIP_NO_ERROR On success.
     * @retval other           Was unable to retrieve data and write it into the writer.
     */
    CHIP_ERROR Init();

    void Shutdown();

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    void SetWriterReserved(uint32_t aReservedSize) { mReservedSize = aReservedSize; }

    void SetMaxAttributesPerChunk(uint32_t aMaxAttributesPerChunk) { mMaxAttributesPerChunk = aMaxAttributesPerChunk; }
#endif

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
    CHIP_ERROR SetDirty(const AttributePathParams & aAttributePathParams);

    /**
     * @brief
     *  Schedule the event delivery
     *
     */
    CHIP_ERROR ScheduleEventDelivery(ConcreteEventPath & aPath, uint32_t aBytesWritten);

    /*
     * Resets the tracker that tracks the currently serviced read handler.
     * apReadHandler can be non-null to indicate that the reset is due to a
     * specific ReadHandler being deallocated.
     */
    void ResetReadHandlerTracker(ReadHandler * apReadHandlerBeingDeleted)
    {
        if (apReadHandlerBeingDeleted == mRunningReadHandler)
        {
            // Just decrement, so our increment after we finish running it will
            // do the right thing.
            --mCurReadHandlerIdx;
        }
        else
        {
            // No idea what to do here to make the indexing sane.  Just start at
            // the beginning.  We need to do better here; see
            // https://github.com/project-chip/connectedhomeip/issues/13809
            mCurReadHandlerIdx = 0;
        }
    }

    uint32_t GetNumReportsInFlight() const { return mNumReportsInFlight; }

    uint64_t GetDirtySetGeneration() const { return mDirtyGeneration; }

    /**
     * Schedule event delivery to happen immediately and run reporting to get
     * those reports into messages and on the wire.  This can be done either for
     * a specific fabric, identified by the provided FabricIndex, or across all
     * fabrics if no FabricIndex is provided.
     */
    void ScheduleUrgentEventDeliverySync(Optional<FabricIndex> fabricIndex = NullOptional);

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    size_t GetGlobalDirtySetSize() { return mGlobalDirtySet.Allocated(); }
#endif

    /* ProviderChangeListener implementation */
    void MarkDirty(const AttributePathParams & path) override;

private:
    /**
     * Main work-horse function that executes the run-loop.
     */
    void Run();

    friend class TestReportingEngine;
    friend class ::chip::app::TestReadInteraction;

    bool IsRunScheduled() const { return mRunScheduled; }

    struct AttributePathParamsWithGeneration : public AttributePathParams
    {
        AttributePathParamsWithGeneration() {}
        AttributePathParamsWithGeneration(const AttributePathParams aPath) : AttributePathParams(aPath) {}
        uint64_t mGeneration = 0;
    };

    /**
     * Build Single Report Data including attribute changes and event data stream, and send out
     *
     */
    CHIP_ERROR BuildAndSendSingleReportData(ReadHandler * apReadHandler);

    CHIP_ERROR BuildSingleReportDataAttributeReportIBs(ReportDataMessage::Builder & reportDataBuilder, ReadHandler * apReadHandler,
                                                       bool * apHasMoreChunks, bool * apHasEncodedData);
    CHIP_ERROR BuildSingleReportDataEventReports(ReportDataMessage::Builder & reportDataBuilder, ReadHandler * apReadHandler,
                                                 bool aBufferIsUsed, bool * apHasMoreChunks, bool * apHasEncodedData);
    CHIP_ERROR CheckAccessDeniedEventPaths(TLV::TLVWriter & aWriter, bool & aHasEncodedData, ReadHandler * apReadHandler);

    // If version match, it means don't send, if version mismatch, it means send.
    // If client sends the same path with multiple data versions, client will get the data back per the spec, because at least one
    // of those will fail to match.  This function should return false if either nothing in the list matches the given
    // endpoint+cluster in the path or there is an entry in the list that matches the endpoint+cluster in the path but does not
    // match the current data version of that cluster.
    bool IsClusterDataVersionMatch(const SingleLinkedListNode<DataVersionFilter> * aDataVersionFilterList,
                                   const ConcreteReadAttributePath & aPath);

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

    CHIP_ERROR ScheduleBufferPressureEventDelivery(uint32_t aBytesWritten);
    void GetMinEventLogPosition(uint32_t & aMinLogPosition);

    /**
     * If the provided path is a superset of our of our existing paths, update that existing path to match the
     * provided path.
     *
     * Return whether one of our paths is now a superset of the provided path.
     */
    bool MergeOverlappedAttributePath(const AttributePathParams & aAttributePath);

    /**
     * If we are running out of ObjectPool for the global dirty set, we will try to merge the existing items by clusters.
     *
     * Returns whether we have released any paths.
     */
    bool MergeDirtyPathsUnderSameCluster();

    /**
     * If we are running out of ObjectPool for the global dirty set and we cannot find a slot after merging the existing items by
     * clusters, we will try to merge the existing items by endpoints.
     *
     * Returns whether we have released any paths.
     */
    bool MergeDirtyPathsUnderSameEndpoint();

    /**
     * During the iterating of the paths, releasing the object in the inner loop will cause undefined behavior of the ObjectPool, so
     * we replace the items to be cleared by a tomb first, then clear all the tombs after the iteration.
     *
     * Returns whether we have released any paths.
     */
    bool ClearTombPaths();

    CHIP_ERROR InsertPathIntoDirtySet(const AttributePathParams & aAttributePath);

    inline void BumpDirtySetGeneration() { mDirtyGeneration++; }

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
     * The read handler we're calling BuildAndSendSingleReportData on right now.
     */
    ReadHandler * mRunningReadHandler = nullptr;

    /**
     *  mGlobalDirtySet is used to track the set of attribute/event paths marked dirty for reporting purposes.
     *
     */
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    // For unit tests, always use inline allocation for code coverage.
    ObjectPool<AttributePathParamsWithGeneration, CHIP_IM_SERVER_MAX_NUM_DIRTY_SET, ObjectPoolMem::kInline> mGlobalDirtySet;
#else
    ObjectPool<AttributePathParamsWithGeneration, CHIP_IM_SERVER_MAX_NUM_DIRTY_SET> mGlobalDirtySet;
#endif

    /**
     * A generation counter for the dirty attrbute set.
     * ReadHandlers can save the generation value when generating reports.
     *
     * Then we can tell whether they might have missed reporting an attribute by
     * comparing its generation counter to the saved one.
     *
     * mDirtySetGeneration will increase by one when SetDirty is called.
     *
     * Count it from 1, so 0 can be used in ReadHandler to indicate "the read handler has never
     * completed a report".
     */
    uint64_t mDirtyGeneration = 1;

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    uint32_t mReservedSize          = 0;
    uint32_t mMaxAttributesPerChunk = UINT32_MAX;
#endif

    InteractionModelEngine * mpImEngine = nullptr;
};

}; // namespace reporting
}; // namespace app
}; // namespace chip
