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

#include <protocols/secure_channel/CASESessionCache.h>

namespace chip {

CASESessionCache::CASESessionCache() {}

CASESessionCache::~CASESessionCache()
{
    mCachePool.ForEachActiveObject([&](auto * ec) {
        mCachePool.ReleaseObject(ec);
        return Loop::Continue;
    });
}

CASESessionCachable * CASESessionCache::GetLRUSession()
{
    uint64_t minTimeStamp            = UINT64_MAX;
    CASESessionCachable * lruSession = nullptr;
    mCachePool.ForEachActiveObject([&](auto * ec) {
        if (minTimeStamp > ec->mSessionSetupTimeStamp)
        {
            minTimeStamp = ec->mSessionSetupTimeStamp;
            lruSession   = ec;
        }
        return Loop::Continue;
    });
    return lruSession;
}

CHIP_ERROR CASESessionCache::Add(CASESessionCachable & cachableSession)
{
    // It's not an error if a device doesn't have cache for storing the sessions.
    VerifyOrReturnError(mCachePool.Capacity() > 0, CHIP_NO_ERROR);

    // If the cache is full, get the least recently used session index and release that.
    if (mCachePool.Allocated() >= kCacheSize)
    {
        mCachePool.ReleaseObject(GetLRUSession());
    }

    mCachePool.CreateObject(cachableSession);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CASESessionCache::Remove(ResumptionID resumptionID)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CASESession session;
    mCachePool.ForEachActiveObject([&](auto * ec) {
        if (resumptionID.data_equal(ResumptionID(ec->mResumptionId)))
        {
            mCachePool.ReleaseObject(ec);
        }
        return Loop::Continue;
    });

    return err;
}

CHIP_ERROR CASESessionCache::Get(ResumptionID resumptionID, CASESessionCachable & outSessionCachable)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    bool found     = false;
    mCachePool.ForEachActiveObject([&](auto * ec) {
        if (resumptionID.data_equal(ResumptionID(ec->mResumptionId)))
        {
            found              = true;
            outSessionCachable = *ec;
            return Loop::Break;
        }
        return Loop::Continue;
    });

    if (!found)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    return err;
}

CHIP_ERROR CASESessionCache::Get(const PeerId & peer, CASESessionCachable & outSessionCachable)
{
    // TODO: Implement this based on peer id
    return CHIP_NO_ERROR;
}

} // namespace chip
