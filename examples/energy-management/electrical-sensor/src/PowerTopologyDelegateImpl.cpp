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

#include <PowerTopologyDelegateImpl.h>

namespace chip {
namespace app {
namespace Clusters {
namespace PowerTopology {

CHIP_ERROR PowerTopologyDelegate::GetAvailableEndpointAtIndex(size_t index, EndpointId & endpointId)
{
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

CHIP_ERROR PowerTopologyDelegate::GetActiveEndpointAtIndex(size_t index, EndpointId & endpointId)
{
    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

CHIP_ERROR PowerTopologyInstance::Init()
{
    return Instance::Init();
}

void PowerTopologyInstance::Shutdown()
{
    Instance::Shutdown();
}

/*
 *  @brief  Creates a Delegate and Instance for PowerTopology
 *
 * The Instance is a container around the Delegate, so
 * create the Delegate first, then wrap it in the Instance
 * Then call the Instance->Init() to register the attribute and command handlers
 */
CHIP_ERROR PowerTopologyInit(chip::EndpointId endpointId, std::unique_ptr<PowerTopologyDelegate> & aDelegate,
                             std::unique_ptr<PowerTopologyInstance> & aInstance)
{
    CHIP_ERROR err;

    if (aDelegate || aInstance)
    {
        ChipLogError(AppServer, "PowerTopology Instance or Delegate already exist.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    aDelegate = std::make_unique<PowerTopologyDelegate>();
    if (!aDelegate)
    {
        ChipLogError(AppServer, "Failed to allocate memory for PowerTopology Delegate");
        return CHIP_ERROR_NO_MEMORY;
    }

    aInstance = std::make_unique<PowerTopologyInstance>(
        EndpointId(endpointId), *aDelegate, BitMask<PowerTopology::Feature, uint32_t>(PowerTopology::Feature::kNodeTopology));

    if (!aInstance)
    {
        ChipLogError(AppServer, "Failed to allocate memory for PowerTopology Instance");
        aDelegate.reset();
        return CHIP_ERROR_NO_MEMORY;
    }

    err = aInstance->Init(); /* Register Attribute & Command handlers */
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Init failed on PowerTopology Instance");
        aInstance.reset();
        aDelegate.reset();
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR PowerTopologyShutdown(std::unique_ptr<PowerTopologyInstance> & aInstance,
                                 std::unique_ptr<PowerTopologyDelegate> & aDelegate)
{
    /* Do this in the order Instance first, then delegate
     * Ensure we call the Instance->Shutdown to free attribute & command handlers first
     */
    if (aInstance)
    {
        /* deregister attribute & command handlers */
        aInstance->Shutdown();
        aInstance.reset();
    }

    if (aDelegate)
    {
        aDelegate.reset();
    }

    return CHIP_NO_ERROR;
}

} // namespace PowerTopology
} // namespace Clusters
} // namespace app
} // namespace chip
