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

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

namespace chip::scenes {

/// Delegate interface for clusters to integrate with the Scenes cluster functionality.
///
/// This allows other clusters (e.g., OnOff, LevelControl, ColorControl) to interact with
/// scenes: store/recall global scenes or invalidate scenes based on various conditions.
class ScenesIntegrationDelegate
{
public:
    virtual ~ScenesIntegrationDelegate() = default;

    // == Global Scene Methods ==
    // These methods interact with the Global Scene (Group ID 0, Scene ID 0).

    /// Stores the current state into the Global Scene for the fabric.
    virtual CHIP_ERROR StoreCurrentGlobalScene(FabricIndex fabricIndex) = 0;

    /// Recalls the Global Scene for the given fabric.
    virtual CHIP_ERROR RecallGlobalScene(FabricIndex fabricIndex) = 0;

    // == General Scene Management Notifications ==

    /// Notifies that a group is about to be removed from the given fabric.
    ///
    /// This can be used to invalidate scenes that reference the given group.
    virtual CHIP_ERROR GroupWillBeRemoved(FabricIndex fabricIndex, GroupId groupId) = 0;

    /// Marks all scenes on the associated endpoint as invalid for all fabrics.
    virtual CHIP_ERROR MakeSceneInvalidForAllFabrics() = 0;
};

} // namespace chip::scenes
