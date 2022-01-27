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

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/core/PeerId.h>
#include <protocols/secure_channel/CASESession.h>

namespace chip {

using ResumptionID = FixedByteSpan<kCASEResumptionIDSize>;

class CASESessionCache
{
public:
    CASESessionCache();
    virtual ~CASESessionCache();

    CHIP_ERROR Add(CASESessionCachable & cachableSession);
    CHIP_ERROR Remove(ResumptionID resumptionID);
    CHIP_ERROR Get(ResumptionID resumptionID, CASESessionCachable & outCachableSession);
    CHIP_ERROR Get(const PeerId & peer, CASESessionCachable & outCachableSession);

private:
    static constexpr size_t kCacheSize = CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE;
    ObjectPool<CASESessionCachable, kCacheSize> mCachePool;
    CASESessionCachable * GetLRUSession();
};

} // namespace chip
