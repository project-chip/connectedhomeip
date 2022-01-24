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
#include <app/EventLogging.h>
#include <app/server/Server.h>
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

// The number of elements in the test attribute list
constexpr uint8_t kAttributeListLength = 4;

// The maximum length of the test attribute list element in bytes
constexpr uint8_t kAttributeEntryLength = 6;

namespace {

class OctetStringData
{
public:
    uint8_t * Data() { return mDataBuf; }
    const uint8_t * Data() const { return mDataBuf; }
    size_t Length() const { return mDataLen; }
    void SetLength(size_t size) { mDataLen = size; }

    ByteSpan AsSpan() const { return ByteSpan(Data(), Length()); }

private:
    uint8_t mDataBuf[kAttributeEntryLength];
    size_t mDataLen = 0;
};

class TestAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the Test Cluster cluster on all endpoints.
    TestAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), TestCluster::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

private:
    CHIP_ERROR ReadListInt8uAttribute(AttributeValueEncoder & aEncoder);
    CHIP_ERROR WriteListInt8uAttribute(AttributeValueDecoder & aDecoder);
    CHIP_ERROR ReadListOctetStringAttribute(AttributeValueEncoder & aEncoder);
    CHIP_ERROR WriteListOctetStringAttribute(AttributeValueDecoder & aDecoder);
    CHIP_ERROR ReadListLongOctetStringAttribute(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadListStructOctetStringAttribute(AttributeValueEncoder & aEncoder);
    CHIP_ERROR WriteListStructOctetStringAttribute(AttributeValueDecoder & aDecoder);
    CHIP_ERROR ReadListNullablesAndOptionalsStructAttribute(AttributeValueEncoder & aEncoder);
    CHIP_ERROR WriteListNullablesAndOptionalsStructAttribute(AttributeValueDecoder & aDecoder);
    CHIP_ERROR ReadStructAttribute(AttributeValueEncoder & aEncoder);
    CHIP_ERROR WriteStructAttribute(AttributeValueDecoder & aDecoder);
    CHIP_ERROR ReadNullableStruct(AttributeValueEncoder & aEncoder);
    CHIP_ERROR WriteNullableStruct(AttributeValueDecoder & aDecoder);
    CHIP_ERROR ReadListFabricScopedAttribute(AttributeValueEncoder & aEncoder);
};

TestAttrAccess gAttrAccess;
uint8_t gListUint8Data[kAttributeListLength];
OctetStringData gListOctetStringData[kAttributeListLength];
OctetStringData gListOperationalCert[kAttributeListLength];
Structs::TestListStructOctet::Type listStructOctetStringData[kAttributeListLength];
OctetStringData gStructAttributeByteSpanData;
Structs::SimpleStruct::Type gStructAttributeValue;
NullableStruct::TypeInfo::Type gNullableStructAttributeValue;

// We don't actually support any interesting bits in the struct for now, except
// for a non-null nullableList member.
SimpleEnum gSimpleEnums[kAttributeListLength];
size_t gSimpleEnumCount = 0;
Structs::NullablesAndOptionalsStruct::Type gNullablesAndOptionalsStruct;

