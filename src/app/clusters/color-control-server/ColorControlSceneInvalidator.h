/**
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

#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {

/// Injected hook that lets the Color Control cluster tell Scene Management that the live color changed
/// (so stored scenes become stale), WITHOUT the core cluster depending on the Scenes cluster, the
/// ScenesServer singleton, or Ember. A concrete implementation lives in the codegen/app layer and calls
/// ScenesServer; the core only sees this abstract interface. Null injection == no scene coupling.
class ColorControlSceneInvalidator
{
public:
    virtual ~ColorControlSceneInvalidator() = default;

    /// Called whenever the cluster's current color changes, so the endpoint's stored scenes are marked
    /// stale for every fabric.
    virtual void InvalidateScenes(EndpointId endpoint) = 0;
};

} // namespace Clusters
} // namespace app
} // namespace chip
