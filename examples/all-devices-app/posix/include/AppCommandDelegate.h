/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "AllDevicesAppClusterImplementationRegistry.h"
#include <NamedPipeCommands.h>
#include <app/util/basic-types.h>
#include <json/json.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "AllDevicesAppNamedPipeCommandHandler.h"

class AllDevicesAppCommandDelegate : public NamedPipeCommandDelegate
{
public:
    void OnEventCommandReceived(const char * json) override;

    AllDevicesAppClusterImplementationRegistry & GetClusterImplementationRegistry() { return mRegistry; }

    void RegisterCommandHandler(std::unique_ptr<AllDevicesAppNamedPipeCommandHandler> handler);
    void RegisterCommandHandlers();

private:
    static void DispatchCommand(intptr_t context);

    // Dynamic cluster registration lists.
    // As the application registers code-driven cluster instances during startup, they are
    // added to these vectors. The named pipe command delegate looks up the appropriate cluster
    // instance by Endpoint ID (retrieved dynamically via cluster->GetPaths()[0].mEndpointId)
    // when executing commands.
    //
    AllDevicesAppClusterImplementationRegistry mRegistry;

    std::map<std::string, std::unique_ptr<AllDevicesAppNamedPipeCommandHandler>> mCommandHandlers;
};
