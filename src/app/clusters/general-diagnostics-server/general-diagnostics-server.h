/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <app/AttributeAccessInterface.h>
#include <app/CommandResponseHelper.h>
#include <app/util/af.h>

namespace chip {
namespace app {
namespace Clusters {
namespace GeneralDiagnostics {

/**
 * @brief general-diagnostics-server class
 */
class Server
{
public:
    static Server & Instance();

    /**
     * @brief
     *   Called after the current device is rebooted.
     */
    void OnDeviceReboot(BootReasonType bootReason);

    /**
     * @brief
     *   Called when the Node detects a hardware fault has been raised.
     */
    void OnHardwareFaultsDetect(GeneralFaults<kMaxHardwareFaults> & previous, GeneralFaults<kMaxHardwareFaults> & current);

    /**
     * @brief
     *   Called when the Node detects a radio fault has been raised.
     */
    void OnRadioFaultsDetect(GeneralFaults<kMaxRadioFaults> & previous, GeneralFaults<kMaxRadioFaults> & current);

    /**
     * @brief
     *   Called when the Node detects a network fault has been raised.
     */
    void OnNetworkFaultsDetect(GeneralFaults<kMaxNetworkFaults> & previous, GeneralFaults<kMaxNetworkFaults> & current);

private:
    static Server instance;
};

} // namespace GeneralDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
