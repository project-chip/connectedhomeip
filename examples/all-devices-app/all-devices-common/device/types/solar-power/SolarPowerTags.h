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

namespace chip::app::SolarPowerTags {

extern const EndpointComposition::SemanticTag kGridConnectionTag[1];
extern const EndpointComposition::SemanticTag kInverterTemperatureTag[1];
extern const EndpointComposition::SemanticTag kParallelPanel1Tag[1];
extern const EndpointComposition::SemanticTag kParallelPanel2Tag[1];
extern const EndpointComposition::SemanticTag kParallelPanel3Tag[1];
extern const EndpointComposition::SemanticTag kParallelPanel4Tag[1];
extern const EndpointComposition::SemanticTag kString1Tag[1];
extern const EndpointComposition::SemanticTag kString2Tag[1];
extern const EndpointComposition::SemanticTag kString1Panel1Tags[2];
extern const EndpointComposition::SemanticTag kString1Panel2Tags[2];
extern const EndpointComposition::SemanticTag kString2Panel1Tags[2];
extern const EndpointComposition::SemanticTag kString2Panel2Tags[2];
extern const EndpointComposition::SemanticTag kPhaseATag[1];
extern const EndpointComposition::SemanticTag kPhaseBTag[1];
extern const EndpointComposition::SemanticTag kPhaseCTag[1];

} // namespace chip::app::SolarPowerTags
