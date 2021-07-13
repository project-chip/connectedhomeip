#pragma once

#include <type_traits>
#include <array>
#include <device/SchemaTypes.h>
#include <basic-types.h>

namespace chip {
namespace app {
namespace Cluster {
namespace TestCluster {
    constexpr chip::ClusterId kClusterId = 0x000000001;

    enum CommandId {
        kCommandAId = 1,
        kCommandBId = 2
    };

    namespace StructA {
        enum FieldId {
            kFieldIdJ = 0,
            kFieldIdK = 1,
            kFieldIdL = 2,
            kFieldIdM = 3
        };

        constexpr uint64_t FieldJ = (0x00000001ULL << 32) | 0x0b;
        constexpr uint64_t FieldK = (0x00000001ULL << 32) | 0x0c;
        constexpr uint64_t FieldL = (0x00000001ULL << 32) | 0x20;
        constexpr uint64_t FieldM = (0x00000001ULL << 32) | 0x21;

        constexpr FullFieldDescriptor _Schema[] = {
            {kFieldIdJ, Type::TYPE_UINT8,   kNone,                  FieldJ, {}},
            {kFieldIdK, Type::TYPE_UINT8,   kNullable,              FieldK, {}},
            {kFieldIdL, Type::TYPE_OCTSTR,  kNone,                  FieldL, {}},
            {kFieldIdM, Type::TYPE_STRING,  kNone,                  FieldM, {}},
        };
    }

    namespace StructB {
        enum FieldId {
            kFieldIdX = 0,
            kFieldIdY = 1,
            kFieldIdZ = 2
        };

        constexpr uint64_t FieldX = (0x00000001ULL << 32) | 0x08;
        constexpr uint64_t FieldY = (0x00000001ULL << 32) | 0x09;
        constexpr uint64_t FieldZ = (0x00000001ULL << 32) | 0x0a;

        constexpr FullFieldDescriptor _Schema[] = {
            {kFieldIdX, Type::TYPE_UINT8, kNone,                  FieldX, {}},
            {kFieldIdY, Type::TYPE_UINT8, kNullable,              FieldY, {}},
            {kFieldIdZ, Type::TYPE_STRUCT, 0,                     FieldZ, {StructA::_Schema, ArraySize(StructA::_Schema)}},
        };
    }

    namespace StructC {
        enum FieldId {
            kFieldIdA = 0,
            kFieldIdB = 1,
            kFieldIdC = 2,
            kFieldIdD = 3,
            kFieldIdE = 4,
        };

        constexpr uint64_t FieldA = (0x00000001ULL << 32) | 0x0d;
        constexpr uint64_t FieldB = (0x00000001ULL << 32) | 0x0e;
        constexpr uint64_t FieldC = (0x00000001ULL << 32) | 0x0f;
        constexpr uint64_t FieldD = (0x00000001ULL << 32) | 0x10;
        constexpr uint64_t FieldE = (0x00000001ULL << 32) | 0x11;

        constexpr FullFieldDescriptor _Schema[] = {
            {kFieldIdA, Type::TYPE_UINT8,           kNone,                  FieldA, {}},
            {kFieldIdB, Type::TYPE_UINT8,           kNullable,              FieldB, {}},
            {kFieldIdC, Type::TYPE_STRUCT,          0,                      FieldC, {StructA::_Schema, ArraySize(StructA::_Schema)}},
            {kFieldIdD, BitFlags<Type>(Type::TYPE_ARRAY)
                        .Set(Type::TYPE_UINT8),     0,                      FieldD, {}},
            {kFieldIdE, BitFlags<Type>(Type::TYPE_ARRAY)
                        .Set(Type::TYPE_STRUCT),    0,                      FieldE, {StructA::_Schema, ArraySize(StructA::_Schema)}},
        };
    }

    namespace CommandA {
        enum FieldId {
            kFieldIdA = 0,
            kFieldIdB = 1,
            kFieldIdC = 2,
            kFieldIdD = 3,
        };