CHIP_ERROR TestAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
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
    case StructAttr::Id: {
        return ReadStructAttribute(aEncoder);
    }
    case ListLongOctetString::Id: {
        return ReadListLongOctetStringAttribute(aEncoder);
    }
    case ListFabricScoped::Id: {
        return ReadListFabricScopedAttribute(aEncoder);
    }
    case NullableStruct::Id: {
        return ReadNullableStruct(aEncoder);
    }
    default: {
        break;
    }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TestAttrAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    switch (aPath.mAttributeId)
    {
    case ListInt8u::Id: {
        return WriteListInt8uAttribute(aDecoder);
    }
    case ListOctetString::Id: {
        return WriteListOctetStringAttribute(aDecoder);
    }
    case ListStructOctetString::Id: {
        return WriteListStructOctetStringAttribute(aDecoder);
    }
    case ListNullablesAndOptionalsStruct::Id: {
        return WriteListNullablesAndOptionalsStructAttribute(aDecoder);
    }
    case StructAttr::Id: {
        return WriteStructAttribute(aDecoder);
    }
    case NullableStruct::Id: {
        return WriteNullableStruct(aDecoder);
    }
    default: {
        break;
    }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TestAttrAccess::ReadNullableStruct(AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(gNullableStructAttributeValue);
}

CHIP_ERROR TestAttrAccess::WriteNullableStruct(AttributeValueDecoder & aDecoder)
{
    return aDecoder.Decode(gNullableStructAttributeValue);
}

CHIP_ERROR TestAttrAccess::ReadListInt8uAttribute(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        for (uint8_t index = 0; index < kAttributeListLength; index++)
        {
            ReturnErrorOnFailure(encoder.Encode(gListUint8Data[index]));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR TestAttrAccess::WriteListInt8uAttribute(AttributeValueDecoder & aDecoder)
{
    ListInt8u::TypeInfo::DecodableType list;

    ReturnErrorOnFailure(aDecoder.Decode(list));

    size_t size;
    ReturnErrorOnFailure(list.ComputeSize(&size));

    // We never change our length, so fail out attempts to change it.  This
    // should really return one of the spec errors!
    VerifyOrReturnError(size == kAttributeListLength, CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t index = 0;
    auto iter     = list.begin();
    while (iter.Next())
    {
        auto & entry = iter.GetValue();

        VerifyOrReturnError(index < kAttributeListLength, CHIP_ERROR_BUFFER_TOO_SMALL);
        gListUint8Data[index++] = entry;
    }

    return iter.GetStatus();
}

CHIP_ERROR TestAttrAccess::ReadListOctetStringAttribute(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        for (uint8_t index = 0; index < kAttributeListLength; index++)
        {
            ReturnErrorOnFailure(encoder.Encode(gListOctetStringData[index].AsSpan()));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR TestAttrAccess::WriteListOctetStringAttribute(AttributeValueDecoder & aDecoder)
{
    ListOctetString::TypeInfo::DecodableType list;

    ReturnErrorOnFailure(aDecoder.Decode(list));

    uint8_t index = 0;
    auto iter     = list.begin();
    while (iter.Next())
    {
        const auto & entry = iter.GetValue();

        VerifyOrReturnError(index < kAttributeListLength, CHIP_ERROR_BUFFER_TOO_SMALL);
        VerifyOrReturnError(entry.size() <= kAttributeEntryLength, CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(gListOctetStringData[index].Data(), entry.data(), entry.size());
        gListOctetStringData[index].SetLength(entry.size());
        index++;
    }

    return iter.GetStatus();
}

CHIP_ERROR TestAttrAccess::ReadListLongOctetStringAttribute(AttributeValueEncoder & aEncoder)
{
    // The ListOctetStringAttribute takes 512 bytes, and the whole attribute will exceed the IPv6 MTU, so we can test list chunking
    // feature with this attribute.
    char buf[513] = "0123456789abcdef"
                    "0123456789abcdef"
                    "0123456789abcdef"
                    "0123456789abcdef"
                    "0123456789abcdef" // 5
                    "0123456789abcdef"
                    "0123456789abcdef"
                    "0123456789abcdef"
                    "0123456789abcdef"
                    "0123456789abcdef" // 10
                    "0123456789abcdef"
                    "0123456789abcdef"
                    "0123456789abcdef"
                    "0123456789abcdef"
                    "0123456789abcdef" // 15
                    "0123456789abcdef"
                    "0123456789abcdef"
                    "0123456789abcdef"
                    "0123456789abcdef"
                    "0123456789abcdef" // 20
                    "0123456789abcdef"
                    "0123456789abcdef"
                    "0123456789abcdef"
                    "0123456789abcdef"
                    "0123456789abcdef" // 25
                    "0123456789abcdef"
                    "0123456789abcdef"
                    "0123456789abcdef"
                    "0123456789abcdef"
                    "0123456789abcdef" // 30
                    "0123456789abcdef"
                    "0123456789abcdef"; // 32 * 16 = 512
    return aEncoder.EncodeList([buf](const auto & encoder) -> CHIP_ERROR {
        for (uint8_t index = 0; index < kAttributeListLength; index++)
        {
            ReturnErrorOnFailure(encoder.Encode(ByteSpan(chip::Uint8::from_const_char(buf), 512)));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR TestAttrAccess::ReadListStructOctetStringAttribute(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        for (uint8_t index = 0; index < kAttributeListLength; index++)
        {
            Structs::TestListStructOctet::Type structOctet;
            structOctet.fabricIndex     = listStructOctetStringData[index].fabricIndex;
            structOctet.operationalCert = listStructOctetStringData[index].operationalCert;
            ReturnErrorOnFailure(encoder.Encode(structOctet));
        }

        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR TestAttrAccess::WriteListStructOctetStringAttribute(AttributeValueDecoder & aDecoder)
{
    ListStructOctetString::TypeInfo::DecodableType list;

    ReturnErrorOnFailure(aDecoder.Decode(list));

    uint8_t index = 0;
    auto iter     = list.begin();
    while (iter.Next())
    {
        const auto & entry = iter.GetValue();

        VerifyOrReturnError(index < kAttributeListLength, CHIP_ERROR_BUFFER_TOO_SMALL);
        VerifyOrReturnError(entry.operationalCert.size() <= kAttributeEntryLength, CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(gListOperationalCert[index].Data(), entry.operationalCert.data(), entry.operationalCert.size());
        gListOperationalCert[index].SetLength(entry.operationalCert.size());

        listStructOctetStringData[index].fabricIndex     = entry.fabricIndex;
        listStructOctetStringData[index].operationalCert = gListOperationalCert[index].AsSpan();
        index++;
    }

    if (iter.GetStatus() != CHIP_NO_ERROR)
    {
        return CHIP_ERROR_INVALID_DATA_LIST;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TestAttrAccess::ReadListNullablesAndOptionalsStructAttribute(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        // Just encode our one struct for now.
        ReturnErrorOnFailure(encoder.Encode(gNullablesAndOptionalsStruct));
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR TestAttrAccess::WriteListNullablesAndOptionalsStructAttribute(AttributeValueDecoder & aDecoder)
{
    DataModel::DecodableList<Structs::NullablesAndOptionalsStruct::DecodableType> list;
    ReturnErrorOnFailure(aDecoder.Decode(list));

    size_t count;
    ReturnErrorOnFailure(list.ComputeSize(&count));
    // This should really send proper errors on invalid input!
    VerifyOrReturnError(count == 1, CHIP_ERROR_INVALID_ARGUMENT);

    auto iter = list.begin();
    while (iter.Next())
    {
        auto & value = iter.GetValue();
        // We only support some values so far.
        VerifyOrReturnError(value.nullableString.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(value.nullableStruct.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);

        VerifyOrReturnError(!value.optionalString.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(!value.nullableOptionalString.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(!value.optionalStruct.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(!value.nullableOptionalStruct.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(!value.optionalList.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(!value.nullableOptionalList.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);

        // Start our value off as null, just in case we fail to decode things.
        gNullablesAndOptionalsStruct.nullableList.SetNull();
        if (!value.nullableList.IsNull())
        {
            ReturnErrorOnFailure(value.nullableList.Value().ComputeSize(&count));
            VerifyOrReturnError(count <= ArraySize(gSimpleEnums), CHIP_ERROR_INVALID_ARGUMENT);
            auto iter2       = value.nullableList.Value().begin();
            gSimpleEnumCount = 0;
            while (iter2.Next())
            {
                gSimpleEnums[gSimpleEnumCount] = iter2.GetValue();
                ++gSimpleEnumCount;
            }
            ReturnErrorOnFailure(iter2.GetStatus());
            gNullablesAndOptionalsStruct.nullableList.SetNonNull(Span<SimpleEnum>(gSimpleEnums, gSimpleEnumCount));
        }
        gNullablesAndOptionalsStruct.nullableInt         = value.nullableInt;
        gNullablesAndOptionalsStruct.optionalInt         = value.optionalInt;
        gNullablesAndOptionalsStruct.nullableOptionalInt = value.nullableOptionalInt;
    }

    ReturnErrorOnFailure(iter.GetStatus());
    return CHIP_NO_ERROR;
}

CHIP_ERROR TestAttrAccess::ReadStructAttribute(AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(gStructAttributeValue);
}

CHIP_ERROR TestAttrAccess::WriteStructAttribute(AttributeValueDecoder & aDecoder)
{
    // We don't support a nonempty charspan here for now.
    Structs::SimpleStruct::DecodableType temp;
    ReturnErrorOnFailure(aDecoder.Decode(temp));

    VerifyOrReturnError(temp.e.empty(), CHIP_ERROR_BUFFER_TOO_SMALL);
    const size_t octet_size = temp.d.size();
    VerifyOrReturnError(octet_size <= kAttributeEntryLength, CHIP_ERROR_BUFFER_TOO_SMALL);

    gStructAttributeValue = temp;
    memcpy(gStructAttributeByteSpanData.Data(), temp.d.data(), octet_size);
    gStructAttributeByteSpanData.SetLength(octet_size);
    gStructAttributeValue.d = gStructAttributeByteSpanData.AsSpan();
    return CHIP_NO_ERROR;
}

CHIP_ERROR TestAttrAccess::ReadListFabricScopedAttribute(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        chip::app::Clusters::TestCluster::Structs::TestFabricScoped::Type val;

        for (const auto & fb : Server::GetInstance().GetFabricTable())
        {
            val.fabricIndex = fb.GetFabricIndex();
            ReturnErrorOnFailure(encoder.Encode(val));
        }

        // Always append a fake fabric index so we can test fabric filter even when there is only one fabric provisioned.
        val.fabricIndex = kUndefinedFabricIndex;
        ReturnErrorOnFailure(encoder.Encode(val));
        return CHIP_NO_ERROR;
    });
}

} // namespace

bool emberAfTestClusterClusterTestCallback(app::CommandHandler *, const app::ConcreteCommandPath & commandPath,
                                           const Test::DecodableType & commandData)
{
    // Setup the test variables
    emAfLoadAttributeDefaults(commandPath.mEndpointId, true, MakeOptional(commandPath.mClusterId));
    for (int i = 0; i < kAttributeListLength; ++i)
    {
        gListUint8Data[i] = 0;
        gListOctetStringData[i].SetLength(0);
        gListOperationalCert[i].SetLength(0);
        listStructOctetStringData[i].fabricIndex     = 0;
        listStructOctetStringData[i].operationalCert = ByteSpan();
        gSimpleEnums[i]                              = SimpleEnum::kUnspecified;
    }
    gSimpleEnumCount = 0;

    gStructAttributeValue = Structs::SimpleStruct::Type();

    gNullableStructAttributeValue.SetNull();

    gNullablesAndOptionalsStruct = Structs::NullablesAndOptionalsStruct::Type();

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
        return emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_COMMAND);
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
bool emberAfTestClusterClusterTestEmitTestEventRequestCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const Commands::TestEmitTestEventRequest::DecodableType & commandData)
{
    Commands::TestEmitTestEventResponse::Type responseData;
    Structs::SimpleStruct::Type arg4;
    DataModel::List<const Structs::SimpleStruct::Type> arg5;
    DataModel::List<const SimpleEnum> arg6;

    // TODO:  Add code to pull arg4, arg5 and arg6 from the arguments of the command
    Events::TestEvent::Type event{ commandData.arg1, commandData.arg2, commandData.arg3, arg4, arg5, arg6 };

    if (CHIP_NO_ERROR != LogEvent(event, commandPath.mEndpointId, responseData.value))
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
        return true;
    }
    commandObj->AddResponseData(commandPath, responseData);
    return true;
}

bool emberAfTestClusterClusterTestListInt8UArgumentRequestCallback(
    CommandHandler * commandObj, ConcreteCommandPath const & commandPath,
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
        if (count == 0)
        {
            SuccessOrExit(commandObj->AddResponseData(commandPath, responseData));
            return true;
        }
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

bool emberAfTestClusterClusterSimpleStructEchoRequestCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                              const Commands::SimpleStructEchoRequest::DecodableType & commandData)
{
    Commands::SimpleStructResponse::Type response;
    response.arg1.a = commandData.arg1.a;
    response.arg1.b = commandData.arg1.b;
    response.arg1.c = commandData.arg1.c;
    response.arg1.d = commandData.arg1.d;
    response.arg1.e = commandData.arg1.e;
    response.arg1.f = commandData.arg1.f;
    response.arg1.g = commandData.arg1.g;
    response.arg1.h = commandData.arg1.h;

    CHIP_ERROR err = commandObj->AddResponseData(commandPath, response);
    if (err != CHIP_NO_ERROR)
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }
    return true;
}

bool emberAfTestClusterClusterTimedInvokeRequestCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                         const Commands::TimedInvokeRequest::DecodableType & commandData)
{
    commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    return true;
}

bool emberAfTestClusterClusterTestSimpleOptionalArgumentRequestCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const Commands::TestSimpleOptionalArgumentRequest::DecodableType & commandData)
{
    Protocols::InteractionModel::Status status = commandData.arg1.HasValue() ? Protocols::InteractionModel::Status::Success
                                                                             : Protocols::InteractionModel::Status::InvalidValue;
    commandObj->AddStatus(commandPath, status);
    return true;
}

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterTestClusterPluginServerInitCallback(void)
{
    registerAttributeAccessOverride(&gAttrAccess);
}
