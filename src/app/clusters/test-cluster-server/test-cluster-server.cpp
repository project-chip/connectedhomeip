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
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/command-id.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::TestCluster;
using namespace chip::app::Clusters::TestCluster::Commands;
using namespace chip::app::Clusters::TestCluster::Attributes;

constexpr const char * kErrorStr = "Test Cluster: List Octet cluster (0x%02x) Error setting '%s' attribute: 0x%02x";

namespace {

class TestAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the Test Cluster cluster on all endpoints.
    TestAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), TestCluster::Id) {}

    CHIP_ERROR Read(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadListInt8uAttribute(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadListOctetStringAttribute(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadListStructOctetStringAttribute(AttributeValueEncoder & aEncoder);
};

TestAttrAccess gAttrAccess;

CHIP_ERROR TestAttrAccess::Read(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case ListInt8u::Id: {
        return ReadListInt8uAttribute(aEncoder);
    }
    case ListOctetString::Id: {
        return ReadListOctetStringAttribute(aEncoder);
    }
    case ListStructOctetString::Id: {
        return ReadListStructOctetStringAttribute(aEncoder);
    }
    default: {
        break;
    }
    }

    return CHIP_NO_ERROR;
}

#if !CHIP_CLUSTER_CONFIG_ENABLE_COMPLEX_ATTRIBUTE_READ
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
#endif // !CHIP_CLUSTER_CONFIG_ENABLE_COMPLEX_ATTRIBUTE_READ

CHIP_ERROR TestAttrAccess::ReadListInt8uAttribute(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const TagBoundEncoder & encoder) -> CHIP_ERROR {
        constexpr uint16_t attributeCount = 4;
        for (uint8_t index = 0; index < attributeCount; index++)
        {
            ReturnErrorOnFailure(encoder.Encode(index));
        }
        return CHIP_NO_ERROR;
    });
}

#if !CHIP_CLUSTER_CONFIG_ENABLE_COMPLEX_ATTRIBUTE_READ
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
#endif // !CHIP_CLUSTER_CONFIG_ENABLE_COMPLEX_ATTRIBUTE_READ

CHIP_ERROR TestAttrAccess::ReadListOctetStringAttribute(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const TagBoundEncoder & encoder) -> CHIP_ERROR {
        constexpr uint16_t attributeCount = 4;
        char data[6]                      = { 'T', 'e', 's', 't', 'N', '\0' };

        for (uint8_t index = 0; index < attributeCount; index++)
        {
            snprintf(data + strlen(data) - 1, 2, "%d", index);
            ByteSpan span(Uint8::from_char(data), strlen(data));
            ReturnErrorOnFailure(encoder.Encode(span));
        }
        return CHIP_NO_ERROR;
    });
}

#if !CHIP_CLUSTER_CONFIG_ENABLE_COMPLEX_ATTRIBUTE_READ
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
#endif // !CHIP_CLUSTER_CONFIG_ENABLE_COMPLEX_ATTRIBUTE_READ

CHIP_ERROR TestAttrAccess::ReadListStructOctetStringAttribute(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const TagBoundEncoder & encoder) -> CHIP_ERROR {
        constexpr uint16_t attributeCount = 4;
        char data[6]                      = { 'T', 'e', 's', 't', 'N', '\0' };

        for (uint8_t index = 0; index < attributeCount; index++)
        {
            snprintf(data + strlen(data) - 1, 2, "%d", index);
            ByteSpan span(Uint8::from_char(data), strlen(data));

            Structs::TestListStructOctet::Type structOctet;
            structOctet.fabricIndex     = index;
            structOctet.operationalCert = span;
            ReturnErrorOnFailure(encoder.Encode(structOctet));
        }
        return CHIP_NO_ERROR;
    });
}
} // namespace

bool emberAfTestClusterClusterTestCallback(app::CommandHandler *, const app::ConcreteCommandPath & commandPath,
                                           const Test::DecodableType & commandData)
{
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfTestClusterClusterTestSpecificCallback(CommandHandler * apCommandObj, const ConcreteCommandPath & commandPath,
                                                   const TestSpecific::DecodableType & commandData)
{
    TestSpecificResponse::Type responseData;
    responseData.returnValue = 7;
    CHIP_ERROR err           = apCommandObj->AddResponseData(commandPath, responseData);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Test Cluster: failed to send TestSpecific response: %" CHIP_ERROR_FORMAT, err.Format());
    }
    return true;
}

bool emberAfTestClusterClusterTestNotHandledCallback(CommandHandler *, const ConcreteCommandPath & commandPath,
                                                     const TestNotHandled::DecodableType & commandData)
{
    return false;
}

bool emberAfTestClusterClusterTestAddArgumentsCallback(CommandHandler * apCommandObj, const ConcreteCommandPath & commandPath,
                                                       const TestAddArguments::DecodableType & commandData)
{
    if (commandData.arg1 > UINT8_MAX - commandData.arg2)
    {
        return emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_ARGUMENT);
    }

    TestAddArgumentsResponse::Type responseData;
    responseData.returnValue = static_cast<uint8_t>(commandData.arg1 + commandData.arg2);
    CHIP_ERROR err           = apCommandObj->AddResponseData(commandPath, responseData);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Test Cluster: failed to send TestAddArguments response: %" CHIP_ERROR_FORMAT, err.Format());
    }
    return true;
}

bool emberAfTestClusterClusterTestStructArgumentRequestCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::TestStructArgumentRequest::DecodableType & commandData)
{
    emberAfSendImmediateDefaultResponse(commandData.arg1.b ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE);
    return true;
}

