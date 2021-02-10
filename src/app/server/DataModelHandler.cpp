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

#include <app/chip-zcl-zpro-codec.h> // For EmberApsFrame
#if __has_include("gen/endpoint_config.h")
#define USE_ZAP_CONFIG 1
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#endif
#include <support/logging/CHIPLogging.h>

#ifdef EMBER_AF_PLUGIN_REPORTING_SERVER
void emberAfPluginReportingStackStatusCallback(EmberStatus status);
#endif
#ifdef EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER
void emberAfPluginTemperatureMeasurementServerStackStatusCallback(EmberStatus status);
#endif
#ifdef EMBER_AF_PLUGIN_IAS_ZONE_SERVER
void emberAfPluginIasZoneServerStackStatusCallback(EmberStatus status);
#endif

using namespace ::chip;

void InitDataModelHandler()
{
#ifdef USE_ZAP_CONFIG
    ChipLogProgress(Zcl, "Using ZAP configuration...");
    emberAfEndpointConfigure();
    emberAfInit();

#if defined(EMBER_AF_PLUGIN_REPORTING_SERVER) || defined(EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER) ||                        \
    defined(EMBER_AF_PLUGIN_IAS_ZONE_SERVER)
    EmberStatus status = EMBER_NETWORK_UP;
#endif

#ifdef EMBER_AF_PLUGIN_REPORTING_SERVER
    emberAfPluginReportingStackStatusCallback(status);
#endif
#ifdef EMBER_AF_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER
    emberAfPluginTemperatureMeasurementServerStackStatusCallback(status);
#endif
#ifdef EMBER_AF_PLUGIN_IAS_ZONE_SERVER
    emberAfPluginIasZoneServerStackStatusCallback(status);
#endif
#endif
}

static CHIP_ERROR CheckACL(NodeId nodeId, const EmberApsFrame & frame, const chip::Transport::AccessControlList & ACL)
{
    // TODO: Lookup the ACL corresponding to the nodeId, and the end point information from the frame,
    //       and enforce it.

    return CHIP_NO_ERROR;
}

void HandleDataModelMessage(NodeId nodeId, chip::Transport::AdminPairingInfo * adminInfo, System::PacketBufferHandle buffer)
{
    EmberApsFrame frame;
#ifdef USE_ZAP_CONFIG
    bool ok = extractApsFrame(buffer->Start(), buffer->DataLength(), &frame) > 0;
    if (ok)
    {
        ChipLogDetail(Zcl, "APS frame processing success!");
    }
    else
    {
        ChipLogDetail(Zcl, "APS frame processing failure!");
        return;
    }

    if (CHIP_NO_ERROR != CheckACL(nodeId, frame, adminInfo->GetACL()))
    {
        ChipLogDetail(Zcl, "ACL check failed for the received APS frame!");
        return;
    }

    uint8_t * message;
    uint16_t messageLen = extractMessage(buffer->Start(), buffer->DataLength(), &message);
    ok                  = emberAfProcessMessage(&frame,
                               0, // type
                               message, messageLen,
                               nodeId, // source identifier
                               NULL);

    if (ok)
    {
        ChipLogDetail(Zcl, "Data model processing success!");
    }
    else
    {
        ChipLogDetail(Zcl, "Data model processing failure!");
    }
#else
    if (CHIP_NO_ERROR != CheckACL(nodeId, frame, adminInfo->GetACL()))
    {
        ChipLogDetail(Zcl, "ACL check failed for the received APS frame!");
        return;
    }
#endif
}
