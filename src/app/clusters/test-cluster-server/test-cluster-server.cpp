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

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
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
    CHIP_ERROR ReadListNullablesAndOptionalsStructAttribute(AttributeValueEncoder & aEncoder);
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
    case ListNullablesAndOptionalsStruct::Id: {
        return ReadListNullablesAndOptionalsStructAttribute(aEncoder);
    }
    default: {
        break;
    }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TestAttrAccess::ReadListInt8uAttribute(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const TagBoundEncoder & encoder) -> CHIP_ERROR {
        constexpr uint8_t maxValue = 4;
        for (uint8_t value = 1; value <= maxValue; value++)
        {
            ReturnErrorOnFailure(encoder.Encode(value));
        }
        return CHIP_NO_ERROR;
    });
}

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

CHIP_ERROR TestAttrAccess::ReadListNullablesAndOptionalsStructAttribute(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const TagBoundEncoder & encoder) -> CHIP_ERROR {
        // Just encode a single default-initialized
        // entry for now.
        Structs::NullablesAndOptionalsStruct::Type entry;
        ReturnErrorOnFailure(encoder.Encode(entry));
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

static bool SendBooleanResponse(CommandHandler * commandObj, const ConcreteCommandPath & commandPath, bool value)
{
    Commands::BooleanResponse::Type response;
    response.value = value;
    CHIP_ERROR err = commandObj->AddResponseData(commandPath, response);
    if (err != CHIP_NO_ERROR)
    {
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Failure);
    }
    return true;
}

bool emberAfTestClusterClusterTestStructArgumentRequestCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::TestStructArgumentRequest::DecodableType & commandData)
{
    return SendBooleanResponse(commandObj, commandPath, commandData.arg1.b);
}

bool emberAfTestClusterClusterTestNestedStructArgumentRequestCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::TestNestedStructArgumentRequest::DecodableType & commandData)
{
    return SendBooleanResponse(commandObj, commandPath, commandData.arg1.c.b);
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
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return true;
    }

    return SendBooleanResponse(commandObj, commandPath, shouldReturnTrue);
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
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return true;
    }

    return SendBooleanResponse(commandObj, commandPath, shouldReturnTrue);
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
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return true;
    }

    return SendBooleanResponse(commandObj, commandPath, shouldReturnTrue);
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
            emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
            return true;
        }
    }

    if (CHIP_NO_ERROR != structIterator.GetStatus())
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return true;
    }

    return SendBooleanResponse(commandObj, commandPath, shouldReturnTrue);
}

bool emberAfTestClusterClusterTestListInt8UReverseRequestCallback(
    CommandHandler * commandObj, ConcreteCommandPath const & commandPath,
    Commands::TestListInt8UReverseRequest::DecodableType const & commandData)
{
    size_t count   = 0;
    CHIP_ERROR err = commandData.arg1.ComputeSize(&count);
    VerifyOrExit(err == CHIP_NO_ERROR, );

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

bool emberAfTestClusterClusterTestEnumsRequestCallback(CommandHandler * commandObj, ConcreteCommandPath const & commandPath,
                                                       TestEnumsRequest::DecodableType const & commandData)
{
    TestEnumsResponse::Type response;
    response.arg1 = commandData.arg1;
    response.arg2 = commandData.arg2;

    CHIP_ERROR err = commandObj->AddResponseData(commandPath, response);
    if (err != CHIP_NO_ERROR)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }
    return true;
}

bool emberAfTestClusterClusterTestNullableOptionalRequestCallback(
    CommandHandler * commandObj, ConcreteCommandPath const & commandPath,
    Commands::TestNullableOptionalRequest::DecodableType const & commandData)
{
    Commands::TestNullableOptionalResponse::Type response;
    response.wasPresent = commandData.arg1.HasValue();
    if (response.wasPresent)
    {
        bool wasNull = commandData.arg1.Value().IsNull();
        response.wasNull.SetValue(wasNull);
        if (!wasNull)
        {
            response.value.SetValue(commandData.arg1.Value().Value());
        }

        response.originalValue.Emplace(commandData.arg1.Value());
    }

    CHIP_ERROR err = commandObj->AddResponseData(commandPath, response);
    if (err != CHIP_NO_ERROR)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }
    return true;
}

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterTestClusterPluginServerInitCallback(void)
{
    registerAttributeAccessOverride(&gAttrAccess);
}
