/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {

namespace Globals {
namespace Attributes {

namespace GeneratedCommandList {
static constexpr AttributeId Id = 0x0000FFF8;
} // namespace GeneratedCommandList

namespace AcceptedCommandList {
static constexpr AttributeId Id = 0x0000FFF9;
} // namespace AcceptedCommandList

// NOTE: EventList has been deprecated and should not be exposed in new clusters
namespace EventList {
static constexpr AttributeId Id = 0x0000FFFA;
} // namespace EventList

namespace AttributeList {
static constexpr AttributeId Id = 0x0000FFFB;
} // namespace AttributeList

namespace FeatureMap {
static constexpr AttributeId Id = 0x0000FFFC;
} // namespace FeatureMap

namespace ClusterRevision {
static constexpr AttributeId Id = 0x0000FFFD;
} // namespace ClusterRevision

} // namespace Attributes
} // namespace Globals
} // namespace Clusters
} // namespace app
} // namespace chip
