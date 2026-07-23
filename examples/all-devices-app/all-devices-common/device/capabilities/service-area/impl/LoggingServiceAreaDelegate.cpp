/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "LoggingServiceAreaDelegate.h"
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app::Clusters::ServiceArea {

bool LoggingServiceAreaDelegate::IsSetSelectedAreasAllowed(MutableCharSpan & statusText)
{
    return true;
}

bool LoggingServiceAreaDelegate::IsValidSelectAreasSet(const Span<const uint32_t> & selectedAreas,
                                                       SelectAreasStatus & locationStatus, MutableCharSpan & statusText)
{
    ChipLogProgress(Zcl, "LoggingServiceAreaDelegate: SelectAreas command received for %u area(s).",
                    static_cast<unsigned>(selectedAreas.size()));
    locationStatus = SelectAreasStatus::kSuccess;
    return true;
}

bool LoggingServiceAreaDelegate::IsSupportedAreasChangeAllowed()
{
    return true;
}

bool LoggingServiceAreaDelegate::IsSupportedMapChangeAllowed()
{
    return true;
}

} // namespace chip::app::Clusters::ServiceArea
