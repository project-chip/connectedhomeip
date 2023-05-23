/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/clusters/operational-state-server/operational-state-delegate.h>
#include <app/util/af.h>
#include <app/util/config.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalState {

// This is an application level delegate to handle operational state commands according to the specific business logic.
class OperationalStateDelegate : public Delegate
{

private:
    CHIP_ERROR Init() override;

public:
    /**
     * Handle Command Callback: Pause
     * @param state operational state.
     * @param error operational error.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    void HandlePauseState(OperationalStateStruct & state, OperationalErrorStateStruct & error) override;

    /**
     * Handle Command Callback: Resume
     * @param state operational state.
     * @param error operational error.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    void HandleResumeState(OperationalStateStruct & state, OperationalErrorStateStruct & error) override;

    /**
     * Handle Command Callback: Start
     * @param state operational state.
     * @param error operational error.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    void HandleStartState(OperationalStateStruct & state, OperationalErrorStateStruct & error) override;

    /**
     * Handle Command Callback: Stop
     * @param state operational state.
     * @param error operational error.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    void HandleStopState(OperationalStateStruct & state, OperationalErrorStateStruct & error) override;

    ~OperationalStateDelegate() override = default;
};

} // namespace OperationalState
} // namespace Clusters
} // namespace app
} // namespace chip
