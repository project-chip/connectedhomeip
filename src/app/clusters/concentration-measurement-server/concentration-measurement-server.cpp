/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "concentration-measurement-server.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/EventLogging.h>
#include <app/data-model/Nullable.h>
#include <app/reporting/reporting.h>
#include <app/util/af.h>
#include <app/util/config.h>
#include <app/util/error-mapping.h>
#include <app/util/util.h>

#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ConcentrationMeasurement;
using chip::Protocols::InteractionModel::Status;

/**********************************************************
 * Attributes Definition
 *********************************************************/

ConcentrationMeasurementServer ConcentrationMeasurementServer::instance;

/**********************************************************
 * ConcentrationMeasurementServer Implementation
 *********************************************************/

ConcentrationMeasurementServer & ConcentrationMeasurementServer::Instance()
{
    return instance;
}

void ConcentrationMeasurementServer::initConcentrationMeasurementServer(EndpointId endpoint)
{
    emberAfPluginConcentrationMeasurementClusterServerPostInitCallback(endpoint);
}

EmberAfStatus ConcentrationMeasurementServer::getMeasuredValue(EndpointId endpoint,
                                                               chip::app::DataModel::Nullable<float> & currentValue)
{
    // read current MeasuredValue value
    EmberAfStatus status = Attributes::MeasuredValue::Get(endpoint, currentValue);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfAlarmClusterPrintln("ERR: reading MeasuredValue %x", status);
    }

    return status;
}

EmberAfStatus ConcentrationMeasurementServer::setMeasuredValue(chip::EndpointId endpoint,
                                                               chip::app::DataModel::Nullable<float> newValue)
{
    chip::app::DataModel::Nullable<float> currentValue;

    EmberAfStatus status = getMeasuredValue(endpoint, currentValue);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        // if the value is already what we want to set it to then do nothing
        if (currentValue != newValue)
        {
            // write new MeasuredValue value
            status = Attributes::MeasuredValue::Set(endpoint, newValue);
            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                emberAfAlarmClusterPrintln("ERR: writing MeasuredValue %x", status);
            }
        }
    }

    return status;
}

EmberAfStatus ConcentrationMeasurementServer::getMinMeasuredValue(EndpointId endpoint,
                                                                  chip::app::DataModel::Nullable<float> & currentValue)
{
    // read current MinMeasuredValue value
    EmberAfStatus status = Attributes::MinMeasuredValue::Get(endpoint, currentValue);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfAlarmClusterPrintln("ERR: reading MinMeasuredValue %x", status);
    }

    return status;
}

EmberAfStatus ConcentrationMeasurementServer::setMinMeasuredValue(chip::EndpointId endpoint,
                                                                  chip::app::DataModel::Nullable<float> newValue)
{
    chip::app::DataModel::Nullable<float> currentValue;

    EmberAfStatus status = getMinMeasuredValue(endpoint, currentValue);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        // if the value is already what we want to set it to then do nothing
        if (currentValue != newValue)
        {
            // write new MinMeasuredValue value
            status = Attributes::MinMeasuredValue::Set(endpoint, newValue);
            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                emberAfAlarmClusterPrintln("ERR: writing MinMeasuredValue %x", status);
            }
        }
    }

    return status;
}

EmberAfStatus ConcentrationMeasurementServer::getMaxMeasuredValue(EndpointId endpoint,
                                                                  chip::app::DataModel::Nullable<float> & currentValue)
{
    // read current MaxMeasuredValue value
    EmberAfStatus status = Attributes::MaxMeasuredValue::Get(endpoint, currentValue);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfAlarmClusterPrintln("ERR: reading MaxMeasuredValue %x", status);
    }

    return status;
}

EmberAfStatus ConcentrationMeasurementServer::setMaxMeasuredValue(chip::EndpointId endpoint,
                                                                  chip::app::DataModel::Nullable<float> newValue)
{
    chip::app::DataModel::Nullable<float> currentValue;

    EmberAfStatus status = getMaxMeasuredValue(endpoint, currentValue);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        // if the value is already what we want to set it to then do nothing
        if (currentValue != newValue)
        {
            // write new MaxMeasuredValue value
            status = Attributes::MaxMeasuredValue::Set(endpoint, newValue);
            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                emberAfAlarmClusterPrintln("ERR: writing MaxMeasuredValue %x", status);
            }
        }
    }

    return status;
}

EmberAfStatus ConcentrationMeasurementServer::getTolerance(EndpointId endpoint,
                                                           chip::app::DataModel::Nullable<float> & currentValue)
{
    // read current Tolerance value
    EmberAfStatus status = Attributes::Tolerance::Get(endpoint, currentValue);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfAlarmClusterPrintln("ERR: reading Tolerance %x", status);
    }

    return status;
}

EmberAfStatus ConcentrationMeasurementServer::setTolerance(chip::EndpointId endpoint,
                                                           chip::app::DataModel::Nullable<float> newValue)
{
    chip::app::DataModel::Nullable<float> currentValue;

    EmberAfStatus status = getTolerance(endpoint, currentValue);
    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        // if the value is already what we want to set it to then do nothing
        if (currentValue != newValue)
        {
            // write new Tolerance value
            status = Attributes::Tolerance::Set(endpoint, newValue);
            if (status != EMBER_ZCL_STATUS_SUCCESS)
            {
                emberAfAlarmClusterPrintln("ERR: writing Tolerance %x", status);
            }
        }
    }

    return status;
}

/**********************************************************
 * Callbacks Implementation
 *********************************************************/

void emberAfConcentrationMeasurementClusterServerInitCallback(chip::EndpointId endpoint)
{
    ConcentrationMeasurementServer::Instance().initConcentrationMeasurementServer(endpoint);
}

void emberAfPluginConcentrationMeasurementClusterServerPostInitCallback(EndpointId endpoint) {}

void MatterConcentrationMeasurementPluginServerInitCallback() {}
