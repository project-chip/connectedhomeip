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

#include "DefaultTimeSyncDelegate.h"
#include "inet/IPAddress.h"

using chip::TimeSyncDataProvider;
using namespace chip::app::Clusters::TimeSynchronization;

void DefaultTimeSyncDelegate::TimeZoneListChanged(const Span<TimeSyncDataProvider::TimeZoneStore> timeZoneList)
{
    // placeholder implementation
}

bool DefaultTimeSyncDelegate::HandleUpdateDSTOffset(chip::CharSpan name)
{
    // placeholder implementation
    return false;
}

bool DefaultTimeSyncDelegate::IsNTPAddressValid(chip::CharSpan ntp)
{
    // placeholder implementation
    chip::Inet::IPAddress addr;
    return chip::Inet::IPAddress::FromString(ntp.data(), ntp.size(), addr) && addr.IsIPv6();
}

bool DefaultTimeSyncDelegate::IsNTPAddressDomain(chip::CharSpan ntp)
{
    // placeholder implementation
    return false;
}
