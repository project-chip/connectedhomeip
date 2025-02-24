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
#include <app/clusters/laundry-washer-controls-server/laundry-washer-controls-server.h>
#include <app/util/config.h>
#include <chef-laundry-washer-controls-delegate-impl.h>

using namespace chip;
using namespace chip::app::Clusters::LaundryWasherControls;

const CharSpan LaundryWasherControlDelegate::spinSpeedsNameOptions[] = {
    CharSpan::fromCharString("Off"),
    CharSpan::fromCharString("Low"),
    CharSpan::fromCharString("Medium"),
    CharSpan::fromCharString("High"),
};

const NumberOfRinsesEnum LaundryWasherControlDelegate::supportRinsesOptions[] = {
    NumberOfRinsesEnum::kNone,
    NumberOfRinsesEnum::kNormal,
    NumberOfRinsesEnum::kExtra,
    NumberOfRinsesEnum::kMax,
};

LaundryWasherControlDelegate LaundryWasherControlDelegate::instance;

CHIP_ERROR LaundryWasherControlDelegate::GetSpinSpeedAtIndex(size_t index, MutableCharSpan & spinSpeed)
{
    if (index >= MATTER_ARRAY_SIZE(spinSpeedsNameOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    return chip::CopyCharSpanToMutableCharSpan(LaundryWasherControlDelegate::spinSpeedsNameOptions[index], spinSpeed);
}

CHIP_ERROR LaundryWasherControlDelegate::GetSupportedRinseAtIndex(size_t index, NumberOfRinsesEnum & supportedRinse)
{
    if (index >= MATTER_ARRAY_SIZE(supportRinsesOptions))
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    supportedRinse = LaundryWasherControlDelegate::supportRinsesOptions[index];
    return CHIP_NO_ERROR;
}

void emberAfLaundryWasherControlsClusterInitCallback(EndpointId endpoint)
{
    VerifyOrDie(endpoint == 1); // this cluster is only enabled for endpoint 1.
    LaundryWasherControlsServer::SetDefaultDelegate(1, &LaundryWasherControlDelegate::getLaundryWasherControlDelegate());
}
