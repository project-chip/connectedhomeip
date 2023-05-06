/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af-types.h>
#include <app/util/basic-types.h>
#include <platform/CHIPDeviceConfig.h>

/**
 * @brief Concentration Measurement Server Plugin class
 */
class ConcentrationMeasurementServer
{
public:
    /**********************************************************
     * Functions Definitions
     *********************************************************/

    static ConcentrationMeasurementServer & Instance();

    void initConcentrationMeasurementServer(chip::EndpointId endpoint);

    EmberAfStatus getMeasuredValue(chip::EndpointId endpoint, chip::app::DataModel::Nullable<float> & currentValue);
    EmberAfStatus setMeasuredValue(chip::EndpointId endpoint, chip::app::DataModel::Nullable<float> newValue);
    EmberAfStatus getMinMeasuredValue(chip::EndpointId endpoint, chip::app::DataModel::Nullable<float> & currentValue);
    EmberAfStatus setMinMeasuredValue(chip::EndpointId endpoint, chip::app::DataModel::Nullable<float> newValue);
    EmberAfStatus getMaxMeasuredValue(chip::EndpointId endpoint, chip::app::DataModel::Nullable<float> & currentValue);
    EmberAfStatus setMaxMeasuredValue(chip::EndpointId endpoint, chip::app::DataModel::Nullable<float> newValue);
    EmberAfStatus getTolerance(chip::EndpointId endpoint, chip::app::DataModel::Nullable<float> & currentValue);
    EmberAfStatus setTolerance(chip::EndpointId endpoint, chip::app::DataModel::Nullable<float> newValue);

private:
    /**********************************************************
     * Functions Definitions
     *********************************************************/

    static ConcentrationMeasurementServer instance;
};

/**********************************************************
 * Global
 *********************************************************/

/**********************************************************
 * Callbacks
 *********************************************************/

/** @brief Concentration Measurement Cluster Server Post Init
 *
 * Following resolution of the Concentration Measurement state at startup for this endpoint,
 * perform any additional initialization needed; e.g., synchronize hardware
 * state.
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 */
void emberAfPluginConcentrationMeasurementClusterServerPostInitCallback(chip::EndpointId endpoint);
