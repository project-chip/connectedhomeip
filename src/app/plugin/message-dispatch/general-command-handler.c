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
 *    @file
 *      This file provides ZCL General message processing dispatch.
 *
 */

#include "general-command-handler.h"

// Forward declarations
static ChipZclStatus_t chipZclReadAttributesCommandParse(ChipZclCommandContext_t * context);
static ChipZclStatus_t chipZclReadAttributesResponseCommandParse(ChipZclCommandContext_t * context);
static ChipZclStatus_t chipZclWriteAttributesCommandParse(ChipZclCommandContext_t * context);
static ChipZclStatus_t chipZclWriteAttributesUndividedCommandParse(ChipZclCommandContext_t * context);
static ChipZclStatus_t chipZclWriteAttributesResponseCommandParse(ChipZclCommandContext_t * context);
static ChipZclStatus_t chipZclWriteAttributesNoResponseCommandParse(ChipZclCommandContext_t * context);
static ChipZclStatus_t chipZclConfigureReportingCommandParse(ChipZclCommandContext_t * context);
static ChipZclStatus_t chipZclConfigureReportingResponseCommandParse(ChipZclCommandContext_t * context);
static ChipZclStatus_t chipZclReadReportingConfigurationCommandParse(ChipZclCommandContext_t * context);
static ChipZclStatus_t chipZclReadReportingConfigurationResponseCommandParse(ChipZclCommandContext_t * context);
static ChipZclStatus_t chipZclReportAttributesCommandParse(ChipZclCommandContext_t * context);
static ChipZclStatus_t chipZclDefaultResponseCommandParse(ChipZclCommandContext_t * context);
static ChipZclStatus_t chipZclDiscoverAttributesCommandParse(ChipZclCommandContext_t * context);
static ChipZclStatus_t chipZclDiscoverAttributesResponseCommandParse(ChipZclCommandContext_t * context);
static ChipZclStatus_t chipZclReadAttributesStructuredCommandParse(ChipZclCommandContext_t * context);
static ChipZclStatus_t chipZclWriteAttributesStructuredCommandParse(ChipZclCommandContext_t * context);
static ChipZclStatus_t chipZclWriteAttributesStructuredResponseCommandParse(ChipZclCommandContext_t * context);
static ChipZclStatus_t chipZclDiscoverCommandsReceivedCommandParse(ChipZclCommandContext_t * context);
static ChipZclStatus_t chipZclDiscoverCommandsReceivedResponseCommandParse(ChipZclCommandContext_t * context);
static ChipZclStatus_t chipZclDiscoverCommandsGeneratedCommandParse(ChipZclCommandContext_t * context);
static ChipZclStatus_t chipZclDiscoverCommandsGeneratedResponseCommandParse(ChipZclCommandContext_t * context);
static ChipZclStatus_t chipZclDiscoverAttributesExtendedCommandParse(ChipZclCommandContext_t * context);
static ChipZclStatus_t chipZclDiscoverAttributesExtendedResponseCommandParse(ChipZclCommandContext_t * context);

