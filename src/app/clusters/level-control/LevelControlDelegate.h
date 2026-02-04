/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/data-model/Nullable.h>
#include <clusters/LevelControl/Enums.h>
#include <lib/support/BitMask.h>

namespace chip::app::Clusters {

class LevelControlDelegate
{
public:
    virtual ~LevelControlDelegate() = default;

    // Basic Level Control notifications
    virtual void OnOptionsChanged(BitMask<LevelControl::OptionsBitmap> options) {}
    virtual void OnOnLevelChanged(DataModel::Nullable<uint8_t> onLevel) {}
    virtual void OnDefaultMoveRateChanged(DataModel::Nullable<uint8_t> defaultMoveRate) {}
    virtual void OnLevelChanged(uint8_t level) {}
};

} // namespace chip::app::Clusters
