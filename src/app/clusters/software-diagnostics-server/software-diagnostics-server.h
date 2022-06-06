/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <app/AttributeAccessInterface.h>
#include <app/CommandResponseHelper.h>
#include <platform/GeneralFaults.h>

namespace chip {
namespace app {
namespace Clusters {
namespace SoftwareDiagnostics {

/**
 * @brief general-diagnostics-server class
 */
class Server
{
public:
    static Server & Instance();

    /**
     * @brief
     *   Called when a software fault that has taken place on the Node.
     */
    void OnSoftwareFaultDetect(chip::app::Clusters::SoftwareDiagnostics::Structs::SoftwareFaultStruct::Type & softwareFault);

private:
    static Server instance;
};

} // namespace SoftwareDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
