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
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventLogging.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::UnitTesting;
using namespace chip::app::Clusters::UnitTesting::Commands;
using namespace chip::app::Clusters::UnitTesting::Attributes;
using chip::Protocols::InteractionModel::Status;

// The number of elements in the test attribute list
constexpr uint8_t kAttributeListLength = 4;

// The maximum length of the test attribute list element in bytes
constexpr uint8_t kAttributeEntryLength = 6;

// The maximum length of the fabric sensitive string within the TestFabricScoped struct.
constexpr uint8_t kFabricSensitiveCharLength = 128;

// The maximum length of the fabric sensitive integer list within the TestFabricScoped struct.
constexpr uint8_t kFabricSensitiveIntListLength = 8;

// The maximum buffer size allowed in TestBatchHelperResponse
constexpr uint16_t kTestBatchHelperResponseBufferMax = 800;

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
    TestAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), Clusters::UnitTesting::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

private:
    CHIP_ERROR WriteListFabricScopedListEntry(const Structs::TestFabricScoped::DecodableType & entry, size_t index);

    CHIP_ERROR ReadListInt8uAttribute(AttributeValueEncoder & aEncoder);
    CHIP_ERROR WriteListInt8uAttribute(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder);
    CHIP_ERROR ReadListOctetStringAttribute(AttributeValueEncoder & aEncoder);
    CHIP_ERROR WriteListOctetStringAttribute(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder);
    CHIP_ERROR ReadListLongOctetStringAttribute(AttributeValueEncoder & aEncoder);
    CHIP_ERROR WriteListLongOctetStringAttribute(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder);
    CHIP_ERROR ReadListStructOctetStringAttribute(AttributeValueEncoder & aEncoder);
    CHIP_ERROR WriteListStructOctetStringAttribute(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder);
    CHIP_ERROR ReadListNullablesAndOptionalsStructAttribute(AttributeValueEncoder & aEncoder);
    CHIP_ERROR WriteListNullablesAndOptionalsStructAttribute(const ConcreteDataAttributePath & aPath,
                                                             AttributeValueDecoder & aDecoder);
    CHIP_ERROR ReadStructAttribute(AttributeValueEncoder & aEncoder);
    CHIP_ERROR WriteStructAttribute(AttributeValueDecoder & aDecoder);
    CHIP_ERROR ReadGlobalStruct(AttributeValueEncoder & aEncoder);
    CHIP_ERROR WriteGlobalStruct(AttributeValueDecoder & aDecoder);
    CHIP_ERROR ReadNullableStruct(AttributeValueEncoder & aEncoder);
    CHIP_ERROR WriteNullableStruct(AttributeValueDecoder & aDecoder);
    CHIP_ERROR ReadNullableGlobalStruct(AttributeValueEncoder & aEncoder);
    CHIP_ERROR WriteNullableGlobalStruct(AttributeValueDecoder & aDecoder);
    CHIP_ERROR ReadListFabricScopedAttribute(AttributeValueEncoder & aEncoder);
    CHIP_ERROR WriteListFabricScopedAttribute(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder);
};

struct AsyncBatchCommandsWorkData
{
    CommandHandler::Handle asyncCommandHandle;
    ConcreteCommandPath commandPath = ConcreteCommandPath(0, 0, 0);
    uint16_t sizeOfResponseBuffer;
    uint8_t fillCharacter;
};

TestAttrAccess gAttrAccess;
uint8_t gListUint8Data[kAttributeListLength];
size_t gListUint8DataLen = kAttributeListLength;
OctetStringData gListOctetStringData[kAttributeListLength];
size_t gListOctetStringDataLen = kAttributeListLength;
OctetStringData gListOperationalCert[kAttributeListLength];
size_t gListOperationalCertLen = kAttributeListLength;
size_t gListLongOctetStringLen = kAttributeListLength;
Structs::TestListStructOctet::Type listStructOctetStringData[kAttributeListLength];
OctetStringData gStructAttributeByteSpanData;
Structs::SimpleStruct::Type gStructAttributeValue;
Globals::Structs::TestGlobalStruct::Type gGlobalStructAttributeValue;
NullableStruct::TypeInfo::Type gNullableStructAttributeValue;
DataModel::Nullable<Globals::Structs::TestGlobalStruct::Type> gNullableGlobalStructAttributeValue;

