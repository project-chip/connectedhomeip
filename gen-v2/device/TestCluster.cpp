#include "TestCluster-Gen.h"
namespace chip {
namespace app {
namespace Cluster {
namespace TestCluster {
namespace StructA {
constexpr std::array<TypeOffsetInfo, NumImplementedFields> InitializeOffsets()
{
    using result_t           = ::std::array<TypeOffsetInfo, NumImplementedFields>;
    result_t r               = {};
    const result_t & const_r = r;

    uint32_t i = 0;

    const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, x), sizeof(Type::x) };
    const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, y), sizeof(Type::y) };
    const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, l), sizeof(uint8_t) };
    const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, m), sizeof(char) };

    return r;
}

constexpr std::array<TypeOffsetInfo, NumImplementedFields> Offsets = InitializeOffsets();

const StructDescriptor<NumImplementedFields> Descriptor = {
    .FieldList = PopulateFieldDescriptors<NumImplementedFields, ArraySize(_Schema)>(_Schema, Offsets)
};

const StructDescriptor<NumImplementedFields> & Type::mDescriptor = Descriptor;
} // namespace StructA

namespace StructB {
constexpr std::array<TypeOffsetInfo, NumImplementedFields> InitializeOffsets()
{
    using result_t           = ::std::array<TypeOffsetInfo, NumImplementedFields>;
    result_t r               = {};
    const result_t & const_r = r;

    uint32_t i = 0;

    const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, x), sizeof(Type::x) };
    const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, y), sizeof(Type::y) };
    const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, z), sizeof(Type::z) };

    return r;
}

constexpr std::array<TypeOffsetInfo, NumImplementedFields> Offsets = InitializeOffsets();

const StructDescriptor<NumImplementedFields> Descriptor = {
    .FieldList = PopulateFieldDescriptors<NumImplementedFields, ArraySize(_Schema)>(
        _Schema, Offsets,
        chip::Span<const CompactFieldDescriptor>({ StructA::Descriptor.FieldList.data(), StructA::Descriptor.FieldList.size() }))
};

const StructDescriptor<NumImplementedFields> & Type::mDescriptor = Descriptor;
} // namespace StructB

namespace StructC {
constexpr std::array<TypeOffsetInfo, NumImplementedFields> InitializeOffsets()
{
    using result_t           = ::std::array<TypeOffsetInfo, NumImplementedFields>;
    result_t r               = {};
    const result_t & const_r = r;

    uint32_t i = 0;

    const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, a), sizeof(Type::a) };
    const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, b), sizeof(Type::b) };
    const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, c), sizeof(Type::c) };
    const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, d), sizeof(uint8_t) };
    const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, e), sizeof(StructA::Type) };

    return r;
}

constexpr std::array<TypeOffsetInfo, NumImplementedFields> Offsets = InitializeOffsets();

const StructDescriptor<NumImplementedFields> Descriptor = {
    .FieldList = PopulateFieldDescriptors<NumImplementedFields, ArraySize(_Schema)>(
        _Schema, Offsets,
        chip::Span<const CompactFieldDescriptor>({ StructA::Descriptor.FieldList.data(), StructA::Descriptor.FieldList.size() }),
        chip::Span<const CompactFieldDescriptor>({ StructA::Descriptor.FieldList.data(), StructA::Descriptor.FieldList.size() }))
};

const StructDescriptor<NumImplementedFields> & Type::mDescriptor = Descriptor;
} // namespace StructC

namespace CommandA {
constexpr std::array<TypeOffsetInfo, NumImplementedFields> InitializeOffsets()
{
    using result_t           = ::std::array<TypeOffsetInfo, NumImplementedFields>;
    result_t r               = {};
    const result_t & const_r = r;

    uint32_t i = 0;

    const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, a), sizeof(Type::a) };
    const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, b), sizeof(Type::b) };
    const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, c), sizeof(Type::c) };
    const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, d), sizeof(uint8_t) };

    return r;
}

constexpr std::array<TypeOffsetInfo, NumImplementedFields> Offsets = InitializeOffsets();

const StructDescriptor<NumImplementedFields> Descriptor = {
    .FieldList = PopulateFieldDescriptors<NumImplementedFields, ArraySize(_Schema)>(
        _Schema, Offsets,
        chip::Span<const CompactFieldDescriptor>({ StructA::Descriptor.FieldList.data(), StructA::Descriptor.FieldList.size() }))
};

const StructDescriptor<NumImplementedFields> & Type::mDescriptor = Descriptor;
} // namespace CommandA

namespace CommandB {
constexpr std::array<TypeOffsetInfo, NumImplementedFields> InitializeOffsets()
{
    using result_t           = ::std::array<TypeOffsetInfo, NumImplementedFields>;
    result_t r               = {};
    const result_t & const_r = r;

    uint32_t i = 0;

    const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, a), sizeof(Type::a) };
    const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, b), sizeof(Type::b) };
    const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, c), sizeof(Type::c) };
    const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, d), sizeof(uint8_t) };
    const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, e), sizeof(StructA::Type) };

    return r;
}

constexpr std::array<TypeOffsetInfo, NumImplementedFields> Offsets = InitializeOffsets();

const StructDescriptor<NumImplementedFields> Descriptor = {
    .FieldList = PopulateFieldDescriptors<NumImplementedFields, ArraySize(_Schema)>(
        _Schema, Offsets,
        chip::Span<const CompactFieldDescriptor>({ StructA::Descriptor.FieldList.data(), StructA::Descriptor.FieldList.size() }),
        chip::Span<const CompactFieldDescriptor>({ StructA::Descriptor.FieldList.data(), StructA::Descriptor.FieldList.size() }))
};

const StructDescriptor<NumImplementedFields> & Type::mDescriptor = Descriptor;
} // namespace CommandB

namespace Attributes {
constexpr std::array<TypeOffsetInfo, NumImplementedFields> InitializeOffsets()
{
    using result_t           = ::std::array<TypeOffsetInfo, NumImplementedFields>;
    result_t r               = {};
    const result_t & const_r = r;

    uint32_t i = 0;

    const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, a), sizeof(Type::a) };
    const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, b), sizeof(Type::b) };

    if (IsImplemented(FieldC))
    {
        const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, c), sizeof(Type::c) };
    }

    if (IsImplemented(FieldD))
    {
        const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, d), sizeof(Type::d) };
    }

    if (IsImplemented(FieldE))
    {
        const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, e), sizeof(uint8_t) };
    }

    const_cast<typename result_t::reference>(const_r[i++]) = { offsetof(struct Type, f), sizeof(Type::f) };

    return r;
}

constexpr std::array<TypeOffsetInfo, NumImplementedFields> Offsets = InitializeOffsets();

const StructDescriptor<NumImplementedFields> Descriptor = {
    .FieldList = PopulateFieldDescriptors<NumImplementedFields, ArraySize(_Schema)>(
        _Schema, Offsets,
        chip::Span<const CompactFieldDescriptor>({ StructB::Descriptor.FieldList.data(), StructB::Descriptor.FieldList.size() }))
};

const StructDescriptor<NumImplementedFields> & Type::mDescriptor = Descriptor;
} // namespace Attributes
} // namespace TestCluster
} // namespace Cluster
} // namespace app
} // namespace chip
