/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "AppTask.h"
#include "ColorFormat.h"
#include "PWMDevice.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <lib/support/logging/CHIPLogging.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OnOff;

/** @brief OnOff Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 */
void emberAfOnOffClusterInitCallback(EndpointId endpoint)
{
    EmberAfStatus status;
    bool storedValue;

    // Read storedValue on/off value
    status = Attributes::OnOff::Get(1, &storedValue);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        // Set actual state to stored before reboot
        GetAppTask().GetPWMDevice().Set(storedValue);
    }

    GetAppTask().UpdateClusterState();
}