chip::app::Clusters::UnitTesting::Structs::TestFabricScoped::Type gListFabricScopedAttributeValue[kAttributeListLength];
uint8_t gListFabricScoped_fabricSensitiveInt8uList[kAttributeListLength][kFabricSensitiveIntListLength];
size_t gListFabricScopedAttributeLen = 0;
char gListFabricScoped_fabricSensitiveCharBuf[kAttributeListLength][kFabricSensitiveCharLength];

//                                                     /16             /32             /48             /64
const char sLongOctetStringBuf[513] = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"  // 64
                                      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"  // 128
                                      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"  // 192
                                      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"  // 256
                                      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"  // 320
                                      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"  // 384
                                      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"  // 448
                                      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"; // 512

// We don't actually support any interesting bits in the struct for now, except
// for a non-null nullableList member.
SimpleEnum gSimpleEnums[kAttributeListLength];
size_t gSimpleEnumCount = 0;
Structs::NullablesAndOptionalsStruct::Type gNullablesAndOptionalsStruct;

void AsyncBatchCommandWork(AsyncBatchCommandsWorkData * asyncWorkData)
{
    auto commandHandleRef = std::move(asyncWorkData->asyncCommandHandle);
    auto commandHandle    = commandHandleRef.Get();
    if (commandHandle == nullptr)
    {
        // Very weird that we are even in here, what set this to nullptr?
        Platform::Delete(asyncWorkData);
        return;
    }

    uint8_t buffer[kTestBatchHelperResponseBufferMax];
    memset(buffer, asyncWorkData->fillCharacter, asyncWorkData->sizeOfResponseBuffer);
    Commands::TestBatchHelperResponse::Type response;
    response.buffer = ByteSpan(buffer, asyncWorkData->sizeOfResponseBuffer);
    commandHandle->AddResponse(asyncWorkData->commandPath, response);
    Platform::Delete(asyncWorkData);
}

static void timerCallback(System::Layer *, void * callbackContext)
{
    AsyncBatchCommandWork(reinterpret_cast<AsyncBatchCommandsWorkData *>(callbackContext));
}

