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

#include "chip-zcl.h"
#include "gen-command-id.h"

static ChipZclStatus_t status(bool wasHandled)
{
    if (wasHandled)
    {
        return CHIP_ZCL_STATUS_SUCCESS;
    }
    else
    {
        return CHIP_ZCL_STATUS_UNSUP_GENERAL_COMMAND;
    }
}

// Main cluster specific command parsing controller.
ChipZclStatus_t chipZclGeneralCommandParse(ChipZclCommandContext_t * context)
{
    ChipZclStatus_t result = status(false);
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
        // Unrecognized cluster ID, error status will apply.
        break;
    }
    return result;
}