#pragma once

#include <IEncodableElement.h>
#include <array>
#include <basic-types.h>
#include <string>
#include <type_traits>
#include <vector>

namespace chip {
namespace app {
namespace Cluster {
namespace TestCluster {
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

struct Type : public IEncodableElement
{
    uint8_t x;
    uint8_t y;
    std::vector<uint8_t> l;
    std::string m;

    CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag) final;
    CHIP_ERROR Decode(TLV::TLVReader & reader) final;
};
} // namespace StructA

namespace StructB {
enum FieldId
{
    kFieldIdX = 0,
    kFieldIdY = 1,
    kFieldIdZ = 2
};

struct Type : public IEncodableElement
{
    uint8_t x;
    uint8_t y;
    StructA::Type z;

    CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag) final;
    CHIP_ERROR Decode(TLV::TLVReader & reader) final;
};
} // namespace StructB

namespace StructC {
enum FieldId
{
    kFieldIdA = 0,
    kFieldIdB = 1,
    kFieldIdC = 2,
    kFieldIdD = 3,
    kFieldIdE = 4,
};

struct Type : public IEncodableElement
{
public:
    struct empty
    {
    };

public:
    uint8_t a;
    uint8_t b;
    StructA::Type c;
    std::vector<uint8_t> d;
    std::vector<StructA::Type> e;

    CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag) final;
    CHIP_ERROR Decode(TLV::TLVReader & reader) final;
};
} // namespace StructC

namespace CommandA {
enum FieldId
{
    kFieldIdA = 0,
    kFieldIdB = 1,
    kFieldIdC = 2,
    kFieldIdD = 3,
};

struct Type : public IEncodableElement
{
public:
    uint8_t a;
    uint8_t b;
    StructA::Type c;
    std::vector<uint8_t> d;

    static chip::ClusterId GetClusterId() { return kClusterId; }
    static chip::CommandId GetCommandId() { return kCommandAId; }

    CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag) final;
    CHIP_ERROR Decode(TLV::TLVReader & reader) final;
};
} // namespace CommandA

namespace CommandB {
enum FieldId
{
    kFieldIdA = 0,
    kFieldIdB = 1,
    kFieldIdC = 2,
    kFieldIdD = 3,
    kFieldIdE = 4,
};

struct Type : public IEncodableElement
{
    struct empty
    {
    };

    uint8_t a;
    uint8_t b;
    StructA::Type c;
    std::vector<uint8_t> d;
    std::vector<StructA::Type> e;

    static chip::ClusterId GetClusterId() { return kClusterId; }
    static chip::CommandId GetCommandId() { return kCommandBId; }

    CHIP_ERROR Encode(TLV::TLVWriter & writer, uint64_t tag) final;
    CHIP_ERROR Decode(TLV::TLVReader & reader) final;
};
} // namespace CommandB
} // namespace TestCluster
} // namespace Cluster
} // namespace app
} // namespace chip
