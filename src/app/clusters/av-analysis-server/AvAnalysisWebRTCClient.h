/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#pragma once

#include <cstdint>

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/ScopedNodeId.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {

/**
 * WebRTC-facing client interactions performed by an AV Analysis server with the
 * RemoteContextDetection feature: initiating and ending WebRTC sessions that carry an analysis
 * video stream from the camera's WebRTCTransportProvider cluster.
 *
 * All methods are asynchronous: a CHIP_NO_ERROR return means the interaction was started and its
 * outcome will be reported through the per-request Callback. A failing return means the
 * interaction could not be started and no callback will occur. Callbacks are always delivered on
 * the Matter thread.
 */
class AvAnalysisWebRTCClient
{
public:
    class Callback
    {
    public:
        virtual ~Callback() = default;

        /**
         * The session with the camera has been initiated: the offer exchange completed and the
         * camera assigned aWebRTCSessionId. A non-Success status means initiation failed and no
         * session exists; it is the camera's response status where one was received.
         */
        virtual void OnSessionInitiated(Protocols::InteractionModel::Status aStatus, uint16_t aWebRTCSessionId) = 0;

        /**
         * The initiated session completed its signaling flow and media is streaming.
         */
        virtual void OnSessionActive(uint16_t aWebRTCSessionId) = 0;

        /**
         * The initiated session failed after initiation, or ended for a reason other than a
         * requested EndSession (e.g. the camera ended it).
         */
        virtual void OnSessionFailed(uint16_t aWebRTCSessionId) = 0;

        /**
         * Outcome of EndSession for aWebRTCSessionId. A non-Success status is the camera's
         * response status.
         */
        virtual void OnSessionEnded(Protocols::InteractionModel::Status aStatus, uint16_t aWebRTCSessionId) = 0;
    };

    virtual ~AvAnalysisWebRTCClient() = default;

    /**
     * Initiates a WebRTC session with StreamUsage Analysis for the given camera video stream, via
     * the WebRTCTransportProvider cluster on the given endpoint of the camera node.
     *
     * OnSessionInitiated reports the outcome of initiation; a successfully initiated session later
     * reports OnSessionActive or OnSessionFailed exactly once.
     *
     * @param aCameraNode      The camera, on the same fabric as this Analysis Node.
     * @param aWebRTCEndpoint  The camera endpoint hosting WebRTCTransportProvider, as provided by
     *                         ActivateAnalysisStream.
     * @param aVideoStreamId   The camera-assigned VideoStreamID the session shall carry.
     * @param aCallback        Receives the outcomes; must outlive the session.
     */
    virtual CHIP_ERROR RequestSession(const ScopedNodeId & aCameraNode, EndpointId aWebRTCEndpoint, uint16_t aVideoStreamId,
                                      Callback & aCallback) = 0;

    /**
     * Ends the given WebRTC session with the camera, with a reason of UserHangup.
     *
     * @param aCameraNode       The camera the session was initiated with.
     * @param aWebRTCEndpoint   The camera endpoint hosting WebRTCTransportProvider.
     * @param aWebRTCSessionId  The camera-assigned session id, as reported by OnSessionInitiated.
     * @param aCallback         Receives the outcome; must outlive the request.
     */
    virtual CHIP_ERROR EndSession(const ScopedNodeId & aCameraNode, EndpointId aWebRTCEndpoint, uint16_t aWebRTCSessionId,
                                  Callback & aCallback) = 0;

    /**
     * Abandons any in-flight request; its callbacks are never delivered. Safe to call from within
     * a completion callback: the request is already finished by then, making this a no-op.
     */
    virtual void Cancel() = 0;
};

} // namespace Clusters
} // namespace app
} // namespace chip
