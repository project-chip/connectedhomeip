/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "ButtonEventsSimulator.h"

#include <functional>
#include <inttypes.h>
#include <stdint.h>
#include <utility>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/EventLogging.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemClock.h>
#include <system/SystemLayer.h>

#include <app/clusters/switch-server/switch-server.h>

namespace chip {
namespace app {

namespace {

void SetButtonPosition(EndpointId endpointId, uint8_t position)
{
    auto switchCluster = Clusters::Switch::FindClusterOnEndpoint(endpointId);
    VerifyOrReturn(switchCluster != nullptr);

    LogErrorOnFailure(switchCluster->SetCurrentPosition(position));
}

void EmitInitialPress(EndpointId endpointId, uint8_t newPosition)
{
    auto switchCluster = Clusters::Switch::FindClusterOnEndpoint(endpointId);
    VerifyOrReturn(switchCluster != nullptr);

    auto event = switchCluster->OnInitialPress(newPosition);
    VerifyOrReturn(event.has_value());

    ChipLogProgress(NotSpecified, "Logged InitialPress(%u) on Endpoint %u", static_cast<unsigned>(newPosition),
                    static_cast<unsigned>(endpointId));
}

void EmitLongPress(EndpointId endpointId, uint8_t newPosition)
{
    auto switchCluster = Clusters::Switch::FindClusterOnEndpoint(endpointId);
    VerifyOrReturn(switchCluster != nullptr);

    auto event = switchCluster->OnLongPress(newPosition);
    VerifyOrReturn(event.has_value());

    ChipLogProgress(NotSpecified, "Logged LongPress(%u) on Endpoint %u", static_cast<unsigned>(newPosition),
                    static_cast<unsigned>(endpointId));
}

void EmitLongRelease(EndpointId endpointId, uint8_t previousPosition)
{
    auto switchCluster = Clusters::Switch::FindClusterOnEndpoint(endpointId);
    VerifyOrReturn(switchCluster != nullptr);

    auto event = switchCluster->OnLongRelease(previousPosition);
    VerifyOrReturn(event.has_value());

    ChipLogProgress(NotSpecified, "Logged LongRelease on Endpoint %u", static_cast<unsigned>(endpointId));
}

void EmitMultiPressComplete(EndpointId endpointId, uint8_t previousPosition, uint8_t count)
{
    auto switchCluster = Clusters::Switch::FindClusterOnEndpoint(endpointId);
    VerifyOrReturn(switchCluster != nullptr);

    auto event = switchCluster->OnMultiPressComplete(previousPosition, count);
    VerifyOrReturn(event.has_value());

    ChipLogProgress(NotSpecified, "Logged MultiPressComplete(count=%u) on Endpoint %u", static_cast<unsigned>(count),
                    static_cast<unsigned>(endpointId));
}

void EmitShortRelease(EndpointId endpointId, uint8_t previousPosition)
{
    auto switchCluster = Clusters::Switch::FindClusterOnEndpoint(endpointId);
    VerifyOrReturn(switchCluster != nullptr);

    auto event = switchCluster->OnShortRelease(previousPosition);
    VerifyOrReturn(event.has_value());

    ChipLogProgress(NotSpecified, "Logged ShortRelease on Endpoint %u", static_cast<unsigned>(endpointId));
}

void EmitMultiPressOngoing(EndpointId endpointId, uint8_t newPosition, uint8_t count)
{
    auto switchCluster = Clusters::Switch::FindClusterOnEndpoint(endpointId);
    VerifyOrReturn(switchCluster != nullptr);

    auto event = switchCluster->OnMultiPressOngoing(newPosition, count);
    VerifyOrReturn(event.has_value());

    ChipLogProgress(NotSpecified, "Logged MultiPressOngoing on Endpoint %u position %u, count %u",
                    static_cast<unsigned>(endpointId), static_cast<unsigned>(newPosition), static_cast<unsigned>(count));
}

} // namespace

void ButtonEventsSimulator::OnTimerDone(System::Layer * layer, void * appState)
{
    ButtonEventsSimulator * that = reinterpret_cast<ButtonEventsSimulator *>(appState);
    that->Next();
}

bool ButtonEventsSimulator::Execute(DoneCallback && doneCallback)
{
    VerifyOrReturnValue(mIdleButtonId != mPressedButtonId, false);

    switch (mMode)
    {
    case Mode::kModeLongPress:
        VerifyOrReturnValue(mLongPressDurationMillis > mLongPressDelayMillis, false);
        SetState(State::kEmitStartOfLongPress);
        break;
    case Mode::kModeMultiPress:
        VerifyOrReturnValue(mMultiPressPressedTimeMillis.count() > 0, false);
        VerifyOrReturnValue(mMultiPressReleasedTimeMillis.count() > 0, false);
        VerifyOrReturnValue(mMultiPressNumPresses > 0, false);
        SetState(State::kEmitStartOfMultiPress);
        break;
    default:
        return false;
    }
    mDoneCallback = std::move(doneCallback);
    Next();
    return true;
}

void ButtonEventsSimulator::SetState(ButtonEventsSimulator::State newState)
{
    ButtonEventsSimulator::State oldState = mState;
    if (oldState != newState)
    {
        ChipLogProgress(NotSpecified, "ButtonEventsSimulator state change %u -> %u", static_cast<unsigned>(oldState),
                        static_cast<unsigned>(newState));
    }

    mState = newState;
}

void ButtonEventsSimulator::StartTimer(System::Clock::Timeout duration)
{
    TEMPORARY_RETURN_IGNORED chip::DeviceLayer::SystemLayer().StartTimer(duration, &ButtonEventsSimulator::OnTimerDone, this);
}

void ButtonEventsSimulator::Next()
{
    switch (mState)
    {
    case ButtonEventsSimulator::State::kIdle: {
        ChipLogError(NotSpecified, "Found idle state where not expected!");
        break;
    }
    case ButtonEventsSimulator::State::kEmitStartOfLongPress: {
        SetButtonPosition(mEndpointId, mPressedButtonId);
        EmitInitialPress(mEndpointId, mPressedButtonId);
        SetState(ButtonEventsSimulator::State::kEmitLongPress);
        StartTimer(mLongPressDelayMillis);
        break;
    }
    case ButtonEventsSimulator::State::kEmitLongPress: {
        EmitLongPress(mEndpointId, mPressedButtonId);
        SetState(ButtonEventsSimulator::State::kEmitLongRelease);
        StartTimer(mLongPressDurationMillis - mLongPressDelayMillis);
        break;
    }
    case ButtonEventsSimulator::State::kEmitLongRelease: {
        SetButtonPosition(mEndpointId, mIdleButtonId);
        if (mFeatureMap & static_cast<uint32_t>(Clusters::Switch::Feature::kMomentarySwitchLongPress))
        {
            EmitLongRelease(mEndpointId, mPressedButtonId);
        }
        else if (mFeatureMap & static_cast<uint32_t>(Clusters::Switch::Feature::kMomentarySwitchRelease))
        {
            EmitShortRelease(mEndpointId, mPressedButtonId);
        }
        SetState(ButtonEventsSimulator::State::kIdle);
        mDoneCallback();
        break;
    }
    case ButtonEventsSimulator::State::kEmitStartOfMultiPress: {
        SetButtonPosition(mEndpointId, mPressedButtonId);
        EmitInitialPress(mEndpointId, mPressedButtonId);
        if (mFeatureMap & static_cast<uint32_t>(Clusters::Switch::Feature::kActionSwitch))
        {
            StartTimer(mMultiPressNumPresses * (mMultiPressPressedTimeMillis + mMultiPressReleasedTimeMillis));
            SetState(ButtonEventsSimulator::State::kEmitEndOfMultiPress);
        }
        else
        {
            SetState(ButtonEventsSimulator::State::kMultiPressButtonRelease);
            StartTimer(mMultiPressPressedTimeMillis);
        }
        break;
    }
    case ButtonEventsSimulator::State::kMultiPressButtonRelease: {
        ++mMultiPressPressesDone;
        if (mMultiPressPressesDone > 1)
        {
            EmitMultiPressOngoing(mEndpointId, mPressedButtonId, mMultiPressPressesDone);
        }

        if (mMultiPressPressesDone == mMultiPressNumPresses)
        {
            SetState(ButtonEventsSimulator::State::kEmitEndOfMultiPress);
        }
        else
        {
            SetState(ButtonEventsSimulator::State::kEmitStartOfMultiPress);
        }

        if (mFeatureMap & static_cast<uint32_t>(Clusters::Switch::Feature::kMomentarySwitchRelease))
        {
            EmitShortRelease(mEndpointId, mPressedButtonId);
        }
        SetButtonPosition(mEndpointId, mIdleButtonId);
        StartTimer(mMultiPressReleasedTimeMillis);
        break;
    }

    case ButtonEventsSimulator::State::kEmitEndOfMultiPress: {
        if (mFeatureMap & static_cast<uint32_t>(Clusters::Switch::Feature::kActionSwitch) && mMultiPressNumPresses > mMultiPressMax)
        {
            EmitMultiPressComplete(mEndpointId, mPressedButtonId, 0);
        }
        else
        {
            EmitMultiPressComplete(mEndpointId, mPressedButtonId, mMultiPressNumPresses);
        }
        SetState(ButtonEventsSimulator::State::kIdle);
        mDoneCallback();
        break;
    }
    }
}

} // namespace app
} // namespace chip
