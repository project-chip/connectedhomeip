/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <inttypes.h>
#include <stddef.h>

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace Revision {

/**
 * A monothonic number identifying the interaction model revision.
 *
 * See section 8.1.1. "Revision History" in the "Interaction Model
 * Specification" chapter of the core Matter specification.
 */
inline constexpr InteractionModelRevision kInteractionModelRevision = 12;
inline constexpr uint8_t kInteractionModelRevisionTag               = 0xFF;

/**
 * A monotonic number identifying the revision number of the Data Model against
 * which the Node is certified.
 *
 * See section 7.1.1. "Revision History" in the "Data Model Specification"
 * chapter of the core Matter specification.
 */
inline constexpr uint16_t kDataModelRevision = 18;

/*
 * A number identifying the specification version against which the
 * Node is certified.
 *
 * See section 11.1.5.22. "SpecificationVersion Attribute" in "Service and
 * Device Management" chapter of the core Matter specification.
 */
inline constexpr uint32_t kSpecificationVersion = 0x01040100;

} // namespace Revision
} // namespace chip
