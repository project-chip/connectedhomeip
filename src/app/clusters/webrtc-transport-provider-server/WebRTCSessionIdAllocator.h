#pragma once

#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

#include <bitset>
#include <cstddef>
#include <cstdint>

namespace chip::app::Clusters::WebRTCTransportProvider::detail {

/// Tracks the fixed WebRTC session ID domain without heap allocation.
/// The caller must serialize access with the CHIP stack lock.
class SessionIdAllocator
{
public:
    static constexpr uint16_t kMaxSessionId = 65534;
    static constexpr size_t kSessionIdCount = static_cast<size_t>(kMaxSessionId) + 1;

    CHIP_ERROR FindFree(uint16_t & outSessionId)
    {
        if (mAllocatedCount == kSessionIdCount)
        {
            return CHIP_IM_GLOBAL_STATUS(ResourceExhausted);
        }

        for (size_t attempts = 0; attempts < kSessionIdCount; ++attempts)
        {
            const uint16_t candidate = mNextSessionId;
            mNextSessionId           = (mNextSessionId == kMaxSessionId) ? 0 : static_cast<uint16_t>(mNextSessionId + 1);
            if (!mAllocatedIds[candidate])
            {
                outSessionId = candidate;
                return CHIP_NO_ERROR;
            }
        }

        return CHIP_ERROR_INTERNAL;
    }

    void MarkAllocated(uint16_t sessionId)
    {
        if (sessionId <= kMaxSessionId && !mAllocatedIds[sessionId])
        {
            mAllocatedIds.set(sessionId);
            ++mAllocatedCount;
        }
    }

    void MarkReleased(uint16_t sessionId)
    {
        if (sessionId <= kMaxSessionId && mAllocatedIds[sessionId])
        {
            mAllocatedIds.reset(sessionId);
            --mAllocatedCount;
        }
    }

private:
    std::bitset<kSessionIdCount> mAllocatedIds;
    size_t mAllocatedCount  = 0;
    uint16_t mNextSessionId = 0;
};

} // namespace chip::app::Clusters::WebRTCTransportProvider::detail
