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
#include <platform/GeneralFaults.h>

namespace chip {
namespace app {
namespace Clusters {

/**
 * @brief software-diagnostics-server class
 */
class SoftwareDiagnosticsServer
{
public:
    static SoftwareDiagnosticsServer & Instance();

    /**
     * @brief
     *   Called when a software fault that has taken place on the Node.
     */
    void OnSoftwareFaultDetect(const chip::app::Clusters::SoftwareDiagnostics::Events::SoftwareFault::Type & softwareFault);

private:
    static SoftwareDiagnosticsServer instance;
};

} // namespace Clusters
} // namespace app
} // namespace chip
