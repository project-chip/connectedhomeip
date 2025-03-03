/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <ClosureControlInstance.h>

#include "silabs_utils.h"
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/closure-control-server/closure-control-server.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ClosureControl;

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

static std::unique_ptr<ClosureControlDelegate> gClosureCtrlDelegate;
static std::unique_ptr<ClosureControlInstance> gClosureCtrlInstance;

/*
 * @brief Creates a Delegate and Instance for Closure Control cluster
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Init() to register the attribute and command handlers
 */
CHIP_ERROR ClosureControlInit(EndpointId endpointId)
{
    CHIP_ERROR err;

    if (gClosureCtrlDelegate || gClosureCtrlInstance)
    {
        ChipLogError(AppServer, "ClosureControl Instance or Delegate already exist.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    gClosureCtrlDelegate = std::make_unique<ClosureControlDelegate>(endpointId);
    if (!gClosureCtrlDelegate)
    {
        ChipLogError(AppServer, "Failed to allocate memory for ClosureControlDelegate");
        return CHIP_ERROR_NO_MEMORY;
    }

    /* Manufacturer may optionally not support all features, commands & attributes */
    gClosureCtrlInstance = std::make_unique<ClosureControlInstance>(EndpointId(endpointId), *gClosureCtrlDelegate,
                                                                    Feature::kCalibration, OptionalAttribute::kCountdownTime);
    if (!gClosureCtrlInstance)
    {
        ChipLogError(AppServer, "Failed to allocate memory for ClosureControlInstance");
        gClosureCtrlDelegate.reset();
        return CHIP_ERROR_NO_MEMORY;
    }

    /* Register Attribute & Command handlers */
    err = gClosureCtrlInstance->Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "gClosureCtrlInstance->Init failed %s", chip::ErrorStr(err));
        gClosureCtrlInstance.reset();
        gClosureCtrlDelegate.reset();
        return err;
    }

    gClosureCtrlDelegate->SetClosureControlInstance(*gClosureCtrlInstance);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClosureControlShutdown()
{
    /* Do this in the order Instance first, then delegate
     * Ensure we call the Instance->Shutdown to free attribute & command handlers first
     */
    if (gClosureCtrlInstance)
    {
        /* Deregister attribute & command handlers */
        gClosureCtrlInstance->Shutdown();
        gClosureCtrlInstance.reset();
    }

    if (gClosureCtrlDelegate)
    {
        gClosureCtrlDelegate.reset();
    }

    return CHIP_NO_ERROR;
}

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
