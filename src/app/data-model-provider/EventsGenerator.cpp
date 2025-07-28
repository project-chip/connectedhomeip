/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <app/data-model-provider/EventsGenerator.h>

namespace chip::app::DataModel::internal {

std::optional<EventNumber> GenerateEvent(const EventOptions & eventOptions, EventsGenerator & generator,
                                         EventLoggingDelegate & delegate, bool isFabricSensitiveEvent)
{
    // this skips generating the event if it is fabric-sensitive but the provided event data is not
    // associated with any fabric.
    if (isFabricSensitiveEvent && (eventOptions.mFabricIndex == kUndefinedFabricIndex))
    {
        ChipLogError(EventLogging, "Event encode failure: no fabric index for fabric scoped event");
        return std::nullopt;
    }

    //
    // Unlike attributes which have a different 'EncodeForRead' for fabric-scoped structs,
    // fabric-sensitive events don't require that since the actual omission of the event in its entirety
    // happens within the event management framework itself at the time of access.
    //
    // The 'mFabricIndex' field in the event options above is encoded out-of-band alongside the event payload
    // and used to match against the accessing fabric.
    //
    EventNumber eventNumber;
    CHIP_ERROR err = generator.GenerateEvent(&delegate, eventOptions, eventNumber);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(EventLogging, "Failed to generate event: %" CHIP_ERROR_FORMAT, err.Format());
        return std::nullopt;
    }

    return eventNumber;
}

} // namespace chip::app::DataModel::internal
