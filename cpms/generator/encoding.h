#pragma once

#include <stddef.h>
#include <stdint.h>

enum EncodingTypes
{
    Type_None = 0,
    Type_UInt8 = 1,
    Type_UInt16 = 2,
    Type_UInt32 = 3,
    Type_Array = 4
};


struct Encoder
{
    Encoder(uint8_t *data, size_t size) :
        _data(data), _size(size), _offset(0)
    {}
    virtual ~Encoder() {}

    void init(uint8_t *data, size_t size);
    void reset();
    int addUint8(uint8_t in);
    int getUint8(uint8_t &out);
    int addUint16(uint16_t in);
    int getUint16(uint16_t &out);
    int addInt32(int32_t in);
    int addUint32(uint32_t in);
    int getUint32(uint32_t &out);
    int addArray(const uint8_t *in, size_t size);
    int getArray(uint8_t *out, size_t max_size, size_t &size);
    int addString(const char *in);
    int getString(char *out, size_t max_size, size_t &size);

    const uint8_t *data() { return _data; }
    size_t offset() { return _offset; }

private:
    uint8_t *_data = nullptr;
    size_t _size = 0;
    size_t _offset = 0;
};
