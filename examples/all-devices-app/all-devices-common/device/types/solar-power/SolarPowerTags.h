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

#pragma once

#include <device/api/Interface.h>
#include <lib/support/Span.h>

namespace chip::app::SolarPowerTags {

Span<const EndpointComposition::SemanticTag> GridConnectionTag();
Span<const EndpointComposition::SemanticTag> InverterTemperatureTag();
Span<const EndpointComposition::SemanticTag> ParallelPanel1Tag();
Span<const EndpointComposition::SemanticTag> ParallelPanel2Tag();
Span<const EndpointComposition::SemanticTag> ParallelPanel3Tag();
Span<const EndpointComposition::SemanticTag> ParallelPanel4Tag();
Span<const EndpointComposition::SemanticTag> String1Tag();
Span<const EndpointComposition::SemanticTag> String2Tag();
Span<const EndpointComposition::SemanticTag> String1Panel1Tags();
Span<const EndpointComposition::SemanticTag> String1Panel2Tags();
Span<const EndpointComposition::SemanticTag> String2Panel1Tags();
Span<const EndpointComposition::SemanticTag> String2Panel2Tags();
Span<const EndpointComposition::SemanticTag> PhaseATag();
Span<const EndpointComposition::SemanticTag> PhaseBTag();
Span<const EndpointComposition::SemanticTag> PhaseCTag();

} // namespace chip::app::SolarPowerTags
