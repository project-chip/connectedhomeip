#include "AttributePersistenceProvider.h"
#include <lib/support/BufferReader.h>
#include <app-common/zap-generated/attribute-type.h>

namespace chip {
namespace app {

CHIP_ERROR AttributePersistenceProvider::WriteValueUint8(const ConcreteAttributePath & aPath, uint8_t & aValue)
{
    return WriteValue(aPath, ByteSpan(&aValue, 1));
}

CHIP_ERROR AttributePersistenceProvider::ReadValueUint8(const ConcreteAttributePath & aPath, uint8_t & aValue)
{
    uint8_t attrData[1];
    MutableByteSpan tempVal(attrData, 1);
    auto err = ReadValue(aPath, ZCL_INT8U_ATTRIBUTE_TYPE, 1, tempVal);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    chip::Encoding::LittleEndian::Reader r(tempVal.data(), tempVal.size());
    return r.Read8(&aValue).StatusCode();
}

CHIP_ERROR AttributePersistenceProvider::WriteValueNullableUint8(const ConcreteAttributePath & aPath,
                                                                 DataModel::Nullable<uint8_t> & aValue)
{
    if (aValue.IsNull())
    {
        return WriteValue(aPath, ByteSpan(nullptr, 0));
    }
    return WriteValue(aPath, ByteSpan(&aValue.Value(), 1));
}

CHIP_ERROR AttributePersistenceProvider::ReadValueNullableUint8(const ConcreteAttributePath & aPath,
                                                                DataModel::Nullable<uint8_t> & aValue)
{
    uint8_t tmpVal;
    auto err = ReadValueUint8(aPath, tmpVal);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    aValue = DataModel::Nullable<uint8_t>(tmpVal);
    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
