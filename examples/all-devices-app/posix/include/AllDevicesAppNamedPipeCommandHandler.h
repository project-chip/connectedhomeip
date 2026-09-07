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

#include <app/util/basic-types.h>
#include <json/json.h>

// Forward declaration to avoid circular dependency. AllDevicesAppCommandDelegate
// registers a set of AllDevicesAppNamedPipeCommandHandler instances.
class AllDevicesAppCommandDelegate;

class AllDevicesAppNamedPipeCommandHandler
{
public:
    virtual ~AllDevicesAppNamedPipeCommandHandler()                                                                     = default;
    virtual const char * GetName() const                                                                                = 0;
    virtual void Handle(const Json::Value & json, AllDevicesAppCommandDelegate * delegate, chip::EndpointId endpointId) = 0;
};
