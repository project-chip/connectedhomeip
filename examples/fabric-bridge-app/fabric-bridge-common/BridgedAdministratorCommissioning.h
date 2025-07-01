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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterfaceRegistry.h>

namespace bridge {

// Forward declaration: AdministratorCommissioning uses the device to initialize and
// handle interfaces, bridged device contains the AdministratorCommissioning
class BridgedDevice;

/**
 * @brief CADMIN cluster implementation for handling attribute interactions of bridged device endpoints.
 *
 */
class BridgedAdministratorCommissioning : public chip::app::AttributeAccessInterface
{
public:
    // Admin commissioning will be tied to the endpoint specific to this
    // device. Device `GetEndpointId` MUST be set.
    //
    // Automatically registers itself to the AttributeAccessInterfaceRegistry
    BridgedAdministratorCommissioning(BridgedDevice & device);
    virtual ~BridgedAdministratorCommissioning();

    /// The only AAI we support: reading some attributes
    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;

private:
    BridgedDevice & mDevice;
};

} // namespace bridge
