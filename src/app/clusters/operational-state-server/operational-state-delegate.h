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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/af-enums.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalState {
constexpr size_t kErrorStateLabelMaxSize       = 64u;
constexpr size_t kErrorStateDetailsMaxSize     = 64u;
constexpr size_t kOperationalStateLabelMaxSize = 64u;

struct OperationalErrorStateStruct
{
    uint8_t ErrorStateID;
    char ErrorStateLabel[kErrorStateLabelMaxSize];
    char ErrorStateDetails[kErrorStateDetailsMaxSize];
};

struct OperationalStateStruct
{
    uint8_t OperationalStateID;
    char OperationalStateLabel[kOperationalStateLabelMaxSize];
};

/**
 * A delegate to handle application logic of the Operational State aliased Cluster.
 */
class Delegate
{
public:
    /**
     * Handle Command Callback: Pause
     * @param state operational state.
     * @param error operational error.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    virtual void HandlePauseState(OperationalStateStruct & state, OperationalErrorStateStruct & error) = 0;

    /**
     * Handle Command Callback: Resume
     * @param state operational state.
     * @param error operational error.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    virtual void HandleResumeState(OperationalStateStruct & state, OperationalErrorStateStruct & error) = 0;

    /**
     * Handle Command Callback: Start
     * @param state operational state.
     * @param error operational error.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    virtual void HandleStartState(OperationalStateStruct & state, OperationalErrorStateStruct & error) = 0;

    /**
     * Handle Command Callback: Stop
     * @param state operational state.
     * @param error operational error.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    virtual void HandleStopState(OperationalStateStruct & state, OperationalErrorStateStruct & error) = 0;

    /**
     *  Init the delegate
     */
    virtual CHIP_ERROR Init() = 0;

    virtual ~Delegate() = default;
};

} // namespace OperationalState
} // namespace Clusters
} // namespace app
} // namespace chip
