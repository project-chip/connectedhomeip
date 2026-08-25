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

#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <lib/core/ScopedNodeId.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AvAnalysis {

/**
 * A camera-bound command (EstablishAnalysisStream, RemoveAnalysisStream) cannot be answered until
 * the camera responds, so its CommandHandler is parked here and the interaction advances through
 * this state machine
 * Exactly one interaction exists at a time; a command arriving while InFlight() is answered Busy.
 */
class CameraInteraction
{
public:
    enum class State : uint8_t
    {
        kIdle,         // No camera interaction in flight
        kEstablishing, // VideoStreamAllocate awaiting the camera's answer
        kRemoving,     // VideoStreamDeallocate awaiting the camera's answer
    };

    State GetState() const { return mState; }
    bool InFlight() const { return mState != State::kIdle; }

    /**
     * The camera the in-flight interaction is bound to.
     */
    const ScopedNodeId & CameraNode() const { return mCameraNode; }

    /**
     * Enters aState, parking the command so it can be answered when the camera responds.
     */
    void Begin(State aState, CommandHandler & aHandler, const ConcreteCommandPath & aPath, const ScopedNodeId & aCameraNode)
    {
        mState      = aState;
        mHandle     = CommandHandler::Handle(&aHandler);
        mPath       = aPath;
        mCameraNode = aCameraNode;
        aHandler.FlushAcksRightAwayOnSlowCommand();
    }

    /**
     * Rolls back a Begin whose camera request could not be started
     */
    void Abort()
    {
        mState  = State::kIdle;
        mHandle = CommandHandler::Handle();
    }

    /**
     * Ends the interaction: returns the parked handle
     */
    CommandHandler::Handle Complete(ConcreteCommandPath & outPath)
    {
        outPath = mPath;
        mState  = State::kIdle;
        return std::move(mHandle);
    }

private:
    State mState = State::kIdle;
    CommandHandler::Handle mHandle;
    ConcreteCommandPath mPath = ConcreteCommandPath(kInvalidEndpointId, kInvalidClusterId, kInvalidCommandId);
    ScopedNodeId mCameraNode;
};

} // namespace AvAnalysis
} // namespace Clusters
} // namespace app
} // namespace chip
