/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "TimeSyncManager.h"

using namespace chip::app::Clusters::TimeSynchronization;

void TimeSyncManager::HandleTimeZoneChanged(TimeZoneList timeZoneList)
{
    // do nothing
    ChipLogProgress(Zcl, "%s", __FUNCTION__);
}

CHIP_ERROR TimeSyncManager::HandleDstoffsetlookup()
{
    ChipLogProgress(Zcl, "%s", __FUNCTION__);
    return CHIP_NO_ERROR;
}

bool TimeSyncManager::HandleDstoffsetavailable(chip::CharSpan name)
{
    ChipLogProgress(Zcl, "%s", __FUNCTION__);
    return false;
}

CHIP_ERROR TimeSyncManager::HandleGetdstoffset()
{
    ChipLogProgress(Zcl, "%s", __FUNCTION__);
    return CHIP_NO_ERROR;
}

bool TimeSyncManager::isNTPAddressValid(chip::CharSpan ntp)
{
    return true;
}

bool TimeSyncManager::isNTPAddressDomain(chip::CharSpan ntp)
{
    return false;
}
