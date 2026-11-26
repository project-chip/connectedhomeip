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
#include <lib/core/ScopedNodeId.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {

/**
 * Camera-facing client interactions performed by an AV Analysis server with the RemoteContextDetection
 * feature: allocating and deallocating analysis video streams on the camera's CameraAVStreamManagement cluster.
 *
 * The interface exists so unit tests can substitute the callbacks directly.
 *
 * All methods are asynchronous: a CHIP_NO_ERROR return means the interaction was started and its outcome
 * will be reported exactly once through the per-request Callback. A failing return means the interaction
 * could not be started and no callback will occur. Callbacks are always delivered on the Matter thread.
 */
class AvAnalysisCameraClient
{
public:
    class Callback
    {
    public:
        virtual ~Callback() = default;

        /**
         * Outcome of RequestVideoStreamAllocation. On Status::Success, aVideoStreamId is the camera-assigned
         * VideoStreamID of the allocated stream; Any other status is the camera's response status, to be propagated as the
         * EstablishAnalysisStream command status.
         */
        virtual void OnVideoStreamAllocated(Protocols::InteractionModel::Status aStatus, uint16_t aVideoStreamId) = 0;

        /**
         * Outcome of RequestVideoStreamDeallocation for the camera stream aVideoStreamId. A non-Success
         * status is the camera's response status, to be propagated as the RemoveAnalysisStream command status.
         */
        virtual void OnVideoStreamDeallocated(Protocols::InteractionModel::Status aStatus, uint16_t aVideoStreamId) = 0;
    };

    virtual ~AvAnalysisCameraClient() = default;

    /**
     * Sends VideoStreamAllocate with StreamUsage Analysis to the CameraAVStreamManagement cluster on the
     * given camera node; remaining allocate fields are implementation defined.
     *
     * @param aCameraNode The camera, on the same fabric as this Analysis Node, as identified by the
     *                    EstablishAnalysisStream NodeID field and the invoking client's fabric.
     * @param aCallback   Receives the outcome; must outlive the request.
     */
    virtual CHIP_ERROR RequestVideoStreamAllocation(const ScopedNodeId & aCameraNode, Callback & aCallback) = 0;

    /**
     * Sends VideoStreamDeallocate for the given camera stream to the given camera node.
     *
     * @param aCameraNode    The camera the stream was allocated on.
     * @param aVideoStreamId The camera-assigned VideoStreamID to deallocate, as reported by
     *                       OnVideoStreamAllocated.
     * @param aCallback      Receives the outcome; must outlive the request.
     */
    virtual CHIP_ERROR RequestVideoStreamDeallocation(const ScopedNodeId & aCameraNode, uint16_t aVideoStreamId,
                                                      Callback & aCallback) = 0;

    /**
     * Abandons any in-flight request; its callback is never delivered. Safe to call from within a
     * completion callback: the request is already finished by then, making this a no-op.
     */
    virtual void Cancel() = 0;
};

} // namespace Clusters
} // namespace app
} // namespace chip
