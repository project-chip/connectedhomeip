/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/AttributePathParams.h>
#include <app/DataVersionFilter.h>
#include <app/EventPathParams.h>
#include <app/InteractionModelTimeout.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <messaging/ExchangeMgr.h>

namespace chip {
namespace app {

struct ReadPrepareParams
{
    SessionHolder mSessionHolder;
    EventPathParams * mpEventPathParamsList         = nullptr;
    size_t mEventPathParamsListSize                 = 0;
    AttributePathParams * mpAttributePathParamsList = nullptr;
    size_t mAttributePathParamsListSize             = 0;
    DataVersionFilter * mpDataVersionFilterList     = nullptr;
    size_t mDataVersionFilterListSize               = 0;
    Optional<EventNumber> mEventNumber;
    // The timeout for waiting for the response or System::Clock::kZero to let the interaction model decide the timeout based on the
    // MRP timeouts of the session.
    System::Clock::Timeout mTimeout     = System::Clock::kZero;
    uint16_t mMinIntervalFloorSeconds   = 0;
    uint16_t mMaxIntervalCeilingSeconds = 0;
    bool mKeepSubscriptions             = false;
    bool mIsFabricFiltered              = true;
    bool mIsPeerLIT                     = false;

    ReadPrepareParams() {}
    ReadPrepareParams(const SessionHandle & sessionHandle) { mSessionHolder.Grab(sessionHandle); }
    ReadPrepareParams(ReadPrepareParams && other) : mSessionHolder(other.mSessionHolder)
    {
        mKeepSubscriptions                 = other.mKeepSubscriptions;
        mpEventPathParamsList              = other.mpEventPathParamsList;
        mEventPathParamsListSize           = other.mEventPathParamsListSize;
        mpAttributePathParamsList          = other.mpAttributePathParamsList;
        mAttributePathParamsListSize       = other.mAttributePathParamsListSize;
        mpDataVersionFilterList            = other.mpDataVersionFilterList;
        mDataVersionFilterListSize         = other.mDataVersionFilterListSize;
        mEventNumber                       = other.mEventNumber;
        mMinIntervalFloorSeconds           = other.mMinIntervalFloorSeconds;
        mMaxIntervalCeilingSeconds         = other.mMaxIntervalCeilingSeconds;
        mTimeout                           = other.mTimeout;
        mIsFabricFiltered                  = other.mIsFabricFiltered;
        mIsPeerLIT                         = other.mIsPeerLIT;
        other.mpEventPathParamsList        = nullptr;
        other.mEventPathParamsListSize     = 0;
        other.mpAttributePathParamsList    = nullptr;
        other.mAttributePathParamsListSize = 0;
    }

    ReadPrepareParams & operator=(ReadPrepareParams && other)
    {
        if (&other == this)
            return *this;

        mKeepSubscriptions                 = other.mKeepSubscriptions;
        mSessionHolder                     = other.mSessionHolder;
        mpEventPathParamsList              = other.mpEventPathParamsList;
        mEventPathParamsListSize           = other.mEventPathParamsListSize;
        mpAttributePathParamsList          = other.mpAttributePathParamsList;
        mAttributePathParamsListSize       = other.mAttributePathParamsListSize;
        mpDataVersionFilterList            = other.mpDataVersionFilterList;
        mDataVersionFilterListSize         = other.mDataVersionFilterListSize;
        mEventNumber                       = other.mEventNumber;
        mMinIntervalFloorSeconds           = other.mMinIntervalFloorSeconds;
        mMaxIntervalCeilingSeconds         = other.mMaxIntervalCeilingSeconds;
        mTimeout                           = other.mTimeout;
        mIsFabricFiltered                  = other.mIsFabricFiltered;
        mIsPeerLIT                         = other.mIsPeerLIT;
        other.mpEventPathParamsList        = nullptr;
        other.mEventPathParamsListSize     = 0;
        other.mpAttributePathParamsList    = nullptr;
        other.mAttributePathParamsListSize = 0;
        return *this;
    }
};
} // namespace app
} // namespace chip
