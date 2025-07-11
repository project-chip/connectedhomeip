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

#include "pushav-prerollbuffer.h"

PushAvPreRollBuffer::PushAvPreRollBuffer(long long maxDurationMs, size_t maxContentBufferSize)
{
    if (maxDurationMs <= 0)
    {
        ChipLogError(Camera, "PushAV - PrerollBuffer initialized with invalid max duration : %lld ms", maxDurationMs);
        maxDurationMs = 1000;
    }

    mMaxDurationMs        = maxDurationMs;
    mMaxContentBufferSize = maxContentBufferSize;
    mCurrentSize          = 0;
    ChipLogProgress(Camera, "PushAV - PrerollBuffer initialized with max duration : %lld ms", mMaxDurationMs);
}

PushAvPreRollBuffer::~PushAvPreRollBuffer()
{
    ChipLogProgress(Camera, "PushAV - PrerollBuffer destroyed");
    std::lock_guard<std::mutex> lock(mPreRollBufferMutex);
    mBuffer.clear();
}

void PushAvPreRollBuffer::AddPacket(RawBufferPacket packet)
{
    std::lock_guard<std::mutex> lock(mPreRollBufferMutex);
    RemovePackets(packet.streamId); // remove as per mMaxDurationMs

    mCurrentSize += packet.size;
    mBuffer.push_back(packet);
    mBuffer.push_back(RawBufferPacket(packet));
}

RawBufferPacket PushAvPreRollBuffer::FetchPacket()
{
    if (GetSize() == 0)
    {
        return RawBufferPacket(nullptr, // data = null
                               0,       // size = 0
                               {},      // default time point
                               false,   // isVideo = false
                               "");
    }
    std::lock_guard<std::mutex> lock(mPreRollBufferMutex);
    RawBufferPacket front = mBuffer.front();
    mBuffer.pop_front();
    mCurrentSize -= front.size;
    return front;
}

int PushAvPreRollBuffer::GetSize()
{
    std::lock_guard<std::mutex> lock(mPreRollBufferMutex);
    return mBuffer.size();
}

void PushAvPreRollBuffer::RemovePackets(std::string streamId)
{

    if (mMaxDurationMs <= 0)
        return;
    auto now = std::chrono::steady_clock::now();
    if (mMaxDurationMs > 0)
    {
        while (!mBuffer.empty()) // buffer is monotonically increasing, so we can safely pop from front without locking.
        {
            auto & front = mBuffer.front();
            auto ageMs   = std::chrono::duration_cast<std::chrono::milliseconds>(now - front.InputTime).count();
            if (ageMs > mMaxDurationMs)
            {
                mBuffer.pop_front();
            }
            else
            {
                break; // all packets are younger than max duration, stop here.
            }
        }
    }

    if (mCurrentSize >= mMaxContentBufferSize)
    {
        for (auto it = mBuffer.begin(); it != mBuffer.end(); ++it)
        {
            mCurrentSize -= it->size;
            mBuffer.erase(it);
            if (mCurrentSize < mMaxContentBufferSize || mBuffer.empty())
            {
                break;
            }
        }
    }
}
