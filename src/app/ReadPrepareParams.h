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
#include <app/EventPathParams.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <messaging/ExchangeMgr.h>

namespace chip {
namespace app {
struct ReadPrepareParams
{
    SessionHandle mSessionHandle;
    EventPathParams * mpEventPathParamsList         = nullptr;
    size_t mEventPathParamsListSize                 = 0;
    AttributePathParams * mpAttributePathParamsList = nullptr;
    size_t mAttributePathParamsListSize             = 0;
    EventNumber mEventNumber                        = 0;
    uint32_t mTimeout                               = kImMessageTimeoutMsec;
    uint16_t mMinIntervalSeconds                    = 0;
    uint16_t mMaxIntervalSeconds                    = 0;

    ReadPrepareParams(SessionHandle sessionHandle) : mSessionHandle(sessionHandle) {}
    ReadPrepareParams(ReadPrepareParams && other) : mSessionHandle(other.mSessionHandle)
    {
        mpEventPathParamsList              = other.mpEventPathParamsList;
        mEventPathParamsListSize           = other.mEventPathParamsListSize;
        mpAttributePathParamsList          = other.mpAttributePathParamsList;
        mAttributePathParamsListSize       = other.mAttributePathParamsListSize;
        mEventNumber                       = other.mEventNumber;
        mMinIntervalSeconds                = other.mMinIntervalSeconds;
        mMaxIntervalSeconds                = other.mMaxIntervalSeconds;
        mTimeout                           = other.mTimeout;
        other.mpEventPathParamsList        = nullptr;
        other.mEventPathParamsListSize     = 0;
        other.mpAttributePathParamsList    = nullptr;
        other.mAttributePathParamsListSize = 0;
    }

    ReadPrepareParams & operator=(ReadPrepareParams && other)
    {
        if (&other == this)
            return *this;

        mSessionHandle                     = other.mSessionHandle;
        mpEventPathParamsList              = other.mpEventPathParamsList;
        mEventPathParamsListSize           = other.mEventPathParamsListSize;
        mpAttributePathParamsList          = other.mpAttributePathParamsList;
        mAttributePathParamsListSize       = other.mAttributePathParamsListSize;
        mEventNumber                       = other.mEventNumber;
        mMinIntervalSeconds                = other.mMinIntervalSeconds;
        mMaxIntervalSeconds                = other.mMaxIntervalSeconds;
        mTimeout                           = other.mTimeout;
        other.mpEventPathParamsList        = nullptr;
        other.mEventPathParamsListSize     = 0;
        other.mpAttributePathParamsList    = nullptr;
        other.mAttributePathParamsListSize = 0;

        return *this;
    }
};
} // namespace app
} // namespace chip
