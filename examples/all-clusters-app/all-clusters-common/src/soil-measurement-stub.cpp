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

#include <soil-measurement-stub.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SoilMeasurement;

Instance * gSoilMeasurementInstance = nullptr;

Instance * SoilMeasurement::GetInstance()
{
    return gSoilMeasurementInstance;
}

void SoilMeasurement::Shutdown()
{
    if (gSoilMeasurementInstance != nullptr)
    {
        delete gSoilMeasurementInstance;
        gSoilMeasurementInstance = nullptr;
    }
}

void emberAfSoilMeasurementClusterInitCallback(EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(!gSoilMeasurementInstance);

    gSoilMeasurementInstance = new Instance(endpointId);
    if (gSoilMeasurementInstance)
    {
        gSoilMeasurementInstance->Init();
    }
}

void emberAfSoilMeasurementClusterShutdownCallback(EndpointId endpointId)
{
    if (gSoilMeasurementInstance)
    {
        SoilMeasurement::Shutdown();
    }
    gSoilMeasurementInstance = nullptr;
}
