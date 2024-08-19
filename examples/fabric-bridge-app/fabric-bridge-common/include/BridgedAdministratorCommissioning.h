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

#include <app/AttributeAccessInterfaceRegistry.h>
#include <app-common/zap-generated/cluster-objects.h>

class BridgedAdministratorCommissioning : public chip::app::AttributeAccessInterface
{
public:
    // Register for the OperationalCredentials cluster on all endpoints.
    BridgedAdministratorCommissioning() :
        AttributeAccessInterface(chip::NullOptional, chip::app::Clusters::AdministratorCommissioning::Id)
    {}

    CHIP_ERROR Init();

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;

    // We currently do not allow for any writes
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder)
    {
        VerifyOrDie(mOriginalAttributeInterface);
        return mOriginalAttributeInterface->Write(aPath, aDecoder);
    }

private:
    chip::app::AttributeAccessInterface * mOriginalAttributeInterface = nullptr;
};
