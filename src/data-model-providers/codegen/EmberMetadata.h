/*
 *    Copyright (c) 2024 Project CHIP Authors
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
#pragma once

#include "app/ConcreteClusterPath.h"
#include <app/util/af-types.h>
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

#include <variant>

namespace chip {
namespace app {
namespace Ember {

/// Fetch the source for the given attribute path: either a cluster (for global ones) or attribute
/// path.
///
/// Possible return values:
///    - EmberAfAttributeMetadata (NEVER null) - if the attribute is known to ember datastore
///    - Status, only specifically for unknown attributes, may only be one of:
///        - Status::UnsupportedEndpoint
///        - Status::UnsupportedCluster
///        - Status::UnsupportedAttribute
std::variant<const EmberAfAttributeMetadata *,   // a specific attribute stored by ember
             Protocols::InteractionModel::Status // one of Status::Unsupported*
             >
FindAttributeMetadata(const ConcreteAttributePath & aPath);

} // namespace Ember
} // namespace app
} // namespace chip
