#pragma once

#include <array>
#include <type_traits>
#include "TestCluster.h"

namespace chip {
namespace app {
namespace Cluster {
namespace TestCluster {
    namespace StructA {
        constexpr int NumImplementedFields = GetNumImplementedFields(_Schema);
        extern const StructDescriptor<NumImplementedFields> Descriptor;

        struct Type {
            uint8_t x;
            uint8_t y;
            chip::ByteSpan l;
            chip::Span<char> m;

            static const StructDescriptor<NumImplementedFields> &mDescriptor;
        };
    }

    namespace StructB {
        constexpr int NumImplementedFields = GetNumImplementedFields(_Schema);
        extern const StructDescriptor<NumImplementedFields> Descriptor;

        struct Type {
            uint8_t x;
            uint8_t y;
            StructA::Type z;

            static const StructDescriptor<NumImplementedFields> &mDescriptor;
        };
    }

    namespace StructC {
        constexpr int NumImplementedFields = GetNumImplementedFields(_Schema);
        extern const StructDescriptor<NumImplementedFields> Descriptor;

        struct Type {
            struct empty {};

            uint8_t a;
            uint8_t b;
            StructA::Type c;
            chip::Span<uint8_t> d;
            chip::Span<StructA::Type> e;

            static const StructDescriptor<NumImplementedFields> &mDescriptor;
        };
    }

    namespace CommandA {
        constexpr int NumImplementedFields = GetNumImplementedFields(_Schema);
        extern const StructDescriptor<NumImplementedFields> Descriptor;

        struct Type {
            struct empty {};

            uint8_t a;
            uint8_t b;
            StructA::Type c;
            chip::Span<uint8_t> d;

            static chip::ClusterId GetClusterId() { return kClusterId; }
            static chip::CommandId GetCommandId() { return kCommandAId; }

            static const StructDescriptor<NumImplementedFields> &mDescriptor;
        };
    }

    namespace CommandB {
        constexpr int NumImplementedFields = GetNumImplementedFields(_Schema);
        extern const StructDescriptor<NumImplementedFields> Descriptor;

        struct Type {
            struct empty {};

            uint8_t a;
            uint8_t b;
            StructA::Type c;
            chip::Span<uint8_t> d;
            chip::Span<StructA::Type> e;

            static chip::ClusterId GetClusterId() { return kClusterId; }
            static chip::CommandId GetCommandId() { return kCommandBId; }

            static const StructDescriptor<NumImplementedFields> &mDescriptor;
        };
    }

    namespace Attributes {
        constexpr int NumImplementedFields = GetNumImplementedFields(_Schema);
        extern const StructDescriptor<NumImplementedFields> Descriptor;

        struct Type {
            struct empty {};

            typedef typename std::conditional<IsImplemented(FieldC), uint32_t, empty>::type FieldCType;
            typedef typename std::conditional<IsImplemented(FieldD), uint8_t, empty>::type FieldDType;

            uint8_t a;
            uint8_t b;
            FieldCType c;
            FieldDType d;
            chip::Span<uint8_t> e;
            StructB::Type f;

            static const StructDescriptor<NumImplementedFields> &mDescriptor;
        };
    }
}
}
}
}
