/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>

class BridgedDeviceBasicInformationImpl : public chip::app::AttributeAccessInterface
{
public:
    BridgedDeviceBasicInformationImpl() :
        chip::app::AttributeAccessInterface(chip::NullOptional /* endpointId */,
                                            chip::app::Clusters::BridgedDeviceBasicInformation::Id)
    {}

    // AttributeAccessInterface implementation
    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & path, chip::app::AttributeValueEncoder & encoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & path, chip::app::AttributeValueDecoder & decoder) override;
};
