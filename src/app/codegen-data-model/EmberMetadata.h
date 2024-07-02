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

#include <app/util/af-types.h>
#include <lib/core/CHIPError.h>

#include <variant>

namespace chip {
namespace app {
namespace Ember {

/// Fetch the source for the given attribute path: either a cluster (for global ones) or attribute
/// path.
///
/// if returning a CHIP_ERROR, it will NEVER be CHIP_NO_ERROR.
std::variant<const EmberAfCluster *,           // global attribute, data from a cluster
             const EmberAfAttributeMetadata *, // a specific attribute stored by ember
             CHIP_ERROR                        // error, this will NEVER be CHIP_NO_ERROR
             >
FindAttributeMetadata(const ConcreteAttributePath & aPath);

} // namespace Ember
} // namespace app
} // namespace chip
