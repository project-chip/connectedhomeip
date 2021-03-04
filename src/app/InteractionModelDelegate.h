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
 *   called by InteractionModel on specific CallbackId. If the user of InteractionModel
 *   is interested in receiving these callbacks, they can specialize this class and handle
 *   each CallbackId in their implementation of this class.
 */
class InteractionModelDelegate
{
public:
    virtual ~InteractionModelDelegate() {}

    // Interaction Model would use the below callbackID as discriminator and notify the zcl/app the particular interaction
    // has completed or is happening or need more things from upper layer via InParam and OutParam
    // The below callbacks may or may not be necessary for upper zcl, once the zcl integration is complete, we will revisit these
    enum class CallbackId
    {
        // Read Client
        /**
         * Sent when it is ready to send the ReadRequest.  The
         * application is expected to fill in the attribute/event paths that it wants to
         * read/subscribe to.
         */
        kReadRequestPrepareNeeded,
        // IM notify zcl  events can be further processed in app.
        kEventStreamReceived,

        // IM notify zcl all reports have been processed.
        kReportProcessed,
    };

    /**
     * Incoming parameters sent with callback generated directly from this component
     *
     */
    union InParam
    {
        InParam() { Clear(); }

        void Clear(void) { memset(this, 0, sizeof(*this)); }

        struct
        {
            // ExchangeContext managed externally and should not be closed by zcl processing.
            Messaging::ExchangeContext * exchangeContext;
            // EventList tlv reader, upper layer can process event list.
            chip::TLV::TLVReader * reader;
        } mEventStreamReceived;
    };

    /**
     * Outgoing parameters sent with CallbackId generated directly from this component
     *
     */
    union OutParam
    {
        OutParam() { Clear(); }
        void Clear(void) { memset(this, 0, sizeof(*this)); }

        struct
        {
            EventPathParams * eventPathParamsList; //< Pointer to a list of event path parameter ZCL are intereted in
            size_t eventPathParamsListSize;        //< Number of event paths in mpEventPathList
            uint64_t eventNumber; //< A event number it has already to limit the set of retrieved events on the server for
                                  // optimization purposes
        } mReadRequestPrepareNeeded;
    };

    /**
     * @brief Set the callback function
     * @param[in]  aCallbackId       A function pointer for particular call back
     * @param[in]  aInParam     A const reference to the input parameter for this CallbackId
     * @param[out] aOutParam    A reference to the output parameter for this CallbackId
     */
    virtual void HandleIMCallBack(CallbackId aCallbackId, const InParam & aInParam, OutParam & aOutParam) = 0;

    void DefaultCallbackIdHandler(CallbackId aCallbackId, const InParam & aInParam, OutParam & aOutParam)
    {
        ChipLogDetail(DataManagement, "%s CallbackId: %d", __func__, aCallbackId);
    }
};

} // namespace app
} // namespace chip