bool emberAfTestClusterClusterTestNestedStructArgumentRequestCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::TestNestedStructArgumentRequest::DecodableType & commandData)
{
    emberAfSendImmediateDefaultResponse(commandData.arg1.c.b ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE);
    return true;
}

bool emberAfTestClusterClusterTestListStructArgumentRequestCallback(
    app::CommandHandler * commandObj, app::ConcreteCommandPath const & commandPath,
    Commands::TestListStructArgumentRequest::DecodableType const & commandData)
{
    bool shouldReturnTrue = true;

    auto structIterator = commandData.arg1.begin();
    while (structIterator.Next())
    {
        auto & structValue = structIterator.GetValue();
        shouldReturnTrue   = shouldReturnTrue && structValue.b;
    }

    if (CHIP_NO_ERROR != structIterator.GetStatus())
    {
        shouldReturnTrue = false;
    }

    emberAfSendImmediateDefaultResponse(shouldReturnTrue ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE);
    return true;
}

bool emberAfTestClusterClusterTestListInt8UArgumentRequestCallback(
    app::CommandHandler * commandObj, app::ConcreteCommandPath const & commandPath,
    Commands::TestListInt8UArgumentRequest::DecodableType const & commandData)
{
    bool shouldReturnTrue = true;

    auto uint8Iterator = commandData.arg1.begin();
    while (uint8Iterator.Next())
    {
        auto & value     = uint8Iterator.GetValue();
        shouldReturnTrue = shouldReturnTrue && (value != 0);
    }

    if (CHIP_NO_ERROR != uint8Iterator.GetStatus())
    {
        shouldReturnTrue = false;
    }

    emberAfSendImmediateDefaultResponse(shouldReturnTrue ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE);
    return true;
}

bool emberAfTestClusterClusterTestNestedStructListArgumentRequestCallback(
    app::CommandHandler * commandObj, app::ConcreteCommandPath const & commandPath,
    Commands::TestNestedStructListArgumentRequest::DecodableType const & commandData)
{
    bool shouldReturnTrue = commandData.arg1.c.b;

    auto structIterator = commandData.arg1.d.begin();
    while (structIterator.Next())
    {
        auto & structValue = structIterator.GetValue();
        shouldReturnTrue   = shouldReturnTrue && structValue.b;
    }

    if (CHIP_NO_ERROR != structIterator.GetStatus())
    {
        shouldReturnTrue = false;
    }

    emberAfSendImmediateDefaultResponse(shouldReturnTrue ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE);
    return true;
}

bool emberAfTestClusterClusterTestListNestedStructListArgumentRequestCallback(
    app::CommandHandler * commandObj, app::ConcreteCommandPath const & commandPath,
    Commands::TestListNestedStructListArgumentRequest::DecodableType const & commandData)
{
    bool shouldReturnTrue = true;

    auto structIterator = commandData.arg1.begin();
    while (structIterator.Next())
    {
        auto & structValue = structIterator.GetValue();
        shouldReturnTrue   = shouldReturnTrue && structValue.c.b;

        auto subStructIterator = structValue.d.begin();
        while (subStructIterator.Next())
        {
            auto & subStructValue = subStructIterator.GetValue();
            shouldReturnTrue      = shouldReturnTrue && subStructValue.b;
        }

        if (CHIP_NO_ERROR != subStructIterator.GetStatus())
        {
            shouldReturnTrue = false;
            break;
        }
    }

    if (CHIP_NO_ERROR != structIterator.GetStatus())
    {
        shouldReturnTrue = false;
    }

    emberAfSendImmediateDefaultResponse(shouldReturnTrue ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE);
    return true;
}

bool emberAfTestClusterClusterTestListInt8UReverseRequestCallback(
    CommandHandler * commandObj, ConcreteCommandPath const & commandPath,
    Commands::TestListInt8UReverseRequest::DecodableType const & commandData)
{
    size_t count = 0;
    {
        auto iter = commandData.arg1.begin();
        while (iter.Next())
        {
            ++count;
        }
        VerifyOrExit(iter.GetStatus() == CHIP_NO_ERROR, );
    }

    {
        auto iter = commandData.arg1.begin();
        Commands::TestListInt8UReverseResponse::Type responseData;
        size_t cur = count;
        Platform::ScopedMemoryBuffer<uint8_t> responseBuf;
        VerifyOrExit(responseBuf.Calloc(count), );
        while (iter.Next() && cur > 0)
        {
            responseBuf[cur - 1] = iter.GetValue();
            --cur;
        }
        VerifyOrExit(cur == 0, );
        VerifyOrExit(iter.GetStatus() == CHIP_NO_ERROR, );
        responseData.arg1 = DataModel::List<uint8_t>(responseBuf.Get(), count);
        SuccessOrExit(commandObj->AddResponseData(commandPath, responseData));
        return true;
    }

exit:
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    return true;
}

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterTestClusterPluginServerInitCallback(void)
{
#if CHIP_CLUSTER_CONFIG_ENABLE_COMPLEX_ATTRIBUTE_READ
    registerAttributeAccessOverride(&gAttrAccess);
#else  // CHIP_CLUSTER_CONFIG_ENABLE_COMPLEX_ATTRIBUTE_READ
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
        VerifyOrReturn(status == EMBER_ZCL_STATUS_SUCCESS,
                       ChipLogError(Zcl, kErrorStr, endpoint, "test list struct octet", status));
    }
#endif // CHIP_CLUSTER_CONFIG_ENABLE_COMPLEX_ATTRIBUTE_READ
}
