#include "NetworkCommissioningCluster-Gen.h"

namespace chip {
namespace app {
namespace Cluster {
namespace NetworkCommissioningCluster {
    namespace AddThreadNetworkCommand {
        constexpr std::array<TypeOffsetInfo,NumImplementedFields> InitializeOffsets() {
            using result_t = ::std::array<TypeOffsetInfo, NumImplementedFields>;
            result_t r = {};
            const result_t& const_r = r;

            uint32_t i = 0;

            const_cast<typename result_t::reference>(const_r[i++]) = {offsetof(struct Type, OperationalDataSet), sizeof(uint8_t)};
            const_cast<typename result_t::reference>(const_r[i++]) = {offsetof(struct Type, Breadcrumb), sizeof(Type::Breadcrumb)};
            const_cast<typename result_t::reference>(const_r[i++]) = {offsetof(struct Type, TimeoutMs), sizeof(Type::TimeoutMs)};

            return r;
        }

        constexpr std::array<TypeOffsetInfo, NumImplementedFields> Offsets = InitializeOffsets();

        const StructDescriptor<NumImplementedFields> Descriptor = {
            .FieldList = PopulateFieldDescriptors<NumImplementedFields, ArraySize(_Schema)>(_Schema, Offsets)
        };

        const StructDescriptor<NumImplementedFields>& Type::mDescriptor = Descriptor;
    }

    namespace AddWifiNetworkCommand {
        constexpr std::array<TypeOffsetInfo,NumImplementedFields> InitializeOffsets() {
            using result_t = ::std::array<TypeOffsetInfo, NumImplementedFields>;
            result_t r = {};
            const result_t& const_r = r;

            uint32_t i = 0;

            const_cast<typename result_t::reference>(const_r[i++]) = {offsetof(struct Type, Ssid), sizeof(uint8_t)};
            const_cast<typename result_t::reference>(const_r[i++]) = {offsetof(struct Type, Credentials), sizeof(uint8_t)};
            const_cast<typename result_t::reference>(const_r[i++]) = {offsetof(struct Type, Breadcrumb), sizeof(Type::Breadcrumb)};
            const_cast<typename result_t::reference>(const_r[i++]) = {offsetof(struct Type, TimeoutMs), sizeof(Type::TimeoutMs)};

            return r;
        }

        constexpr std::array<TypeOffsetInfo, NumImplementedFields> Offsets = InitializeOffsets();

        const StructDescriptor<NumImplementedFields> Descriptor = {
            .FieldList = PopulateFieldDescriptors<NumImplementedFields, ArraySize(_Schema)>(_Schema, Offsets)
        };

        const StructDescriptor<NumImplementedFields>& Type::mDescriptor = Descriptor;
    }

    namespace EnableNetworkCommand {
        constexpr std::array<TypeOffsetInfo,NumImplementedFields> InitializeOffsets() {
            using result_t = ::std::array<TypeOffsetInfo, NumImplementedFields>;
            result_t r = {};
            const result_t& const_r = r;


            uint32_t i = 0;

            const_cast<typename result_t::reference>(const_r[i++]) = {offsetof(struct Type, NetworkId), sizeof(uint8_t)};
            const_cast<typename result_t::reference>(const_r[i++]) = {offsetof(struct Type, Breadcrumb), sizeof(Type::Breadcrumb)};
            const_cast<typename result_t::reference>(const_r[i++]) = {offsetof(struct Type, TimeoutMs), sizeof(Type::TimeoutMs)};

            return r;
        }

        constexpr std::array<TypeOffsetInfo, NumImplementedFields> Offsets = InitializeOffsets();

        const StructDescriptor<NumImplementedFields> Descriptor = {
            .FieldList = PopulateFieldDescriptors<NumImplementedFields, ArraySize(_Schema)>(_Schema, Offsets)
        };

        const StructDescriptor<NumImplementedFields>& Type::mDescriptor = Descriptor;
    }
}
}
}
}
