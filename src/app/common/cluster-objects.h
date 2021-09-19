#pragma once

#include <app/data-model/DecodableList.h>
#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>
#include <app/data-model/List.h>
#include <app/util/basic-types.h>

namespace chip {
namespace app {
namespace clusters {
namespace TestCluster {
constexpr ClusterId kClusterId = 0x0000050F;

enum class SimpleEnum : uint8_t
{
    UNSPECIFIED = 0x00,
    VALUEA      = 0x01,
    VALUEB      = 0x02,
    VALUEC      = 0x03
};

namespace TestListStructOctet {
enum FieldId
{
    kFabricIndexFieldId     = 0,
    kOperationalCertFieldId = 1,
};

struct Type
{
    int64_t fabricIndex;
    ByteSpan operationalCert;

    CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag) const;
    CHIP_ERROR Decode(TLV::TLVReader & reader);
};
} // namespace TestListStructOctet

namespace SimpleStruct {
enum FieldId
{
    kAFieldId = 0,
    kBFieldId = 1,
    kCFieldId = 2,
    kDFieldId = 3,
    kEFieldId = 4
};

struct Type
{
    uint8_t a    = 0;
    bool b       = false;
    SimpleEnum c = SimpleEnum::UNSPECIFIED;
    ByteSpan d;
    Span<const char> e;

    CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag) const;
    CHIP_ERROR Decode(TLV::TLVReader & reader);
};
} // namespace SimpleStruct

namespace NestedStruct {
enum FieldId
{
    kAFieldId = 0,
    kBFieldId = 1,
    kCFieldId = 2
};

class Type
{
public:
    uint8_t a = 0;
    bool b    = false;
    SimpleStruct::Type c;

    CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag) const;
    CHIP_ERROR Decode(TLV::TLVReader & reader);
};
} // namespace NestedStruct

namespace NestedStructList {
enum FieldId
{
    kAFieldId = 0,
    kBFieldId = 1,
    kCFieldId = 2,
    kDFieldId = 3,
    kEFieldId = 4,
    kFFieldId = 5,
    kGFieldId = 6,
};

struct Type
{
public:
    uint8_t a = 0;
    bool b    = false;
    SimpleStruct::Type c;
    DataModel::List<SimpleStruct::Type> d;
    DataModel::List<uint32_t> e;
    DataModel::List<ByteSpan> f;
    DataModel::List<uint8_t> g;

    CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag) const;
    CHIP_ERROR Decode(TLV::TLVReader & reader);
};

struct DecodableType
{
public:
    uint8_t a = 0;
    bool b    = false;
    SimpleStruct::Type c;
    DataModel::DecodableList<SimpleStruct::Type> d;
    DataModel::DecodableList<uint32_t> e;
    DataModel::DecodableList<ByteSpan> f;
    DataModel::DecodableList<uint8_t> g;

    CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag) const;
    CHIP_ERROR Decode(TLV::TLVReader & reader);
};

} // namespace NestedStructList

namespace DoubleNestedStructList {
enum FieldId
{
    kAFieldId = 0,
};

struct Type
{
public:
    DataModel::List<NestedStructList::Type> a;

    CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag) const;
    CHIP_ERROR Decode(TLV::TLVReader & reader);
};

struct DecodableType
{
public:
    DataModel::DecodableList<NestedStructList::DecodableType> a;

    CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag) const;
    CHIP_ERROR Decode(TLV::TLVReader & reader);
};

} // namespace DoubleNestedStructList

} // namespace TestCluster
} // namespace clusters
} // namespace app
} // namespace chip
