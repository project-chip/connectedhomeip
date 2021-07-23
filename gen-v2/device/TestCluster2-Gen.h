#pragma once

#include <IEncodableElement.h>
#include <array>
#include <basic-types.h>
#include <device/SchemaUtils.h>
#include <string>
#include <type_traits>
#include <vector>

namespace chip {
namespace app {
namespace Cluster {
namespace TestCluster2 {
constexpr chip::ClusterId kClusterId = 0x000000001;

enum CommandId
{
    kCommandAId = 1,
    kCommandBId = 2
};

namespace StructA {
enum FieldId
{
    kFieldIdJ = 0,
    kFieldIdK = 1,
    kFieldIdL = 2,
    kFieldIdM = 3
};

struct Type
{
    uint8_t x;
    uint8_t y;
    chip::ByteSpan l;
    chip::Span<char> m;

    CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag);
    CHIP_ERROR Decode(TLV::TLVReader & reader);
};
} // namespace StructA

namespace StructC {
enum FieldId
{
    kFieldIdA = 0,
    kFieldIdB = 1,
    kFieldIdC = 2,
    kFieldIdD = 3,
    kFieldIdE = 4,
};

struct Type
{
public:
    struct empty
    {
    };

public:
    uint8_t a;
    uint8_t b;
    StructA::Type c;
    chip::Span<uint8_t> d;
    chip::Span<StructA::Type> e;

    CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag);
    CHIP_ERROR Decode(TLV::TLVReader & reader);
};
} // namespace StructC

namespace IteratableStructC {
enum FieldId
{
    kFieldIdA = 0,
    kFieldIdB = 1,
    kFieldIdC = 2,
    kFieldIdD = 3,
    kFieldIdE = 4,
};

struct Type
{
public:
    struct empty
    {
    };

public:
    uint8_t a;
    uint8_t b;
    StructA::Type c;
    IteratableList<uint8_t> d;
    IteratableList<StructA::Type> e;

    CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag);
    CHIP_ERROR Decode(TLV::TLVReader & reader);
};
} // namespace IteratableStructC
} // namespace TestCluster2
} // namespace Cluster
} // namespace app
} // namespace chip
