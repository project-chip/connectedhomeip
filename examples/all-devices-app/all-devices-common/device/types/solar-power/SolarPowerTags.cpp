/*
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

#include <device/types/solar-power/SolarPowerTags.h>

#include <lib/core/CHIPError.h>

namespace chip::app::SolarPowerTags {

Span<const EndpointComposition::SemanticTag> ParallelPanelTag(uint8_t panelIndex)
{
    switch (panelIndex)
    {
    case 1:
        return Span(kParallelPanel1Tag);
    case 2:
        return Span(kParallelPanel2Tag);
    case 3:
        return Span(kParallelPanel3Tag);
    case 4:
        return Span(kParallelPanel4Tag);
    default:
        return Span<const EndpointComposition::SemanticTag>();
    }
}

Span<const EndpointComposition::SemanticTag> StringPanelTags(uint8_t stringIndex, uint8_t panelIndexInString)
{
    if (stringIndex == 1 && panelIndexInString == 1)
    {
        return Span(kString1Panel1Tags);
    }
    if (stringIndex == 1 && panelIndexInString == 2)
    {
        return Span(kString1Panel2Tags);
    }
    if (stringIndex == 2 && panelIndexInString == 1)
    {
        return Span(kString2Panel1Tags);
    }
    if (stringIndex == 2 && panelIndexInString == 2)
    {
        return Span(kString2Panel2Tags);
    }
    return Span<const EndpointComposition::SemanticTag>();
}

} // namespace chip::app::SolarPowerTags
