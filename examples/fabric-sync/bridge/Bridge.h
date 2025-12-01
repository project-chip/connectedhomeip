/*
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

#pragma once

#include "CommissionableInit.h"
#include "CommissionerControlDelegate.h"
#include "FabricAdminDelegate.h"

namespace bridge {

/**
 * @brief Initializes the local fabric bridge system.
 *
 * This function sets up and initializes all necessary components required for
 * device management and commissioning within the local fabric bridge. It interacts
 * with the provided FabricAdminDelegate instance to manage fabric-level operations.
 * Specifically, it registers command handlers, initializes device attributes, and
 * prepares the bridge for commissioning operations.
 *
 * @param delegate A pointer to a FabricAdminDelegate instance, allowing the bridge
 *        to initialize and manage fabric operations.
 *
 * @return CHIP_NO_ERROR if all initializations are successful, or an appropriate
 *         CHIP_ERROR code if an initialization step fails.
 */
CHIP_ERROR BridgeInit(FabricAdminDelegate * delegate);

/**
 * @brief Shuts down the local fabric bridge system.
 *
 * This function performs cleanup operations and shuts down the bridge system components
 * responsible for device management and commissioning. It stops the Commissioner Control
 * Server and handles any errors that may occur during shutdown.
 *
 * @return CHIP_NO_ERROR if shutdown is successful, or an appropriate CHIP_ERROR code if any component fails to shut down.
 */
CHIP_ERROR BridgeShutdown();

} // namespace bridge
