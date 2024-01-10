#include "ProvisionEncoder.h"
#include <lib/core/CHIPEncoding.h>
#include <lib/support/CodeUtils.h>
#include <string.h>

using namespace chip::Encoding;

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {


void Encoder::Reset()
{
    _offset = 0;
}

CHIP_ERROR Encoder::addUint8(uint8_t in)
{
    VerifyOrReturnError(nullptr != _data, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(_size > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError((_offset + 2) <= _size, CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t *p = &_data[_offset];
    Write8(p, Type_UInt8);
    Write8(p, in);
    _offset += 2;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Encoder::getUint8(uint8_t &out)
{
    VerifyOrReturnError(nullptr != _data, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(_size > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError((_offset + 2) <= _size, CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t *p = &_data[_offset];
    VerifyOrReturnError(Type_UInt8 == Read8(p), CHIP_ERROR_WRONG_KEY_TYPE);
    out = Read8(p);
    _offset += 2;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Encoder::addUint16(uint16_t in)
{
    VerifyOrReturnError(nullptr != _data, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(_size > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError((_offset + 3) <= _size, CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t *p = &_data[_offset];
    Write8(p, Type_UInt16);
    BigEndian::Write16(p, in);
    _offset += 3;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Encoder::getUint16(uint16_t &out)
{
    VerifyOrReturnError(nullptr != _data, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(_size > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError((_offset + 3) <= _size, CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t *p = &_data[_offset];
    VerifyOrReturnError(Type_UInt16 == Read8(p), CHIP_ERROR_WRONG_KEY_TYPE);
    out = BigEndian::Read16(p);
    _offset += 3;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Encoder::addUint32(uint32_t in)
{
    VerifyOrReturnError(nullptr != _data, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(_size > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError((_offset + 3) <= _size, CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t *p = &_data[_offset];
    Write8(p, Type_UInt32);
    BigEndian::Write32(p, in);
    _offset += 5;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Encoder::addInt32(int32_t in)
{
    return (in < 0) ? addUint32(0x80000000 | (-in)) : addUint32(in);
}

CHIP_ERROR Encoder::getUint32(uint32_t &out)
{
    VerifyOrReturnError(nullptr != _data, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(_size > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError((_offset + 3) <= _size, CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t *p = &_data[_offset];
    VerifyOrReturnError(Type_UInt32 == Read8(p), CHIP_ERROR_WRONG_KEY_TYPE);
    out = BigEndian::Read32(p);
    _offset += 5;
    return CHIP_NO_ERROR;
}


CHIP_ERROR Encoder::addArray(const uint8_t *in, size_t size)
{
    VerifyOrReturnError(nullptr != _data, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(_size > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError((_offset + 3) <= _size, CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t *p = &_data[_offset];
    Write8(p, Type_Array);
    BigEndian::Write16(p, (uint16_t) size);
    memcpy(p, in, size);
    _offset += 3 + size;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Encoder::getArray(uint8_t *out, size_t max_size, size_t &size)
{
    size = 0;
    VerifyOrReturnError(nullptr != _data, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(_size > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError((_offset + 3) <= _size, CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t *p = &_data[_offset];
    VerifyOrReturnError(Type_Array == Read8(p), CHIP_ERROR_WRONG_KEY_TYPE);
    uint16_t sz = BigEndian::Read16(p);
    VerifyOrReturnError((sz <= max_size) && (_offset + 2 + sz <= _size), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(out, p, sz);
    size = sz;
    _offset += 3 + sz;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Encoder::addString(const char *in)
{
    VerifyOrReturnError(nullptr != _data, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(_size > 0, CHIP_ERROR_INVALID_ARGUMENT);
    size_t size = strlen(in);
    return addArray((uint8_t *)in, size + 1);
}

CHIP_ERROR Encoder::getString(char *out, size_t max_size, size_t &size)
{
    VerifyOrReturnError(max_size > 0, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(this->getArray((uint8_t *)out, max_size - 1, size));
    out[size] = 0;
    return CHIP_NO_ERROR;
}

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
