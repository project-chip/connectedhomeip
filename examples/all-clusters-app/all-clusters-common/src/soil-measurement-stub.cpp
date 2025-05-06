/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/clusters/soil-measurement-server/soil-measurement-server.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SoilMeasurement;

static std::unique_ptr<SoilMeasurementAttrAccess> gAttrAccess;

void emberAfSoilMeasurementClusterInitCallback(EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(!gAttrAccess);

    gAttrAccess = std::make_unique<SoilMeasurementAttrAccess>();

    if (gAttrAccess)
    {
        gAttrAccess->Init();
    }
}

void emberAfSoilMeasurementClusterShutdownCallback(EndpointId endpointId)
{
    if (gAttrAccess)
    {
        gAttrAccess->Shutdown();
    }
    gAttrAccess = nullptr;
}
