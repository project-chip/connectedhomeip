/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include "esp_err.h"
#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>
#include <lib/support/Span.h>

class OTARequestorNVSHelper
{
private:
    static const char namespaceStorage[];
    static const char softwareVersionName[];
    static const char otaProviderNodeId[];
    static const char otaProviderFabIndex[];
    static const char otaProviderEndpointId[];

public:
    static esp_err_t ReadSoftwareVersion(uint32_t & softwareVersion);
    static esp_err_t ReadProviderLocation(chip::NodeId & nodeId, chip::FabricIndex & fabIndex, chip::EndpointId & endpointId);
    static esp_err_t ClearAllData();
    static esp_err_t WriteSoftwareVersion(uint32_t softwareVersion);
    static esp_err_t WriteProviderParameters(chip::NodeId nodeId, chip::FabricIndex fabIndex, chip::EndpointId endpointId);
};
