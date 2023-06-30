#pragma once

#include <app/AttributePersistenceProvider.h>
#include <app/util/attribute-storage.h>

namespace chip {
namespace app {
namespace AttributePersistenceAccessors {

CHIP_ERROR ReadUint8Value(const ConcreteAttributePath & aPath, uint8_t & aValue)
{
    uint8_t attrData[1];
    MutableByteSpan tempVal(attrData, 1);
    auto err = GetAttributePersistenceProvider()->ReadValue(aPath, ZCL_INT8U_ATTRIBUTE_TYPE, 1, tempVal);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    chip::Encoding::LittleEndian::Reader r(tempVal.data(), tempVal.size());
    return r.Read8(&aValue).StatusCode();
}

CHIP_ERROR ReadNullableUint8Value(const ConcreteAttributePath & aPath, DataModel::Nullable<uint8_t> & aValue)
{
    uint8_t tmpVal;
    auto err = ReadUint8Value(aPath, tmpVal);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    aValue = DataModel::Nullable<uint8_t>(tmpVal);
    return CHIP_NO_ERROR;
}

CHIP_ERROR WriteUint8Value(const ConcreteAttributePath & aPath, uint8_t & aValue)
{
    return GetAttributePersistenceProvider()->WriteValue(aPath, ByteSpan(&aValue, 1));
}

CHIP_ERROR WriteNullableUint8Value(const ConcreteAttributePath & aPath, DataModel::Nullable<uint8_t> & aValue)
{
    if (aValue.IsNull())
    {
        return GetAttributePersistenceProvider()->WriteValue(aPath, ByteSpan(nullptr, 0));
    }
    return GetAttributePersistenceProvider()->WriteValue(aPath, ByteSpan(&aValue.Value(), 1));
}

}
}
}