// Main general command parsing controller.
ChipZclStatus_t chipZclGeneralCommandParse(ChipZclCommandContext_t * context)
{
    ChipZclStatus_t result = CHIP_ZCL_STATUS_UNSUP_GENERAL_COMMAND;
    switch (context->commandId)
    {
    case ZCL_READ_ATTRIBUTES_COMMAND_ID:
        result = chipZclReadAttributesCommandParse(context);
        break;
    case ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID:
        result = chipZclReadAttributesResponseCommandParse(context);
        break;
    case ZCL_WRITE_ATTRIBUTES_COMMAND_ID:
        result = chipZclWriteAttributesCommandParse(context);
        break;
    case ZCL_WRITE_ATTRIBUTES_UNDIVIDED_COMMAND_ID:
        result = chipZclWriteAttributesUndividedCommandParse(context);
        break;
    case ZCL_WRITE_ATTRIBUTES_RESPONSE_COMMAND_ID:
        result = chipZclWriteAttributesResponseCommandParse(context);
        break;
    case ZCL_WRITE_ATTRIBUTES_NO_RESPONSE_COMMAND_ID:
        result = chipZclWriteAttributesNoResponseCommandParse(context);
        break;
    case ZCL_CONFIGURE_REPORTING_COMMAND_ID:
        result = chipZclConfigureReportingCommandParse(context);
        break;
    case ZCL_CONFIGURE_REPORTING_RESPONSE_COMMAND_ID:
        result = chipZclConfigureReportingResponseCommandParse(context);
        break;
    case ZCL_READ_REPORTING_CONFIGURATION_COMMAND_ID:
        result = chipZclReadReportingConfigurationCommandParse(context);
        break;
    case ZCL_READ_REPORTING_CONFIGURATION_RESPONSE_COMMAND_ID:
        result = chipZclReadReportingConfigurationResponseCommandParse(context);
        break;
    case ZCL_REPORT_ATTRIBUTES_COMMAND_ID:
        result = chipZclReportAttributesCommandParse(context);
        break;
    case ZCL_DEFAULT_RESPONSE_COMMAND_ID:
        result = chipZclDefaultResponseCommandParse(context);
        break;
    case ZCL_DISCOVER_ATTRIBUTES_COMMAND_ID:
        result = chipZclDiscoverAttributesCommandParse(context);
        break;
    case ZCL_DISCOVER_ATTRIBUTES_RESPONSE_COMMAND_ID:
        result = chipZclDiscoverAttributesResponseCommandParse(context);
        break;
    case ZCL_READ_ATTRIBUTES_STRUCTURED_COMMAND_ID:
        result = chipZclReadAttributesStructuredCommandParse(context);
        break;
    case ZCL_WRITE_ATTRIBUTES_STRUCTURED_COMMAND_ID:
        result = chipZclWriteAttributesStructuredCommandParse(context);
        break;
    case ZCL_WRITE_ATTRIBUTES_STRUCTURED_RESPONSE_COMMAND_ID:
        result = chipZclWriteAttributesStructuredResponseCommandParse(context);
        break;
    case ZCL_DISCOVER_COMMANDS_RECEIVED_COMMAND_ID:
        result = chipZclDiscoverCommandsReceivedCommandParse(context);
        break;
    case ZCL_DISCOVER_COMMANDS_RECEIVED_RESPONSE_COMMAND_ID:
        result = chipZclDiscoverCommandsReceivedResponseCommandParse(context);
        break;
    case ZCL_DISCOVER_COMMANDS_GENERATED_COMMAND_ID:
        result = chipZclDiscoverCommandsGeneratedCommandParse(context);
        break;
    case ZCL_DISCOVER_COMMANDS_GENERATED_RESPONSE_COMMAND_ID:
        result = chipZclDiscoverCommandsGeneratedResponseCommandParse(context);
        break;
    case ZCL_DISCOVER_ATTRIBUTES_EXTENDED_COMMAND_ID:
        result = chipZclDiscoverAttributesExtendedCommandParse(context);
        break;
    case ZCL_DISCOVER_ATTRIBUTES_EXTENDED_RESPONSE_COMMAND_ID:
        result = chipZclDiscoverAttributesExtendedResponseCommandParse(context);
        break;
    default:
        // Unrecognized general command ID, error status will apply.
        break;
    }
    return result;
}

