#include "assert.h"
#include "encoding.h"
#include <string.h>


void Encoder::init(uint8_t *data, size_t size)
{
    _data = data;
    _size = size;
    reset();
}

void Encoder::reset()
{
    _offset = 0;
}

int Encoder::addUint8(uint8_t in)
{
    ASSERT(_data, return 1, "Invalid pointer");
    ASSERT(_size > 0, return 2, "Invalid size");
    ASSERT((_offset + 2) <= _size, return 3, "Buffer at end");
    _data[_offset++] = Type_UInt8;
    _data[_offset++] = in;
    return 0;
}

int Encoder::getUint8(uint8_t &out)
{
    ASSERT(_data, return 1, "Invalid pointer");
    ASSERT(_size > 0, return 2, "Invalid size");
    ASSERT((_offset + 2) <= _size, return 3, "Buffer at end");
    ASSERT(Type_UInt8 == _data[_offset++], return 4, "Invalid type");

    out = _data[_offset++];
    return 0;
}

int Encoder::addUint16(uint16_t in)
{
    ASSERT(_data, return 1, "Invalid pointer");
    ASSERT(_size > 0, return 2, "Invalid size");
    ASSERT((_offset + 3) <= _size, return 3, "Buffer at end");
    _data[_offset++] = Type_UInt16;
    _data[_offset++] = (in >> 8) & 0xff;
    _data[_offset++] = in & 0xff;
    return 0;
}

int Encoder::getUint16(uint16_t &out)
{
    ASSERT(_data, return 1, "Invalid pointer");
    ASSERT(_size > 0, return 2, "Invalid size");
    ASSERT((_offset + 3) <= _size, return 3, "Buffer at end");
    ASSERT(Type_UInt16 == _data[_offset++], return 4, "Invalid type");
    const uint8_t *p = &_data[_offset];
    out = (p[0] << 8) + p[1];
    _offset += 2;
    return 0;
}

int Encoder::addUint32(uint32_t in)
{
    ASSERT(_data, return 1, "Invalid pointer");
    ASSERT(_size > 0, return 2, "Invalid size");
    ASSERT((_offset + 3) <= _size, return 3, "Buffer at end");
    _data[_offset++] = Type_UInt32;
    _data[_offset++] = (in >> 24) & 0xff;
    _data[_offset++] = (in >> 16) & 0xff;
    _data[_offset++] = (in >> 8) & 0xff;
    _data[_offset++] = in & 0xff;
    return 0;
}

int Encoder::addInt32(int32_t in)
{
    return (in < 0) ? addUint32(0x80000000 | (-in)) : addUint32(in);
}

int Encoder::getUint32(uint32_t &out)
{
    ASSERT(_data, return 1, "Invalid pointer");
    ASSERT(_size > 0, return 2, "Invalid size");
    ASSERT((_offset + 3) <= _size, return 3, "Buffer at end");
    ASSERT(Type_UInt32 == _data[_offset++], return 4, "Invalid type");
    const uint8_t *p = &_data[_offset];
    out = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
    _offset += 4;
    return 0;
}


int Encoder::addArray(const uint8_t *in, size_t size)
{
    ASSERT(_data, return 1, "Invalid pointer");
    ASSERT(_size > 0, return 2, "Invalid size");
    ASSERT((_offset + 3) <= _size, return 3, "Buffer at end");
    _data[_offset++] = Type_Array;
    _data[_offset++] = (size >> 8) & 0xff;
    _data[_offset++] = (size) & 0xff;
    memcpy(&_data[_offset], in, size);
    _offset += size;
    return 0;
}

int Encoder::getArray(uint8_t *out, size_t max_size, size_t &size)
{
    size = 0;
    ASSERT(_data, return 1, "Invalid pointer");
    ASSERT(_size > 0, return 2, "Invalid size");
    ASSERT((_offset + 3) <= _size, return 3, "Buffer at end");
    ASSERT(Type_Array == _data[_offset++], return 4, "Invalid type");

    const uint8_t *p = &_data[_offset];
    size_t sz = (p[0] << 8) + p[1];
    ASSERT((sz <= max_size) && (_offset + 2 + sz <= _size), return 5, "Invalid size");
    memcpy(out, &p[2], sz);
    size = sz;
    _offset += 2 + sz;
    return 0;
}

int Encoder::addString(const char *in)
{
    ASSERT(_data, return 1, "Invalid pointer");
    ASSERT(_size > 0, return 2, "Invalid size");
    size_t size = strlen(in);
    return addArray((uint8_t *)in, size + 1);
}

int Encoder::getString(char *out, size_t max_size, size_t &size)
{
    ASSERT(max_size > 0, return 1, "Invalid size");
    int err = this->getArray((uint8_t *)out, max_size - 1, size);
    ASSERT(!err, return err, "Array encoding error");
    out[size] = 0;
    return 0;
}
