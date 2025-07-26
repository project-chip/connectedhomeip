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
#include "app/EventLoggingDelegate.h"
#include "app/EventLoggingTypes.h"
#include <app/EventLogging.h>
#include <app/EventManagement.h>
#include <app/data-model/Encode.h>
#include <app/data-model/FabricScoped.h>
#include <app/data-model/List.h> // So we can encode lists

namespace chip::app::internal {

CHIP_ERROR LogEvent(EventLoggingDelegate * delegate, const EventOptions & eventOptions, EventNumber & outEventNumber,
                    bool isFabricScoped)
{
    if (isFabricScoped)
    {
        // this skips logging the event if it's fabric-scoped but no fabric association exists yet.
        VerifyOrReturnError(eventOptions.mFabricIndex != kUndefinedFabricIndex, CHIP_ERROR_INVALID_FABRIC_INDEX);
    }

    //
    // Unlike attributes which have a different 'EncodeForRead' for fabric-scoped structs,
    // fabric-sensitive events don't require that since the actual omission of the event in its entirety
    // happens within the event management framework itself at the time of access.
    //
    // The 'mFabricIndex' field in the event options above is encoded out-of-band alongside the event payload
    // and used to match against the accessing fabric.
    //
    return chip::app::EventManagement::GetInstance().LogEvent(delegate, eventOptions, outEventNumber);
}

} // namespace chip::app::internal
