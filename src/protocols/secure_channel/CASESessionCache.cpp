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
        return true;
    });
}

CASESessionSerializable * CASESessionCache::GetLRUSession()
{
    uint64_t minTimeStamp                = UINT64_MAX;
    CASESessionSerializable * lruSession = nullptr;
    mCachePool.ForEachActiveObject([&](auto * ec) {
        if (minTimeStamp > ec->mSessionSetupTimeStamp)
        {
            minTimeStamp = ec->mSessionSetupTimeStamp;
            lruSession   = ec;
        }
        return true;
    });
    return lruSession;
}

CHIP_ERROR CASESessionCache::Add(CASESession & session)
{
    // It's not an error if a device doesn't have cache for storing the sessions.
    VerifyOrReturnError(mCachePool.Size() > 0, CHIP_NO_ERROR);

    // If the cache is full, get the least recently used session index and release that.
    if (mCachePool.Exhausted())
    {
        mCachePool.ReleaseObject(GetLRUSession());
    }

    CASESessionSerializable serializable;
    ReturnLogErrorOnFailure(session.ToSerializable(serializable));
    mCachePool.CreateObject(serializable);
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
        return true;
    });

    return err;
}

CHIP_ERROR CASESessionCache::Get(ResumptionID resumptionID, CASESession & outSession)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    bool found     = false;
    mCachePool.ForEachActiveObject([&](auto * ec) {
        if (resumptionID.data_equal(ResumptionID(ec->mResumptionId)))
        {
            found = true;
            err   = outSession.FromSerializable(*ec);
            return false;
        }
        return true;
    });

    if (!found)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    return err;
}

CHIP_ERROR CASESessionCache::Get(const PeerId & peer, CASESession & outSession)
{
    // TODO: Implement this based on peer id
    return CHIP_NO_ERROR;
}

} // namespace chip
