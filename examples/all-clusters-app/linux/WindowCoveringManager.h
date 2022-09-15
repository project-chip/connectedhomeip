/**
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

#include <app/clusters/window-covering-server/window-covering-server.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WindowCovering {

class WindowCoveringManager : public Delegate
{
public:
    void Init(chip::EndpointId endpoint);
    CHIP_ERROR HandleMovement(WindowCoveringType type) override;
    CHIP_ERROR HandleStopMotion() override;

private:
    OperationalState mState;
    NPercent100ths mCurrentPosition;
    NPercent100ths mTargetPosition;

    /**
     * @brief
     *  The callback function to be called when "movement timer" expires.
     */
    static void HandleMovementTimer(chip::System::Layer * layer, void * aAppState);
};

} // namespace WindowCovering
} // namespace Clusters
} // namespace app
} // namespace chip