static void scheduleTimerCallbackMs(AsyncBatchCommandsWorkData * asyncWorkData, uint32_t delayMs)
{
    DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(delayMs), timerCallback,
                                          reinterpret_cast<void *>(asyncWorkData));
}

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
    case GlobalStruct::Id: {
        return ReadGlobalStruct(aEncoder);
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
    case NullableGlobalStruct::Id: {
        return ReadNullableGlobalStruct(aEncoder);
    }
    case GeneralErrorBoolean::Id: {
        return StatusIB(Protocols::InteractionModel::Status::InvalidDataType).ToChipError();
    }
    case ClusterErrorBoolean::Id: {
        return StatusIB(Protocols::InteractionModel::Status::Failure, 17).ToChipError();
    }
    case WriteOnlyInt8u::Id: {
        return StatusIB(Protocols::InteractionModel::Status::UnsupportedRead).ToChipError();
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
        return WriteListInt8uAttribute(aPath, aDecoder);
    }
    case ListOctetString::Id: {
        return WriteListOctetStringAttribute(aPath, aDecoder);
    }
    case ListLongOctetString::Id: {
        return WriteListLongOctetStringAttribute(aPath, aDecoder);
    }
    case ListFabricScoped::Id: {
        return WriteListFabricScopedAttribute(aPath, aDecoder);
    }
    case ListStructOctetString::Id: {
        return WriteListStructOctetStringAttribute(aPath, aDecoder);
    }
    case ListNullablesAndOptionalsStruct::Id: {
        return WriteListNullablesAndOptionalsStructAttribute(aPath, aDecoder);
    }
    case StructAttr::Id: {
        return WriteStructAttribute(aDecoder);
    }
    case GlobalStruct::Id: {
        return WriteGlobalStruct(aDecoder);
    }
    case NullableStruct::Id: {
        return WriteNullableStruct(aDecoder);
    }
    case NullableGlobalStruct::Id: {
        return WriteNullableGlobalStruct(aDecoder);
    }
    case GeneralErrorBoolean::Id: {
        return StatusIB(Protocols::InteractionModel::Status::InvalidDataType).ToChipError();
    }
    case ClusterErrorBoolean::Id: {
        return StatusIB(Protocols::InteractionModel::Status::Failure, 17).ToChipError();
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

CHIP_ERROR TestAttrAccess::ReadNullableGlobalStruct(AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(gNullableGlobalStructAttributeValue);
}

CHIP_ERROR TestAttrAccess::WriteNullableGlobalStruct(AttributeValueDecoder & aDecoder)
{
    Attributes::NullableGlobalStruct::TypeInfo::DecodableType temp;
    ReturnErrorOnFailure(aDecoder.Decode(temp));

    if (!temp.IsNull())
    {
        // We don't support a nonempty charspan here for now.
        VerifyOrReturnError(temp.Value().name.empty(), CHIP_ERROR_BUFFER_TOO_SMALL);
    }

    gNullableGlobalStructAttributeValue = temp;
    return CHIP_NO_ERROR;
}

CHIP_ERROR TestAttrAccess::ReadListInt8uAttribute(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        for (size_t index = 0; index < gListUint8DataLen; index++)
        {
            ReturnErrorOnFailure(encoder.Encode(gListUint8Data[index]));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR TestAttrAccess::WriteListInt8uAttribute(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    if (!aPath.IsListItemOperation())
    {

        ListInt8u::TypeInfo::DecodableType list;

        ReturnErrorOnFailure(aDecoder.Decode(list));

        size_t size;
        ReturnErrorOnFailure(list.ComputeSize(&size));

        uint8_t index = 0;
        auto iter     = list.begin();
        while (iter.Next())
        {
            auto & entry = iter.GetValue();

            VerifyOrReturnError(index < kAttributeListLength, CHIP_ERROR_BUFFER_TOO_SMALL);
            gListUint8Data[index++] = entry;
        }

        gListUint8DataLen = size;

        return iter.GetStatus();
    }
    if (aPath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        VerifyOrReturnError(gListUint8DataLen < kAttributeListLength, CHIP_ERROR_INVALID_ARGUMENT);
        ReturnErrorOnFailure(aDecoder.Decode(gListUint8Data[gListUint8DataLen]));
        gListUint8DataLen++;
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR TestAttrAccess::ReadListOctetStringAttribute(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        for (size_t index = 0; index < gListOctetStringDataLen; index++)
        {
            ReturnErrorOnFailure(encoder.Encode(gListOctetStringData[index].AsSpan()));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR TestAttrAccess::WriteListOctetStringAttribute(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    if (!aPath.IsListItemOperation())
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

        gListOctetStringDataLen = index;

        return iter.GetStatus();
    }
    if (aPath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        ByteSpan entry;
        ReturnErrorOnFailure(aDecoder.Decode(entry));

        VerifyOrReturnError(gListOctetStringDataLen < kAttributeListLength, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(entry.size() <= kAttributeEntryLength, CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(gListOctetStringData[gListOctetStringDataLen].Data(), entry.data(), entry.size());
        gListOctetStringData[gListOctetStringDataLen].SetLength(entry.size());
        gListOctetStringDataLen++;
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR TestAttrAccess::ReadListLongOctetStringAttribute(AttributeValueEncoder & aEncoder)
{
    // The ListOctetStringAttribute takes 512 bytes, and the whole attribute will exceed the IPv6 MTU, so we can test list chunking
    // feature with this attribute.
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        for (size_t index = 0; index < gListLongOctetStringLen; index++)
        {
            ReturnErrorOnFailure(encoder.Encode(ByteSpan(chip::Uint8::from_const_char(sLongOctetStringBuf), 512)));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR TestAttrAccess::WriteListLongOctetStringAttribute(const ConcreteDataAttributePath & aPath,
                                                             AttributeValueDecoder & aDecoder)
{
    if (!aPath.IsListItemOperation())
    {
        ListLongOctetString::TypeInfo::DecodableType list;

        ReturnErrorOnFailure(aDecoder.Decode(list));

        auto iter               = list.begin();
        gListLongOctetStringLen = 0;
        while (iter.Next())
        {
            const auto & entry = iter.GetValue();
            VerifyOrReturnError(entry.size() == sizeof(sLongOctetStringBuf) - 1, CHIP_ERROR_BUFFER_TOO_SMALL);
            VerifyOrReturnError(memcmp(entry.data(), sLongOctetStringBuf, entry.size()) == 0, CHIP_ERROR_INVALID_ARGUMENT);
            gListLongOctetStringLen++;
        }

        return iter.GetStatus();
    }
    if (aPath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        ByteSpan entry;
        ReturnErrorOnFailure(aDecoder.Decode(entry));

        VerifyOrReturnError(entry.size() == sizeof(sLongOctetStringBuf) - 1, CHIP_ERROR_BUFFER_TOO_SMALL);
        VerifyOrReturnError(memcmp(entry.data(), sLongOctetStringBuf, entry.size()) == 0, CHIP_ERROR_INVALID_ARGUMENT);
        gListLongOctetStringLen++;
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR TestAttrAccess::ReadListStructOctetStringAttribute(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        for (size_t index = 0; index < gListOperationalCertLen; index++)
        {
            Structs::TestListStructOctet::Type structOctet;
            structOctet.member1 = listStructOctetStringData[index].member1;
            structOctet.member2 = listStructOctetStringData[index].member2;
            ReturnErrorOnFailure(encoder.Encode(structOctet));
        }

        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR TestAttrAccess::WriteListStructOctetStringAttribute(const ConcreteDataAttributePath & aPath,
                                                               AttributeValueDecoder & aDecoder)
{
    if (!aPath.IsListItemOperation())
    {
        ListStructOctetString::TypeInfo::DecodableType list;

        ReturnErrorOnFailure(aDecoder.Decode(list));

        uint8_t index = 0;
        auto iter     = list.begin();
        while (iter.Next())
        {
            const auto & entry = iter.GetValue();

            VerifyOrReturnError(index < kAttributeListLength, CHIP_ERROR_BUFFER_TOO_SMALL);
            VerifyOrReturnError(entry.member2.size() <= kAttributeEntryLength, CHIP_ERROR_BUFFER_TOO_SMALL);
            memcpy(gListOperationalCert[index].Data(), entry.member2.data(), entry.member2.size());
            gListOperationalCert[index].SetLength(entry.member2.size());

            listStructOctetStringData[index].member1 = entry.member1;
            listStructOctetStringData[index].member2 = gListOperationalCert[index].AsSpan();

            index++;
        }

        gListOperationalCertLen = index;

        if (iter.GetStatus() != CHIP_NO_ERROR)
        {
            return CHIP_ERROR_INVALID_DATA_LIST;
        }

        return CHIP_NO_ERROR;
    }
    if (aPath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        chip::app::Clusters::UnitTesting::Structs::TestListStructOctet::DecodableType entry;
        ReturnErrorOnFailure(aDecoder.Decode(entry));
        size_t index = gListOperationalCertLen;

        VerifyOrReturnError(index < kAttributeListLength, CHIP_ERROR_BUFFER_TOO_SMALL);
        VerifyOrReturnError(entry.member2.size() <= kAttributeEntryLength, CHIP_ERROR_BUFFER_TOO_SMALL);
        memcpy(gListOperationalCert[index].Data(), entry.member2.data(), entry.member2.size());
        gListOperationalCert[index].SetLength(entry.member2.size());

        listStructOctetStringData[index].member1 = entry.member1;
        listStructOctetStringData[index].member2 = gListOperationalCert[index].AsSpan();

        gListOperationalCertLen++;
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR TestAttrAccess::ReadListNullablesAndOptionalsStructAttribute(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        // Just encode our one struct for now.
        ReturnErrorOnFailure(encoder.Encode(gNullablesAndOptionalsStruct));
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR TestAttrAccess::WriteListNullablesAndOptionalsStructAttribute(const ConcreteDataAttributePath & aPath,
                                                                         AttributeValueDecoder & aDecoder)
{
    static size_t count = 1;
    if (!aPath.IsListItemOperation())
    {
        DataModel::DecodableList<Structs::NullablesAndOptionalsStruct::DecodableType> list;
        ReturnErrorOnFailure(aDecoder.Decode(list));

        ReturnErrorOnFailure(list.ComputeSize(&count));
        // We are assuming we are using list chunking feature for attribute writes.
        VerifyOrReturnError(count == 0, CHIP_ERROR_INVALID_ARGUMENT);

        return CHIP_NO_ERROR;
    }
    if (aPath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        // And we only support one entry in the list.
        VerifyOrReturnError(count == 0, CHIP_ERROR_INVALID_ARGUMENT);

        Structs::NullablesAndOptionalsStruct::DecodableType value;
        ReturnErrorOnFailure(aDecoder.Decode(value));

        // We only support some values so far.
        VerifyOrReturnError(value.nullableString.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(value.nullableStruct.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);

        VerifyOrReturnError(!value.optionalString.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(!value.nullableOptionalString.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(!value.optionalStruct.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(!value.nullableOptionalStruct.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(!value.optionalList.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(!value.nullableOptionalList.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);

        count++;

        // Start our value off as null, just in case we fail to decode things.
        gNullablesAndOptionalsStruct.nullableList.SetNull();
        if (!value.nullableList.IsNull())
        {
            ReturnErrorOnFailure(value.nullableList.Value().ComputeSize(&count));
            VerifyOrReturnError(count <= MATTER_ARRAY_SIZE(gSimpleEnums), CHIP_ERROR_INVALID_ARGUMENT);
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

        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
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

CHIP_ERROR TestAttrAccess::ReadGlobalStruct(AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(gGlobalStructAttributeValue);
}

CHIP_ERROR TestAttrAccess::WriteGlobalStruct(AttributeValueDecoder & aDecoder)
{
    // We don't support a nonempty charspan here for now.
    Attributes::GlobalStruct::TypeInfo::DecodableType temp;
    ReturnErrorOnFailure(aDecoder.Decode(temp));

    VerifyOrReturnError(temp.name.empty(), CHIP_ERROR_BUFFER_TOO_SMALL);

    gGlobalStructAttributeValue = temp;
    return CHIP_NO_ERROR;
}

CHIP_ERROR TestAttrAccess::ReadListFabricScopedAttribute(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        for (size_t index = 0; index < gListFabricScopedAttributeLen; index++)
        {
            ReturnErrorOnFailure(encoder.Encode(gListFabricScopedAttributeValue[index]));
        }

        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR TestAttrAccess::WriteListFabricScopedListEntry(const Structs::TestFabricScoped::DecodableType & entry, size_t index)
{
    VerifyOrReturnError(index < kAttributeListLength, CHIP_ERROR_BUFFER_TOO_SMALL);

    //
    // The fabric index in the entry has already been set to the right index
    // by the decoder.
    //
    gListFabricScopedAttributeValue[index].fabricIndex = entry.fabricIndex;

    gListFabricScopedAttributeValue[index].optionalFabricSensitiveInt8u         = entry.optionalFabricSensitiveInt8u;
    gListFabricScopedAttributeValue[index].nullableFabricSensitiveInt8u         = entry.nullableFabricSensitiveInt8u;
    gListFabricScopedAttributeValue[index].nullableOptionalFabricSensitiveInt8u = entry.nullableOptionalFabricSensitiveInt8u;

    VerifyOrReturnError(entry.fabricSensitiveCharString.size() < kFabricSensitiveCharLength, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(gListFabricScoped_fabricSensitiveCharBuf[index], entry.fabricSensitiveCharString.data(),
           entry.fabricSensitiveCharString.size());
    gListFabricScopedAttributeValue[index].fabricSensitiveCharString =
        CharSpan(gListFabricScoped_fabricSensitiveCharBuf[index], entry.fabricSensitiveCharString.size());

    //
    // For now, we're not permitting the SimpleStruct's contents to have valid strings, since that just
    // increases the complexity of this logic. We don't really need to validate that since there are other tests
    // that validate that struct, so let's just do the bare minimum here.
    //
    VerifyOrReturnError(entry.fabricSensitiveStruct.d.size() == 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(entry.fabricSensitiveStruct.e.size() == 0, CHIP_ERROR_INVALID_ARGUMENT);

    gListFabricScopedAttributeValue[index].fabricSensitiveStruct = entry.fabricSensitiveStruct;
    gListFabricScopedAttributeValue[index].fabricSensitiveInt8u  = entry.fabricSensitiveInt8u;

    auto intIter = entry.fabricSensitiveInt8uList.begin();
    size_t i     = 0;
    while (intIter.Next())
    {
        VerifyOrReturnError(i < kFabricSensitiveIntListLength, CHIP_ERROR_BUFFER_TOO_SMALL);
        gListFabricScoped_fabricSensitiveInt8uList[index][i++] = intIter.GetValue();
    }
    ReturnErrorOnFailure(intIter.GetStatus());

    gListFabricScopedAttributeValue[index].fabricSensitiveInt8uList =
        DataModel::List<uint8_t>(gListFabricScoped_fabricSensitiveInt8uList[index], i);

    return CHIP_NO_ERROR;
}

CHIP_ERROR TestAttrAccess::WriteListFabricScopedAttribute(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    if (!aPath.IsListItemOperation())
    {
        ListFabricScoped::TypeInfo::DecodableType list;

        ReturnErrorOnFailure(aDecoder.Decode(list));

        //
        // Delete all existing entries matching the accessing fabric. This is achieved by 'shifting down'
        // entries that don't match the accessing fabric into the slots occupied by the deleted entries.
        //
        size_t srcIndex = 0, dstIndex = 0;
        while (srcIndex < gListFabricScopedAttributeLen)
        {
            if (gListFabricScopedAttributeValue[srcIndex].fabricIndex != aDecoder.AccessingFabricIndex())
            {
                auto & dstEntry = gListFabricScopedAttributeValue[dstIndex];
                auto & srcEntry = gListFabricScopedAttributeValue[srcIndex];

                dstEntry = srcEntry;

                //
                // We copy the data referenced by spans over to the right slot in the backing buffers.
                //
                memcpy(gListFabricScoped_fabricSensitiveCharBuf[dstIndex], srcEntry.fabricSensitiveCharString.data(),
                       srcEntry.fabricSensitiveCharString.size());
                dstEntry.fabricSensitiveCharString =
                    CharSpan(gListFabricScoped_fabricSensitiveCharBuf[dstIndex], srcEntry.fabricSensitiveCharString.size());

                memcpy(gListFabricScoped_fabricSensitiveInt8uList[dstIndex], gListFabricScoped_fabricSensitiveInt8uList[srcIndex],
                       srcEntry.fabricSensitiveInt8uList.size() * sizeof(uint8_t));
                gListFabricScopedAttributeValue[dstIndex].fabricSensitiveInt8uList = DataModel::List<uint8_t>(
                    gListFabricScoped_fabricSensitiveInt8uList[dstIndex], srcEntry.fabricSensitiveInt8uList.size());

                dstIndex++;
            }

            srcIndex++;
        }

        size_t size;
        ReturnErrorOnFailure(list.ComputeSize(&size));

        auto iter = list.begin();
        while (iter.Next())
        {
            auto & entry = iter.GetValue();
            ReturnErrorOnFailure(WriteListFabricScopedListEntry(entry, dstIndex++));
        }

        gListFabricScopedAttributeLen = dstIndex;
        return iter.GetStatus();
    }
    if (aPath.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
    {
        VerifyOrReturnError(gListFabricScopedAttributeLen < kAttributeListLength, CHIP_ERROR_INVALID_ARGUMENT);

        Structs::TestFabricScoped::DecodableType listEntry;
        ReturnErrorOnFailure(aDecoder.Decode(listEntry));
        ReturnErrorOnFailure(WriteListFabricScopedListEntry(listEntry, gListFabricScopedAttributeLen));

        gListFabricScopedAttributeLen++;
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

} // namespace

bool emberAfUnitTestingClusterTestCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                           const Clusters::UnitTesting::Commands::Test::DecodableType & commandData)
{
    // Setup the test variables
    emberAfInitializeAttributes(commandPath.mEndpointId);
    for (int i = 0; i < kAttributeListLength; ++i)
    {
        gListUint8Data[i] = 0;
        gListOctetStringData[i].SetLength(0);
        gListOperationalCert[i].SetLength(0);
        listStructOctetStringData[i].member1 = 0;
        listStructOctetStringData[i].member2 = ByteSpan();
        gSimpleEnums[i]                      = SimpleEnum::kUnspecified;
    }
    gSimpleEnumCount = 0;

    gStructAttributeValue = Structs::SimpleStruct::Type();

    gNullableStructAttributeValue.SetNull();

    gNullablesAndOptionalsStruct = Structs::NullablesAndOptionalsStruct::Type();

    commandObj->AddStatus(commandPath, Status::Success);
    return true;
}

bool emberAfUnitTestingClusterTestSpecificCallback(CommandHandler * apCommandObj, const ConcreteCommandPath & commandPath,
                                                   const TestSpecific::DecodableType & commandData)
{
    TestSpecificResponse::Type responseData;
    responseData.returnValue = 7;
    apCommandObj->AddResponse(commandPath, responseData);
    return true;
}

bool emberAfUnitTestingClusterTestNotHandledCallback(CommandHandler *, const ConcreteCommandPath & commandPath,
                                                     const TestNotHandled::DecodableType & commandData)
{
    return false;
}

bool emberAfUnitTestingClusterTestAddArgumentsCallback(CommandHandler * apCommandObj, const ConcreteCommandPath & commandPath,
                                                       const TestAddArguments::DecodableType & commandData)
{
    if (commandData.arg1 > UINT8_MAX - commandData.arg2)
    {
        apCommandObj->AddStatus(commandPath, Status::InvalidCommand);
        return true;
    }

    TestAddArgumentsResponse::Type responseData;
    responseData.returnValue = static_cast<uint8_t>(commandData.arg1 + commandData.arg2);
    apCommandObj->AddResponse(commandPath, responseData);
    return true;
}

static bool SendBooleanResponse(CommandHandler * commandObj, const ConcreteCommandPath & commandPath, bool value)
{
    Commands::BooleanResponse::Type response;
    response.value = value;
    commandObj->AddResponse(commandPath, response);
    return true;
}

bool emberAfUnitTestingClusterTestDifferentVendorMeiRequestCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::TestDifferentVendorMeiRequest::DecodableType & commandData)
{
    Commands::TestDifferentVendorMeiResponse::Type response;

    {
        Events::TestDifferentVendorMeiEvent::Type event{ commandData.arg1 };

        if (CHIP_NO_ERROR != LogEvent(event, commandPath.mEndpointId, response.eventNumber))
        {
            commandObj->AddStatus(commandPath, Status::Failure);
            return true;
        }
    }

    response.arg1 = commandData.arg1;
    commandObj->AddResponse(commandPath, response);
    return true;
}

bool emberAfUnitTestingClusterTestStructArgumentRequestCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::TestStructArgumentRequest::DecodableType & commandData)
{
    return SendBooleanResponse(commandObj, commandPath, commandData.arg1.b);
}

bool emberAfUnitTestingClusterTestNestedStructArgumentRequestCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::TestNestedStructArgumentRequest::DecodableType & commandData)
{
    return SendBooleanResponse(commandObj, commandPath, commandData.arg1.c.b);
}

bool emberAfUnitTestingClusterTestListStructArgumentRequestCallback(
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
        commandObj->AddStatus(commandPath, Status::Failure);
        return true;
    }

    return SendBooleanResponse(commandObj, commandPath, shouldReturnTrue);
}

bool emberAfUnitTestingClusterTestEmitTestEventRequestCallback(
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
        commandObj->AddStatus(commandPath, Status::Failure);
        return true;
    }

    commandObj->AddResponse(commandPath, responseData);
    return true;
}

bool emberAfUnitTestingClusterTestEmitTestFabricScopedEventRequestCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const Commands::TestEmitTestFabricScopedEventRequest::DecodableType & commandData)
{
    Commands::TestEmitTestFabricScopedEventResponse::Type responseData;
    Events::TestFabricScopedEvent::Type event{ commandData.arg1 };
    event.fabricIndex = commandData.arg1;
    if (CHIP_NO_ERROR != LogEvent(event, commandPath.mEndpointId, responseData.value))
    {
        commandObj->AddStatus(commandPath, Status::Failure);
        return true;
    }
    commandObj->AddResponse(commandPath, responseData);
    return true;
}

bool emberAfUnitTestingClusterTestListInt8UArgumentRequestCallback(
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
        commandObj->AddStatus(commandPath, Status::Failure);
        return true;
    }

    return SendBooleanResponse(commandObj, commandPath, shouldReturnTrue);
}

bool emberAfUnitTestingClusterTestNestedStructListArgumentRequestCallback(
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
        commandObj->AddStatus(commandPath, Status::Failure);
        return true;
    }

    return SendBooleanResponse(commandObj, commandPath, shouldReturnTrue);
}

bool emberAfUnitTestingClusterTestListNestedStructListArgumentRequestCallback(
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
            commandObj->AddStatus(commandPath, Status::Failure);
            return true;
        }
    }

    if (CHIP_NO_ERROR != structIterator.GetStatus())
    {
        commandObj->AddStatus(commandPath, Status::Failure);
        return true;
    }

    return SendBooleanResponse(commandObj, commandPath, shouldReturnTrue);
}

bool emberAfUnitTestingClusterTestListInt8UReverseRequestCallback(
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
            commandObj->AddResponse(commandPath, responseData);
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
        commandObj->AddResponse(commandPath, responseData);
        return true;
    }

exit:
    commandObj->AddStatus(commandPath, Status::Failure);
    return true;
}

bool emberAfUnitTestingClusterTestEnumsRequestCallback(CommandHandler * commandObj, ConcreteCommandPath const & commandPath,
                                                       TestEnumsRequest::DecodableType const & commandData)
{
    TestEnumsResponse::Type response;
    response.arg1 = commandData.arg1;
    response.arg2 = commandData.arg2;

    commandObj->AddResponse(commandPath, response);
    return true;
}

bool emberAfUnitTestingClusterTestNullableOptionalRequestCallback(
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

    commandObj->AddResponse(commandPath, response);
    return true;
}

bool emberAfUnitTestingClusterSimpleStructEchoRequestCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
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

    commandObj->AddResponse(commandPath, response);
    return true;
}

bool emberAfUnitTestingClusterStringEchoRequestCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                        const Commands::StringEchoRequest::DecodableType & commandData)
{
    Commands::StringEchoResponse::Type response;
    response.payload = commandData.payload;

    commandObj->AddResponse(commandPath, response);
    return true;
}

bool emberAfUnitTestingClusterTimedInvokeRequestCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                         const Commands::TimedInvokeRequest::DecodableType & commandData)
{
    commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
    return true;
}

bool emberAfUnitTestingClusterTestSimpleOptionalArgumentRequestCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const Commands::TestSimpleOptionalArgumentRequest::DecodableType & commandData)
{
    Protocols::InteractionModel::Status status = commandData.arg1.HasValue() ? Protocols::InteractionModel::Status::Success
                                                                             : Protocols::InteractionModel::Status::InvalidValue;
    commandObj->AddStatus(commandPath, status);
    return true;
}

// TestBatchHelperRequest and TestSecondBatchHelperRequest do the same thing.
// The reason there are two identical commands is because batch command requires
// command paths in the same batch to be unique. These command allow for
// client to control order of the response and control size of CommandDataIB
// being sent back to help test some corner cases.
bool TestBatchHelperCommon(CommandHandler * commandObj, const ConcreteCommandPath & commandPath, const uint16_t sleepTimeMs,
                           const uint16_t sizeOfResponseBuffer, const uint8_t fillCharacter)
{
    if (sizeOfResponseBuffer > kTestBatchHelperResponseBufferMax)
    {
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::ConstraintError);
        return true;
    }

    AsyncBatchCommandsWorkData * asyncWorkData = Platform::New<AsyncBatchCommandsWorkData>();
    if (asyncWorkData == nullptr)
    {
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Busy);
        return true;
    }

    asyncWorkData->asyncCommandHandle   = commandObj;
    asyncWorkData->commandPath          = commandPath;
    asyncWorkData->sizeOfResponseBuffer = sizeOfResponseBuffer;
    asyncWorkData->fillCharacter        = fillCharacter;

    scheduleTimerCallbackMs(asyncWorkData, sleepTimeMs);

    return true;
}

bool emberAfUnitTestingClusterTestBatchHelperRequestCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                             const Commands::TestBatchHelperRequest::DecodableType & commandData)
{
    return TestBatchHelperCommon(commandObj, commandPath, commandData.sleepBeforeResponseTimeMs, commandData.sizeOfResponseBuffer,
                                 commandData.fillCharacter);
}

bool emberAfUnitTestingClusterTestSecondBatchHelperRequestCallback(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const Commands::TestSecondBatchHelperRequest::DecodableType & commandData)
{
    return TestBatchHelperCommon(commandObj, commandPath, commandData.sleepBeforeResponseTimeMs, commandData.sizeOfResponseBuffer,
                                 commandData.fillCharacter);
}

bool emberAfUnitTestingClusterGlobalEchoRequestCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                        const Commands::GlobalEchoRequest::DecodableType & commandData)
{
    Commands::GlobalEchoResponse::Type response;
    response.field1 = commandData.field1;
    response.field2 = commandData.field2;

    commandObj->AddResponse(commandPath, response);
    return true;
}

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterUnitTestingPluginServerInitCallback()
{
    AttributeAccessInterfaceRegistry::Instance().Register(&gAttrAccess);
}
