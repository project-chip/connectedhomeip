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

#include <clusters/shared/Enums.h>

namespace chip::app::SolarPowerTags {

Span<const EndpointComposition::SemanticTag> GridConnectionTag()
{
    using NS                                             = Clusters::Globals::SemanticTagNamespaceId;
    static const EndpointComposition::SemanticTag kTag[] = {
        { .mfgCode = DataModel::NullNullable, .namespaceID = to_underlying(NS::kPowerSource), .tag = 0x01 },
    };
    return Span(kTag);
}

} // namespace chip::app::SolarPowerTags