        constexpr uint64_t FieldA = (0x00000001ULL << 32) | 0x12;
        constexpr uint64_t FieldB = (0x00000001ULL << 32) | 0x13;
        constexpr uint64_t FieldC = (0x00000001ULL << 32) | 0x14;
        constexpr uint64_t FieldD = (0x00000001ULL << 32) | 0x15;

        constexpr FullFieldDescriptor _Schema[] = {
            {kFieldIdA, Type::TYPE_UINT8,           kNone,                  FieldA, {}},
            {kFieldIdB, Type::TYPE_UINT8,           kNullable,              FieldB, {}},
            {kFieldIdC, Type::TYPE_STRUCT,          0,                      FieldC, {StructA::_Schema, ArraySize(StructA::_Schema)}},
            {kFieldIdD, BitFlags<Type>(Type::TYPE_ARRAY)
                        .Set(Type::TYPE_UINT8),     0,                      FieldD, {}},
        };
    }

    namespace CommandB {
        enum FieldId {
            kFieldIdA = 0,
            kFieldIdB = 1,
            kFieldIdC = 2,
            kFieldIdD = 3,
            kFieldIdE = 4,
        };

        constexpr uint64_t FieldA = (0x00000001ULL << 32) | 0x16;
        constexpr uint64_t FieldB = (0x00000001ULL << 32) | 0x17;
        constexpr uint64_t FieldC = (0x00000001ULL << 32) | 0x18;
        constexpr uint64_t FieldD = (0x00000001ULL << 32) | 0x19;
        constexpr uint64_t FieldE = (0x00000001ULL << 32) | 0x1a;

        constexpr FullFieldDescriptor _Schema[] = {
            {kFieldIdA, Type::TYPE_UINT8,           kNone,                  FieldA, {}},
            {kFieldIdB, Type::TYPE_UINT8,           kNullable,              FieldB, {}},
            {kFieldIdC, Type::TYPE_STRUCT,          0,                      FieldC, {StructA::_Schema, ArraySize(StructA::_Schema)}},
            {kFieldIdD, BitFlags<Type>(Type::TYPE_ARRAY)
                        .Set(Type::TYPE_UINT8),     0,                      FieldD, {}},
            {kFieldIdE, BitFlags<Type>(Type::TYPE_ARRAY)
                        .Set(Type::TYPE_STRUCT),    0,                      FieldE, {StructA::_Schema, ArraySize(StructA::_Schema)}},
        };
    }

    namespace Attributes {
        enum FieldId {
            kFieldIdA = 0,
            kFieldIdB = 1,
            kFieldIdC = 2,
            kFieldIdD = 3,
            kFieldIdE = 4,
            kFieldIdF = 5
        };

        constexpr uint64_t FieldA = (0x00000001ULL << 32) | 0x01;
        constexpr uint64_t FieldB = (0x00000001ULL << 32) | 0x02;
        constexpr uint64_t FieldC = (0x00000001ULL << 32) | 0x04;
        constexpr uint64_t FieldD = (0x00000001ULL << 32) | 0x05;
        constexpr uint64_t FieldE = (0x00000001ULL << 32) | 0x06;
        constexpr uint64_t FieldF = (0x00000001ULL << 32) | 0x07;

        constexpr FullFieldDescriptor _Schema[] = {
            {kFieldIdA, Type::TYPE_UINT8,           kNone,                  FieldA,     {}},
            {kFieldIdB, Type::TYPE_UINT8,           kNullable,              FieldB,     {}},
            {kFieldIdC, Type::TYPE_UINT32,          kOptional | kNullable,  FieldC,     {}},
            {kFieldIdD, Type::TYPE_UINT8,           kOptional | kNullable,  FieldD,     {}},
            {kFieldIdE, BitFlags<Type>(Type::TYPE_ARRAY).Set(Type::TYPE_UINT8), kNone,                  FieldF,     {}},
            {kFieldIdF, Type::TYPE_STRUCT,          kNone,                  FieldE,     {StructB::_Schema, ArraySize(StructB::_Schema)}},
        };
    }
}
}
}
}
