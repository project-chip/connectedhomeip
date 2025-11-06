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

/**
 * This file contains backwards-compatibility cluster object definitions.  This file
 * is meant to be included at the end of cluster-objects.h, after all the normal
 * definitions are available.
 */
#pragma once

#ifndef CHIP_INCLUDING_FROM_CLUSTER_OBJECTS
#error "CompatClusterObjects.h must only be included from cluster-objects.h"
#endif // CHIP_INCLUDING_FROM_CLUSTER_OBJECTS

namespace chip {
namespace app {
namespace Clusters {

namespace Descriptor {
namespace Structs {

namespace SemanticTagStruct = Globals::Structs::SemanticTagStruct;

} // namespace Structs
} // namespace Descriptor

} // namespace Clusters
} // namespace app
} // namespace chip