static ChipZclStatus_t chipZclReadAttributesCommandParse(ChipZclCommandContext_t * context)
{
    // TODO: This is a total work in progress, it doesn't do anything like what needs to be done, we need to
    // add in the retrieval of the Cluster Command Spec from the zcl-data-model, pass it to the codec
    // to get the command parsed. Then it needs to use the parsed request to interact with the zcl-data-model
    // again and create the response. Everything goes into the command context

    chipZclCorePrintln("Rx: Read Attributes Command");

    // TODO: Get Command Struct Spec from the attribute and command DB so that incoming request can be populated

    // TODO: Pass context along with command struct spec into codec to get request populated

    ChipZclGeneralCommandReadAttributesRequest_t * request = (ChipZclGeneralCommandReadAttributesRequest_t *) context->request;
    chipZclCorePrintln("Attribute Count: %d", request->count);
    for (uint16_t i = 0; i < request->count; i++)
    {
        ChipZclAttributeMetadata * metadata = chipZclLocateAttributeMetadata(
            context->endpointId, context->clusterId, request->attributes[i], CLUSTER_MASK_SERVER, CHIP_ZCL_NULL_MANUFACTURER_CODE);
        if (metadata != NULL)
        {
            chipZclCorePrintln("Metadata default value: %d", metadata->defaultValue);
        }
        else
        {
            chipZclCorePrintln("Metadata is null for %d, %d, %d, %d, %d", context->endpointId, context->clusterId,
                               request->attributes[i], CLUSTER_MASK_SERVER, CHIP_ZCL_NULL_MANUFACTURER_CODE);
        }
    }
    return CHIP_ZCL_STATUS_SUCCESS;
}

static ChipZclStatus_t chipZclReadAttributesResponseCommandParse(ChipZclCommandContext_t * context)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

static ChipZclStatus_t chipZclWriteAttributesCommandParse(ChipZclCommandContext_t * context)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

static ChipZclStatus_t chipZclWriteAttributesUndividedCommandParse(ChipZclCommandContext_t * context)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

static ChipZclStatus_t chipZclWriteAttributesResponseCommandParse(ChipZclCommandContext_t * context)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

static ChipZclStatus_t chipZclWriteAttributesNoResponseCommandParse(ChipZclCommandContext_t * context)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

static ChipZclStatus_t chipZclConfigureReportingCommandParse(ChipZclCommandContext_t * context)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}
static ChipZclStatus_t chipZclConfigureReportingResponseCommandParse(ChipZclCommandContext_t * context)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

static ChipZclStatus_t chipZclReadReportingConfigurationCommandParse(ChipZclCommandContext_t * context)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

static ChipZclStatus_t chipZclReadReportingConfigurationResponseCommandParse(ChipZclCommandContext_t * context)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

static ChipZclStatus_t chipZclReportAttributesCommandParse(ChipZclCommandContext_t * context)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

static ChipZclStatus_t chipZclDefaultResponseCommandParse(ChipZclCommandContext_t * context)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

static ChipZclStatus_t chipZclDiscoverAttributesCommandParse(ChipZclCommandContext_t * context)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

static ChipZclStatus_t chipZclDiscoverAttributesResponseCommandParse(ChipZclCommandContext_t * context)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

static ChipZclStatus_t chipZclReadAttributesStructuredCommandParse(ChipZclCommandContext_t * context)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

static ChipZclStatus_t chipZclWriteAttributesStructuredCommandParse(ChipZclCommandContext_t * context)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

static ChipZclStatus_t chipZclWriteAttributesStructuredResponseCommandParse(ChipZclCommandContext_t * context)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

static ChipZclStatus_t chipZclDiscoverCommandsReceivedCommandParse(ChipZclCommandContext_t * context)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

static ChipZclStatus_t chipZclDiscoverCommandsReceivedResponseCommandParse(ChipZclCommandContext_t * context)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

static ChipZclStatus_t chipZclDiscoverCommandsGeneratedCommandParse(ChipZclCommandContext_t * context)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

static ChipZclStatus_t chipZclDiscoverCommandsGeneratedResponseCommandParse(ChipZclCommandContext_t * context)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

static ChipZclStatus_t chipZclDiscoverAttributesExtendedCommandParse(ChipZclCommandContext_t * context)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

static ChipZclStatus_t chipZclDiscoverAttributesExtendedResponseCommandParse(ChipZclCommandContext_t * context)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}
