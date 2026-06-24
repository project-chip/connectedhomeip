/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#include <PosixBleRssiRangingAdapter.h>

using namespace chip::app::Clusters::ProximityRanging;

PosixBleRssiRangingAdapter::~PosixBleRssiRangingAdapter() = default;

CHIP_ERROR PosixBleRssiRangingAdapter::GetActiveSessionIds(std::vector<uint8_t> & sessionIds)
{
    for (uint8_t id : mActiveSessions)
    {
        sessionIds.push_back(id);
    }
    return CHIP_NO_ERROR;
}

void PosixBleRssiRangingAdapter::StopAllSessions()
{
    while (!mActiveSessions.empty())
    {
        uint8_t id = mActiveSessions.back();
        mActiveSessions.pop_back();
        if (mCallback != nullptr)
        {
            // kHardwareError is used as a placeholder for all stop reasons until the
            // specification defines a distinct status for deliberate stops.
            mCallback->OnRangingSessionStopped(id, RangingSessionStatusEnum::kHardwareError);
        }
    }
}

void PosixBleRssiRangingAdapter::AddSession(uint8_t sessionId)
{
    mActiveSessions.push_back(sessionId);
}

bool PosixBleRssiRangingAdapter::RemoveSession(uint8_t sessionId)
{
    for (auto it = mActiveSessions.begin(); it != mActiveSessions.end(); ++it)
    {
        if (*it == sessionId)
        {
            mActiveSessions.erase(it);
            if (mCallback != nullptr)
            {
                // kHardwareError is used as a placeholder for all stop reasons until the
                // specification defines a distinct status for deliberate stops.
                mCallback->OnRangingSessionStopped(sessionId, RangingSessionStatusEnum::kHardwareError);
            }
            return true;
        }
    }
    return false;
}
