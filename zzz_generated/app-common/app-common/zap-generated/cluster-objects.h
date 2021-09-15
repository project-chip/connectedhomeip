#pragma once

#include <app/ClusterObjectUtils.h>
#include <app/util/basic-types.h>
#include <array>
#include <lib/core/CHIPTLV.h>
#include <string>
#include <type_traits>
#include <vector>

namespace chip {
namespace app {
namespace clusters {
namespace TestCluster {
constexpr chip::ClusterId kClusterId = 0x0000050F;

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
    chip::ByteSpan operationalCert;

    CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag);
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
    chip::ByteSpan d;
    chip::Span<const char> e;

    CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag);
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

    CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag);
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
};

struct Type
{
public:
    uint8_t a = 0;
    bool b    = false;
    SimpleStruct::Type c;
    chip::Span<SimpleStruct::Type> d;
    chip::Span<uint32_t> e;
    chip::Span<chip::ByteSpan> f;

    CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag);
    CHIP_ERROR Decode(TLV::TLVReader & reader);
};
} // namespace NestedStructList

namespace IteratableNestedStructList {
enum FieldId
{
    kAFieldId = 0,
    kBFieldId = 1,
    kCFieldId = 2,
    kDFieldId = 3,
    kEFieldId = 4,
    kFFieldId = 5
};

struct Type
{
public:
    uint8_t a = 0;
    bool b    = false;
    SimpleStruct::Type c;
    ClusterObjectUtils::IteratableList<SimpleStruct::Type> d;
    ClusterObjectUtils::IteratableList<uint32_t> e;
    ClusterObjectUtils::IteratableList<chip::ByteSpan> f;

    CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag);
    CHIP_ERROR Decode(TLV::TLVReader & reader);
};
} // namespace IteratableNestedStructList

namespace DoubleNestedStructList {
enum FieldId
{
    kAFieldId = 0,
};

struct Type
{
public:
    chip::Span<NestedStructList::Type> a;

    CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag);
    CHIP_ERROR Decode(TLV::TLVReader & reader);
};
} // namespace DoubleNestedStructList

namespace IteratableDoubleNestedStructList {
enum FieldId
{
    kAFieldId = 0,
};

struct Type
{
public:
    ClusterObjectUtils::IteratableList<IteratableNestedStructList::Type> a;

    CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag);
    CHIP_ERROR Decode(TLV::TLVReader & reader);
};
} // namespace IteratableDoubleNestedStructList
} // namespace TestCluster
} // namespace clusters
} // namespace app
} // namespace chip
