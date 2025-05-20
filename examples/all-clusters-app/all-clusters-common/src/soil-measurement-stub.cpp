/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SoilMeasurement;

static const Globals::Structs::MeasurementAccuracyRangeStruct::Type kDefaultSoilMoistureMeasurementLimitsAccuracyRange[] = {
    { .rangeMin = 0, .rangeMax = 100, .percentMax = MakeOptional(static_cast<chip::Percent100ths>(10)) }
};

static const Globals::Structs::MeasurementAccuracyStruct::Type kDefaultSoilMoistureMeasurementLimits = {
    .measurementType  = Globals::MeasurementTypeEnum::kSoilMoisture,
    .measured         = true,
    .minMeasuredValue = 0,
    .maxMeasuredValue = 100,
    .accuracyRanges   = DataModel::List<const Globals::Structs::MeasurementAccuracyRangeStruct::Type>(
        kDefaultSoilMoistureMeasurementLimitsAccuracyRange)
};

namespace {
static std::unique_ptr<Instance> gSoilMeasurementInstance;
} // namespace

Instance * SoilMeasurement::GetInstance()
{
    return gSoilMeasurementInstance.get();
}

void SoilMeasurement::Shutdown()
{
    VerifyOrDie(gSoilMeasurementInstance);
    gSoilMeasurementInstance->Shutdown();
    gSoilMeasurementInstance.reset(nullptr);
}

void emberAfSoilMeasurementClusterInitCallback(EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(!gSoilMeasurementInstance);

    gSoilMeasurementInstance = std::make_unique<Instance>(endpointId);
    if (gSoilMeasurementInstance)
    {
        gSoilMeasurementInstance->Init(kDefaultSoilMoistureMeasurementLimits);
    }
}
