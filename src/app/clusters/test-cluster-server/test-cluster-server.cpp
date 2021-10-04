/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 * @brief Implementation for the Test Server Cluster
 ***************************************************************************/

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/CommandHandler.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app::Clusters::TestCluster;

constexpr const char * kErrorStr = "Test Cluster: List Octet cluster (0x%02x) Error setting '%s' attribute: 0x%02x";

namespace {
EmberAfStatus writeAttribute(EndpointId endpoint, AttributeId attributeId, uint8_t * buffer, int32_t index = -1)
{
    EmberAfAttributeSearchRecord record;
    record.endpoint         = endpoint;
    record.clusterId        = ZCL_TEST_CLUSTER_ID;
    record.clusterMask      = CLUSTER_MASK_SERVER;
    record.manufacturerCode = EMBER_AF_NULL_MANUFACTURER_CODE;
    record.attributeId      = attributeId;

    // When reading or writing a List attribute the 'index' value could have 3 types of values:
    //  -1: Read/Write the whole list content, including the number of elements in the list
    //   0: Read/Write the number of elements in the list, represented as a uint16_t
    //   n: Read/Write the nth element of the list
    //
    // Since the first 2 bytes of the attribute are used to store the number of elements, elements indexing starts
    // at 1. In order to hide this to the rest of the code of this file, the element index is incremented by 1 here.
    // This also allows calling writeAttribute() with no index arg to mean "write the length".
    return emAfReadOrWriteAttribute(&record, NULL, buffer, 0, true, index + 1);
}

EmberAfStatus writeTestListInt8uAttribute(EndpointId endpoint)
{
    EmberAfStatus status    = EMBER_ZCL_STATUS_SUCCESS;
    AttributeId attributeId = ZCL_LIST_ATTRIBUTE_ID;

    uint16_t attributeCount = 4;
    for (uint8_t index = 0; index < attributeCount; index++)
    {
        status = writeAttribute(endpoint, attributeId, (uint8_t *) &index, index);
        VerifyOrReturnError(status == EMBER_ZCL_STATUS_SUCCESS, status);
    }

    status = writeAttribute(endpoint, attributeId, (uint8_t *) &attributeCount);
    VerifyOrReturnError(status == EMBER_ZCL_STATUS_SUCCESS, status);
    return status;
}

EmberAfStatus writeTestListOctetAttribute(EndpointId endpoint)
{
    EmberAfStatus status    = EMBER_ZCL_STATUS_SUCCESS;
    AttributeId attributeId = ZCL_LIST_OCTET_STRING_ATTRIBUTE_ID;

    uint16_t attributeCount = 4;
    char data[6]            = { 'T', 'e', 's', 't', 'N', '\0' };
    ByteSpan span           = ByteSpan(Uint8::from_char(data), strlen(data));

    for (uint8_t index = 0; index < attributeCount; index++)
    {
        sprintf(data + strlen(data) - 1, "%d", index);

        status = writeAttribute(endpoint, attributeId, (uint8_t *) &span, index);
        VerifyOrReturnError(status == EMBER_ZCL_STATUS_SUCCESS, status);
    }

    status = writeAttribute(endpoint, attributeId, (uint8_t *) &attributeCount);
    VerifyOrReturnError(status == EMBER_ZCL_STATUS_SUCCESS, status);
    return status;
}

EmberAfStatus writeTestListStructOctetAttribute(EndpointId endpoint)
{
    EmberAfStatus status    = EMBER_ZCL_STATUS_SUCCESS;
    AttributeId attributeId = ZCL_LIST_STRUCT_OCTET_STRING_ATTRIBUTE_ID;

    uint16_t attributeCount = 4;
    char data[6]            = { 'T', 'e', 's', 't', 'N', '\0' };
    ByteSpan span           = ByteSpan(Uint8::from_char(data), strlen(data));

    for (uint8_t index = 0; index < attributeCount; index++)
    {
        sprintf(data + strlen(data) - 1, "%d", index);

        _TestListStructOctet structOctet;
        structOctet.fabricIndex     = index;
        structOctet.operationalCert = span;

        status = writeAttribute(endpoint, attributeId, (uint8_t *) &structOctet, index);
        VerifyOrReturnError(status == EMBER_ZCL_STATUS_SUCCESS, status);
    }

    status = writeAttribute(endpoint, attributeId, (uint8_t *) &attributeCount);
    VerifyOrReturnError(status == EMBER_ZCL_STATUS_SUCCESS, status);
    return status;
}
} // namespace

void emberAfPluginTestClusterServerInitCallback(void)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    for (uint8_t index = 0; index < emberAfEndpointCount(); index++)
    {
        EndpointId endpoint = emberAfEndpointFromIndex(index);
        if (!emberAfContainsCluster(endpoint, ZCL_TEST_CLUSTER_ID))
        {
            continue;
        }

        status = writeTestListInt8uAttribute(endpoint);
        VerifyOrReturn(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, kErrorStr, endpoint, "test list int8u", status));

        status = writeTestListOctetAttribute(endpoint);
        VerifyOrReturn(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, kErrorStr, endpoint, "test list octet", status));

        status = writeTestListStructOctetAttribute(endpoint);
        VerifyOrReturn(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, kErrorStr, endpoint, "test list strut octet", status));
    }
}

bool emberAfTestClusterClusterTestCallback(EndpointId endpoint, app::CommandHandler *)
{
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool sendNumericResponse(EndpointId endpoint, app::CommandHandler * apCommandObj, CommandId responseCommand, uint8_t returnValue)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    app::CommandPathParams cmdParams = { endpoint, /* group id */ 0, ZCL_TEST_CLUSTER_ID, responseCommand,
                                         (app::CommandPathFlags::kEndpointIdValid) };
    TLV::TLVWriter * writer          = nullptr;

    VerifyOrExit(apCommandObj != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    SuccessOrExit(err = apCommandObj->PrepareCommand(cmdParams));
    writer = apCommandObj->GetCommandDataElementTLVWriter();
    SuccessOrExit(err = writer->Put(TLV::ContextTag(0), returnValue));
    SuccessOrExit(err = apCommandObj->FinishCommand());

exit:
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Test Cluster: failed to send TestSpecific response: %" CHIP_ERROR_FORMAT, err.Format());
    }
    return true;
}

bool emberAfTestClusterClusterTestSpecificCallback(EndpointId endpoint, app::CommandHandler * apCommandObj)
{
    return sendNumericResponse(endpoint, apCommandObj, Commands::TestSpecificResponse::Id, 7);
}

bool emberAfTestClusterClusterTestNotHandledCallback(EndpointId endpoint, app::CommandHandler *)
{
    return false;
}

bool emberAfTestClusterClusterTestAddArgumentsCallback(EndpointId endpoint, app::CommandHandler * apCommandObj, uint8_t arg1,
                                                       uint8_t arg2)
{
    if (arg1 > UINT8_MAX - arg2)
    {
        return emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_ARGUMENT);
    }

    return sendNumericResponse(endpoint, apCommandObj, Commands::TestAddArgumentsResponse::Id, static_cast<uint8_t>(arg1 + arg2));
}
