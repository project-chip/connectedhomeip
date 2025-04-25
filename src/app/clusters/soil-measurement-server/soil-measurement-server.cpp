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

#include "soil-measurement-server.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SoilMeasurement;
using namespace chip::app::Clusters::SoilMeasurement::Attributes;

SoilMeasurementServer SoilMeasurementServer::mInstance;

SoilMeasurementServer & SoilMeasurementServer::Instance()
{
    return SoilMeasurementServer::mInstance;
}

CHIP_ERROR SoilMeasurementServer::ReadClusterRevision(AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(kClusterRevision);
}

CHIP_ERROR SoilMeasurementServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == SoilMeasurement::Id);

    switch (aPath.mAttributeId)
    {
    case Attributes::SoilMoistureMeasuredValue::Id: {
        DataModel::Nullable<uint16_t> soilMoistureMeasuredValue;
        soilMoistureMeasuredValue.SetNonNull(1);
        return aEncoder.Encode(soilMoistureMeasuredValue);
    }
    case ClusterRevision::Id: {
        return ReadClusterRevision(aEncoder);
    }
    default: {
        break;
    }
    }
    return CHIP_NO_ERROR;
}

void MatterSoilMeasurementPluginServerInitCallback()
{
    AttributeAccessInterfaceRegistry::Instance().Register(&SoilMeasurementServer::Instance());
}

void MatterSoilMeasurementPluginServerShutdownCallback()
{
    AttributeAccessInterfaceRegistry::Instance().Unregister(&SoilMeasurementServer::Instance());
}
