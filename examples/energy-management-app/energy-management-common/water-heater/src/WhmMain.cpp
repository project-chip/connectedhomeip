/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <WhmInstance.h>
#include <WhmManufacturer.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/water-heater-management-server/water-heater-management-server.h>
#include <lib/support/logging/CHIPLogging.h>

static constexpr int WHM_ENDPOINT = 1;

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;

namespace chip {
namespace app {
namespace Clusters {
namespace WaterHeaterManagement {

static std::unique_ptr<WaterHeaterManagementDelegate> gWhmDelegate;
static std::unique_ptr<WaterHeaterManagementInstance> gWhmInstance;

static std::unique_ptr<WhmManufacturer> gWhmManufacturer;

WhmManufacturer * GetWhmManufacturer()
{
    return gWhmManufacturer.get();
}

/*
 *  @brief  Creates a Delegate and Instance for Water Heater Management cluster
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */
CHIP_ERROR WhmInit()
{
    CHIP_ERROR err;

    if (gWhmDelegate || gWhmInstance)
    {
        ChipLogError(AppServer, "WaterHeaterManager Instance or Delegate already exist.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    gWhmDelegate = std::make_unique<WaterHeaterManagementDelegate>(WHM_ENDPOINT);
    if (!gWhmDelegate)
    {
        ChipLogError(AppServer, "Failed to allocate memory for WaterHeaterManagementDelegate");
        return CHIP_ERROR_NO_MEMORY;
    }

    /* Manufacturer may optionally not support all features, commands & attributes */
    gWhmInstance = std::make_unique<WaterHeaterManagementInstance>(
        EndpointId(WHM_ENDPOINT), *gWhmDelegate, BitMask<Feature>(Feature::kEnergyManagement, Feature::kTankPercent));
    if (!gWhmInstance)
    {
        ChipLogError(AppServer, "Failed to allocate memory for WaterHeaterManagementInstance");
        gWhmDelegate.reset();
        return CHIP_ERROR_NO_MEMORY;
    }

    /* Register Attribute & Command handlers */
    err = gWhmInstance->Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "gWhmInstance->Init failed %s", chip::ErrorStr(err));
        gWhmInstance.reset();
        gWhmDelegate.reset();
        return err;
    }

    gWhmDelegate->SetWaterHeaterManagementInstance(*gWhmInstance);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WhmShutdown()
{
    /* Do this in the order Instance first, then delegate
     * Ensure we call the Instance->Shutdown to free attribute & command handlers first
     */
    if (gWhmInstance)
    {
        /* Deregister attribute & command handlers */
        gWhmInstance->Shutdown();
        gWhmInstance.reset();
    }

    if (gWhmDelegate)
    {
        gWhmDelegate.reset();
    }

    return CHIP_NO_ERROR;
}

/*
 *  @brief  Creates a WhmManufacturer class to hold the Whm cluster
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */
CHIP_ERROR WhmManufacturerInit()
{
    CHIP_ERROR err;

    if (gWhmManufacturer)
    {
        ChipLogError(AppServer, "WhmManufacturer already exist.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    /* Now create WhmManufacturer */
    gWhmManufacturer = std::make_unique<WhmManufacturer>(gWhmInstance.get());
    if (!gWhmManufacturer)
    {
        ChipLogError(AppServer, "Failed to allocate memory for WhmManufacturer");
        return CHIP_ERROR_NO_MEMORY;
    }

    /* Call Manufacturer specific init */
    err = gWhmManufacturer->Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Init failed on gWhmManufacturer");
        gWhmManufacturer.reset();
        return err;
    }

    // Let the WhmDelegate know about the WhmManufacturer object.
    gWhmDelegate->SetWhmManufacturer(*gWhmManufacturer);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WhmManufacturerShutdown()
{
    if (gWhmManufacturer)
    {
        /* Shutdown the WhmManufacturer */
        gWhmManufacturer->Shutdown();
        gWhmManufacturer.reset();
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR WhmApplicationInit()
{
    ReturnErrorOnFailure(WhmInit());

    /* Do this last so that the instances for other clusters can be wrapped inside */
    ReturnErrorOnFailure(WhmManufacturerInit());

    return CHIP_NO_ERROR;
}

CHIP_ERROR WhmApplicationShutdown()
{
    /* Shutdown in reverse order that they were created */
    WhmManufacturerShutdown();

    return WhmShutdown();
}

} // namespace WaterHeaterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
