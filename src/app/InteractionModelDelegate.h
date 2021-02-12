/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file defines the classes corresponding to CHIP Interaction Model Delegate.
 *
 */

#pragma once

#include <core/CHIPCore.h>
#include <messaging/ExchangeContext.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace app {

struct EventPathParams;
class ReadHandler;

/**
 * @brief
 *   This class provides a skeleton for the callback functions. The functions will be
 *   called by InteractionModel on specific events. If the user of InteractionModel
 *   is interested in receiving these callbacks, they can specialize this class and handle
 *   each event in their implementation of this class.
 */
class InteractionModelDelegate
{
public:
    virtual ~InteractionModelDelegate() {}

    enum class EventId
    {
        // InteractionModelEngine
        kIncomingInvokeCommandRequest,
        kIncomingReadRequest,
        kIncomingReportRequest,
        // ReadClient
        kReadRequestPrepareNeeded,
        kReportDataRequest,
        kReportProcessed,
        kEventStreamReceived,
        // ReadHandler
        kReadRequestParsed,
    };

    /**
     * Incoming parameters sent with events generated directly from this component
     *
     */
    union InEventParam
    {
        InEventParam() { Clear(); }

        void Clear(void) { memset(this, 0, sizeof(*this)); }

        struct
        {
            // ExchangeContext managed externally and should not be closed by event processing.
            const PacketHeader * packetHeader;
        } mIncomingInvokeCommandRequest;

        struct
        {
            Messaging::ExchangeContext * exchangeContext;
            const PacketHeader * packetHeader;
        } mIncomingReadRequest;

        struct
        {
            // ExchangeContext managed externally and should not be closed by event processing.
            Messaging::ExchangeContext * exchangeContext;
            const PacketHeader * packetHeader;
        } mReportDataRequest;

        struct
        {
            // ExchangeContext managed externally and should not be closed by event processing.
            Messaging::ExchangeContext * exchangeContext;
            chip::TLV::TLVReader * reader;
            const PacketHeader * packetHeader;
        } mEventStreamReceived;

        struct
        {
            bool mGetToAllEvents;
            // ExchangeContext managed externally and should not be closed by event processing.
            const Messaging::ExchangeContext * exchangeContext;
            const PacketHeader * packetHeader;
            ReadHandler * readHandler;
        } mReadRequestParsed;
    };

    /**
     * Outgoing parameters sent with events generated directly from this component
     *
     */
    union OutEventParam
    {
        OutEventParam() { Clear(); }
        void Clear(void) { memset(this, 0, sizeof(*this)); }

        struct
        {
            bool invokeCommandAllowed;
            uint32_t * reasonClusterId;  //< A pointer to the Cluster ID of reason for rejection
            uint16_t * reasonStatusCode; //< A pointer to the status code of reason for rejection
        } mIncomingInvokeCommandRequest;

        struct
        {
            bool readRequestAllowed;
            uint32_t * reasonClusterId;  //< A pointer to the Cluster ID of reason for rejection
            uint16_t * reasonStatusCode; //< A pointer to the status code of reason for rejection
        } mIncomingReadRequest;

        struct
        {
            EventPathParams * eventPathParamsList; //< Pointer to a list of event path parameter
            size_t eventPathParamsListSize;        //< Number of event paths in mpEventPathList
            uint64_t eventNumber;                  //< A event number it has already
        } mReadRequestPrepareNeeded;
    };

    /**
     * @brief Set the event calllback function
     * @param[in]  aEvent       A function pointer for event call back
     * @param[in]  aInParam     A const reference to the input parameter for this event
     * @param[out] aOutParam    A reference to the output parameter for this event
     */
    virtual void HandleEvent(EventId aEvent, const InEventParam & aInParam, OutEventParam & aOutParam) = 0;

    void DefaultEventHandler(EventId aEvent, const InEventParam & aInParam, OutEventParam & aOutParam)
    {
        ChipLogDetail(DataManagement, "%s event: %d", __func__, aEvent);
    }
};

} // namespace app
} // namespace chip
