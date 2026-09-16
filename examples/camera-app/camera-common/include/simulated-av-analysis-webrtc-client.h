/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/av-analysis-server/AvAnalysisWebRTCClient.h>

/**
 * Stands in for an Analysis Node's WebRTC client when the camera itself runs the AV Analysis
 * cluster with RemoteContextDetection (--camera-remote-analysis, a CI configuration for the
 * cluster's test scripts). A camera hosts the provider side of WebRTC, not the client side, so no
 * signaling or media takes place: a requested session is initiated and active at once, and ends
 * when asked. The interface allows completions to arrive before the call returns.
 */
class SimulatedAvAnalysisWebRTCClient : public chip::app::Clusters::AvAnalysisWebRTCClient
{
public:
    CHIP_ERROR RequestSession(const chip::ScopedNodeId & aCameraNode, chip::EndpointId aWebRTCEndpoint, uint16_t aVideoStreamId,
                              Callback & aCallback) override
    {
        const uint16_t sessionId = mNextSessionId++;
        aCallback.OnSessionInitiated(chip::Protocols::InteractionModel::Status::Success, sessionId, /* aOfferSent = */ true);
        aCallback.OnSessionActive(aCameraNode, sessionId);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR EndSession(const chip::ScopedNodeId & aCameraNode, chip::EndpointId aWebRTCEndpoint, uint16_t aWebRTCSessionId,
                          Callback & aCallback) override
    {
        aCallback.OnSessionEnded(chip::Protocols::InteractionModel::Status::Success, aWebRTCSessionId);
        return CHIP_NO_ERROR;
    }

    // Nothing is in flight or tracked between calls
    void Cancel() override {}

private:
    uint16_t mNextSessionId = 0;
};
