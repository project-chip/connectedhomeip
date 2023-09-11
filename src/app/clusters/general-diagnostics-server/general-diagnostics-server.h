/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandResponseHelper.h>
#include <app/util/af.h>
#include <platform/GeneralFaults.h>

namespace chip {
namespace app {
namespace Clusters {

/**
 * @brief general-diagnostics-server class
 */
class GeneralDiagnosticsServer
{
public:
    static GeneralDiagnosticsServer & Instance();

    /**
     * @brief
     *   Called after the current device is rebooted.
     */
    void OnDeviceReboot(GeneralDiagnostics::BootReasonEnum bootReason);

    /**
     * @brief
     *   Called when the Node detects a hardware fault has been raised.
     */
    void OnHardwareFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> & previous,
                                const DeviceLayer::GeneralFaults<DeviceLayer::kMaxHardwareFaults> & current);

    /**
     * @brief
     *   Called when the Node detects a radio fault has been raised.
     */
    void OnRadioFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> & previous,
                             const DeviceLayer::GeneralFaults<DeviceLayer::kMaxRadioFaults> & current);

    /**
     * @brief
     *   Called when the Node detects a network fault has been raised.
     */
    void OnNetworkFaultsDetect(const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & previous,
                               const DeviceLayer::GeneralFaults<DeviceLayer::kMaxNetworkFaults> & current);

private:
    static GeneralDiagnosticsServer instance;
};

} // namespace Clusters
} // namespace app
} // namespace chip
