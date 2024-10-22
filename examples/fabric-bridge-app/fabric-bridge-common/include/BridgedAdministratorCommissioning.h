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

/**
 * @brief CADMIN cluster implementation for handling attribute interactions of bridged device endpoints.
 *
 * The current Administrator Commissioning Cluster server's zap generated code will automatically
 * register an Attribute Access Interface for the root node endpoint implementation. In order to
 * properly respond to a read attribute for bridged devices we are representing, we override the
 * currently registered Attribute Interface such that we are first to receive any read attribute
 * request on Administrator Commissioning Cluster, and if it is not an endpoint for a device we
 * are a bridge for we redirect to the default cluster server implementation of Administrator
 * Commissioning Cluster.
 */
class BridgedAdministratorCommissioning : public chip::app::AttributeAccessInterface
{
public:
    // Register for the AdministratorCommissioning cluster on all endpoints.
    BridgedAdministratorCommissioning() :
        AttributeAccessInterface(chip::NullOptional, chip::app::Clusters::AdministratorCommissioning::Id)
    {}

    CHIP_ERROR Init();

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;

    // We do not allow writing to CADMIN attributes of a bridged device endpoint. We simply redirect
    // write requests to the original attribute interface.
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override
    {
        VerifyOrDie(mOriginalAttributeInterface);
        return mOriginalAttributeInterface->Write(aPath, aDecoder);
    }

private:
    // If mOriginalAttributeInterface is removed from here, the class description needs to be updated
    // to reflect this change.
    chip::app::AttributeAccessInterface * mOriginalAttributeInterface = nullptr;
};
