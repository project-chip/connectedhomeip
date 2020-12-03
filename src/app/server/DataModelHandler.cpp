/*
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
 * @file
 *   This file implements the handler for data model messages.
 */

#include <app/server/DataModelHandler.h>

#include <app/chip-zcl-zpro-codec.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <lib/support/logging/CHIPLogging.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemPacketBuffer.h>

#ifdef EMBER_AF_PLUGIN_REPORTING
void emberAfPluginReportingStackStatusCallback(EmberStatus status);
#endif
#ifdef EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER
void emberAfPluginTemperatureMeasurementServerStackStatusCallback(EmberStatus status);
#endif
#ifdef EMBER_AF_PLUGIN_IAS_ZONE_SERVER
void emberAfPluginIasZoneServerStackStatusCallback(EmberStatus status);
#endif

using namespace ::chip;

/**
 * Handle a message that should be processed via our data model processing
 * codepath. This function will free the packet buffer.
 *
 * @param [in] buffer The buffer holding the message.  This function guarantees
 *                    that it will free the buffer before returning.
 */
void HandleDataModelMessage(const PacketHeader & header, System::PacketBufferHandle buffer, SecureSessionMgr * mgr)
{
    EmberApsFrame frame;
    bool ok = extractApsFrame(buffer->Start(), buffer->DataLength(), &frame) > 0;
    if (ok)
    {
        ChipLogProgress(Zcl, "APS frame processing success!");
    }
    else
    {
        ChipLogProgress(Zcl, "APS frame processing failure!");
        return;
    }

    uint8_t * message;
    uint16_t messageLen = extractMessage(buffer->Start(), buffer->DataLength(), &message);
    ok                  = emberAfProcessMessage(&frame,
                               0, // type
                               message, messageLen,
                               header.GetSourceNodeId().Value(), // source identifier
                               NULL);

    if (ok)
    {
        ChipLogProgress(Zcl, "Data model processing success!");
    }
    else
    {
        ChipLogProgress(Zcl, "Data model processing failure!");
    }
}

void InitDataModelHandler()
{
    emberAfEndpointConfigure();
    emberAfInit();

#if defined(EMBER_AF_PLUGIN_REPORTING) || defined(EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER) ||                               \
    defined(EMBER_AF_PLUGIN_IAS_ZONE_SERVER)
    EmberStatus status = EMBER_NETWORK_UP;
#endif

#ifdef EMBER_AF_PLUGIN_REPORTING
    emberAfPluginReportingStackStatusCallback(status);
#endif
#ifdef EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER
    emberAfPluginTemperatureMeasurementServerStackStatusCallback(status);
#endif
#ifdef EMBER_AF_PLUGIN_IAS_ZONE_SERVER
    emberAfPluginIasZoneServerStackStatusCallback(status);
#endif
}
