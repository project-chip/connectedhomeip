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

/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/****************************************************************************
 * @file
 * @brief Routines for the Temperature Measurement Server
 *plugin.
 *******************************************************************************
 ******************************************************************************/

#include <app/util/af.h>

#include <app/util/af-event.h>
#include <app/util/attribute-storage.h>

#include "gen/attribute-id.h"
#include "gen/attribute-type.h"
#include "gen/cluster-id.h"

using namespace chip;

EmberEventControl emberAfPluginTemperatureMeasurementServerReadEventControl;

// TODO: There's no header that declares this event handler, and it's not 100%
// clear where best to declare it.
// https://github.com/project-chip/connectedhomeip/issues/3619
void emberAfPluginTemperatureMeasurementServerReadEventHandler() {}

void emberAfPluginTemperatureMeasurementServerStackStatusCallback(EmberStatus status) {}

// -------------------------------------------------------------------------
// ****** callback section *******

void emberAfPluginTemperatureMeasurementServerInitCallback(void)
{
    EmberAfStatus status;
    // FIXME Use real values for the temperature sensor polling the sensor using the
    //       EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER_MAX_MEASUREMENT_FREQUENCY_S macro
    EndpointId endpointId = 1; // Hardcoded to 1 for now
    int16_t newValue      = 0x1234;

    status = emberAfWriteAttribute(endpointId, ZCL_TEMP_MEASUREMENT_CLUSTER_ID, ZCL_CURRENT_TEMPERATURE_ATTRIBUTE_ID,
                                   CLUSTER_MASK_SERVER, (uint8_t *) &newValue, ZCL_INT16S_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        emberAfTempMeasurementClusterPrint("Err: writing temperature: %x", status);
        return;
    }
}
