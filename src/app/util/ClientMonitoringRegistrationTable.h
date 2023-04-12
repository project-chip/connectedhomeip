/**
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

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/core/CHIPConfig.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceConfig.h>

namespace chip {
class ClientMonitoringRegistrationTable
{
public:
    using MonitoringRegistrationStruct = chip::app::Clusters::ClientMonitoring::Structs::MonitoringRegistration::Type;

    ClientMonitoringRegistrationTable(FabricIndex fabricIndex);
    ~ClientMonitoringRegistrationTable(){};

    void SaveToStorage();

    // Getter
    NodeId getClientNodeId();
    uint64_t getICid();
    FabricIndex getFaricIndex();

    // Setter
    void setClientNodeId(NodeId clientNodeId);
    void setICid(uint64_t ICid);
    void setFabricIndex(FabricIndex fabric);

private:
    void LoadFromStorage(FabricIndex fabricIndex);
    MonitoringRegistrationStruct mRegisteredClient;
};

} // namespace chip
