// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster UnitTesting (cluster code: 4294048773/0xFFF1FC05)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/UnitTesting/UnitTestingIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace UnitTesting {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kBooleanEntry = {
    .attributeId    = UnitTesting::Attributes::Boolean::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kBitmap8Entry = {
    .attributeId    = UnitTesting::Attributes::Bitmap8::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kBitmap16Entry = {
    .attributeId    = UnitTesting::Attributes::Bitmap16::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kBitmap32Entry = {
    .attributeId    = UnitTesting::Attributes::Bitmap32::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kBitmap64Entry = {
    .attributeId    = UnitTesting::Attributes::Bitmap64::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kInt8uEntry = {
    .attributeId    = UnitTesting::Attributes::Int8u::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kInt16uEntry = {
    .attributeId    = UnitTesting::Attributes::Int16u::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kInt24uEntry = {
    .attributeId    = UnitTesting::Attributes::Int24u::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kInt32uEntry = {
    .attributeId    = UnitTesting::Attributes::Int32u::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kInt40uEntry = {
    .attributeId    = UnitTesting::Attributes::Int40u::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kInt48uEntry = {
    .attributeId    = UnitTesting::Attributes::Int48u::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kInt56uEntry = {
    .attributeId    = UnitTesting::Attributes::Int56u::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kInt64uEntry = {
    .attributeId    = UnitTesting::Attributes::Int64u::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kInt8sEntry = {
    .attributeId    = UnitTesting::Attributes::Int8s::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kInt16sEntry = {
    .attributeId    = UnitTesting::Attributes::Int16s::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kInt24sEntry = {
    .attributeId    = UnitTesting::Attributes::Int24s::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kInt32sEntry = {
    .attributeId    = UnitTesting::Attributes::Int32s::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kInt40sEntry = {
    .attributeId    = UnitTesting::Attributes::Int40s::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kInt48sEntry = {
    .attributeId    = UnitTesting::Attributes::Int48s::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kInt56sEntry = {
    .attributeId    = UnitTesting::Attributes::Int56s::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kInt64sEntry = {
    .attributeId    = UnitTesting::Attributes::Int64s::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kEnum8Entry = {
    .attributeId    = UnitTesting::Attributes::Enum8::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kEnum16Entry = {
    .attributeId    = UnitTesting::Attributes::Enum16::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kFloatSingleEntry = {
    .attributeId    = UnitTesting::Attributes::FloatSingle::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kFloatDoubleEntry = {
    .attributeId    = UnitTesting::Attributes::FloatDouble::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kOctetStringEntry = {
    .attributeId    = UnitTesting::Attributes::OctetString::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kListInt8uEntry = {
    .attributeId    = UnitTesting::Attributes::ListInt8u::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kListOctetStringEntry = {
    .attributeId    = UnitTesting::Attributes::ListOctetString::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kListStructOctetStringEntry = {
    .attributeId    = UnitTesting::Attributes::ListStructOctetString::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kLongOctetStringEntry = {
    .attributeId    = UnitTesting::Attributes::LongOctetString::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kCharStringEntry = {
    .attributeId    = UnitTesting::Attributes::CharString::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kLongCharStringEntry = {
    .attributeId    = UnitTesting::Attributes::LongCharString::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kEpochUsEntry = {
    .attributeId    = UnitTesting::Attributes::EpochUs::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kEpochSEntry = {
    .attributeId    = UnitTesting::Attributes::EpochS::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kVendorIdEntry = {
    .attributeId    = UnitTesting::Attributes::VendorId::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kListNullablesAndOptionalsStructEntry = {
    .attributeId    = UnitTesting::Attributes::ListNullablesAndOptionalsStruct::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kEnumAttrEntry = {
    .attributeId    = UnitTesting::Attributes::EnumAttr::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kStructAttrEntry = {
    .attributeId    = UnitTesting::Attributes::StructAttr::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kRangeRestrictedInt8uEntry = {
    .attributeId    = UnitTesting::Attributes::RangeRestrictedInt8u::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kRangeRestrictedInt8sEntry = {
    .attributeId    = UnitTesting::Attributes::RangeRestrictedInt8s::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kRangeRestrictedInt16uEntry = {
    .attributeId    = UnitTesting::Attributes::RangeRestrictedInt16u::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kRangeRestrictedInt16sEntry = {
    .attributeId    = UnitTesting::Attributes::RangeRestrictedInt16s::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kListLongOctetStringEntry = {
    .attributeId    = UnitTesting::Attributes::ListLongOctetString::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kListFabricScopedEntry = {
    .attributeId    = UnitTesting::Attributes::ListFabricScoped::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kTimedWriteBooleanEntry = {
    .attributeId    = UnitTesting::Attributes::TimedWriteBoolean::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kTimed },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kGeneralErrorBooleanEntry = {
    .attributeId    = UnitTesting::Attributes::GeneralErrorBoolean::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kClusterErrorBooleanEntry = {
    .attributeId    = UnitTesting::Attributes::ClusterErrorBoolean::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kGlobalEnumEntry = {
    .attributeId    = UnitTesting::Attributes::GlobalEnum::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kGlobalStructEntry = {
    .attributeId    = UnitTesting::Attributes::GlobalStruct::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kUnsupportedEntry = {
    .attributeId    = UnitTesting::Attributes::Unsupported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kReadFailureCodeEntry = {
    .attributeId    = UnitTesting::Attributes::ReadFailureCode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kFailureInt32UEntry = {
    .attributeId    = UnitTesting::Attributes::FailureInt32U::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableBooleanEntry = {
    .attributeId    = UnitTesting::Attributes::NullableBoolean::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableBitmap8Entry = {
    .attributeId    = UnitTesting::Attributes::NullableBitmap8::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableBitmap16Entry = {
    .attributeId    = UnitTesting::Attributes::NullableBitmap16::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableBitmap32Entry = {
    .attributeId    = UnitTesting::Attributes::NullableBitmap32::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableBitmap64Entry = {
    .attributeId    = UnitTesting::Attributes::NullableBitmap64::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableInt8uEntry = {
    .attributeId    = UnitTesting::Attributes::NullableInt8u::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableInt16uEntry = {
    .attributeId    = UnitTesting::Attributes::NullableInt16u::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableInt24uEntry = {
    .attributeId    = UnitTesting::Attributes::NullableInt24u::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableInt32uEntry = {
    .attributeId    = UnitTesting::Attributes::NullableInt32u::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableInt40uEntry = {
    .attributeId    = UnitTesting::Attributes::NullableInt40u::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableInt48uEntry = {
    .attributeId    = UnitTesting::Attributes::NullableInt48u::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableInt56uEntry = {
    .attributeId    = UnitTesting::Attributes::NullableInt56u::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableInt64uEntry = {
    .attributeId    = UnitTesting::Attributes::NullableInt64u::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableInt8sEntry = {
    .attributeId    = UnitTesting::Attributes::NullableInt8s::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableInt16sEntry = {
    .attributeId    = UnitTesting::Attributes::NullableInt16s::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableInt24sEntry = {
    .attributeId    = UnitTesting::Attributes::NullableInt24s::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableInt32sEntry = {
    .attributeId    = UnitTesting::Attributes::NullableInt32s::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableInt40sEntry = {
    .attributeId    = UnitTesting::Attributes::NullableInt40s::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableInt48sEntry = {
    .attributeId    = UnitTesting::Attributes::NullableInt48s::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableInt56sEntry = {
    .attributeId    = UnitTesting::Attributes::NullableInt56s::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableInt64sEntry = {
    .attributeId    = UnitTesting::Attributes::NullableInt64s::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableEnum8Entry = {
    .attributeId    = UnitTesting::Attributes::NullableEnum8::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableEnum16Entry = {
    .attributeId    = UnitTesting::Attributes::NullableEnum16::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableFloatSingleEntry = {
    .attributeId    = UnitTesting::Attributes::NullableFloatSingle::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableFloatDoubleEntry = {
    .attributeId    = UnitTesting::Attributes::NullableFloatDouble::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableOctetStringEntry = {
    .attributeId    = UnitTesting::Attributes::NullableOctetString::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableCharStringEntry = {
    .attributeId    = UnitTesting::Attributes::NullableCharString::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableEnumAttrEntry = {
    .attributeId    = UnitTesting::Attributes::NullableEnumAttr::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableStructEntry = {
    .attributeId    = UnitTesting::Attributes::NullableStruct::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableRangeRestrictedInt8uEntry = {
    .attributeId    = UnitTesting::Attributes::NullableRangeRestrictedInt8u::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableRangeRestrictedInt8sEntry = {
    .attributeId    = UnitTesting::Attributes::NullableRangeRestrictedInt8s::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableRangeRestrictedInt16uEntry = {
    .attributeId    = UnitTesting::Attributes::NullableRangeRestrictedInt16u::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableRangeRestrictedInt16sEntry = {
    .attributeId    = UnitTesting::Attributes::NullableRangeRestrictedInt16s::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kWriteOnlyInt8uEntry = {
    .attributeId    = UnitTesting::Attributes::WriteOnlyInt8u::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableGlobalEnumEntry = {
    .attributeId    = UnitTesting::Attributes::NullableGlobalEnum::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kNullableGlobalStructEntry = {
    .attributeId    = UnitTesting::Attributes::NullableGlobalStruct::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kMeiInt8uEntry = {
    .attributeId    = UnitTesting::Attributes::MeiInt8u::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kTestEntry = {
    .commandId       = UnitTesting::Commands::Test::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kTestNotHandledEntry = {
    .commandId       = UnitTesting::Commands::TestNotHandled::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kTestSpecificEntry = {
    .commandId       = UnitTesting::Commands::TestSpecific::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kTestUnknownCommandEntry = {
    .commandId       = UnitTesting::Commands::TestUnknownCommand::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kTestAddArgumentsEntry = {
    .commandId       = UnitTesting::Commands::TestAddArguments::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kTestSimpleArgumentRequestEntry = {
    .commandId       = UnitTesting::Commands::TestSimpleArgumentRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kTestStructArrayArgumentRequestEntry = {
    .commandId       = UnitTesting::Commands::TestStructArrayArgumentRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kTestStructArgumentRequestEntry = {
    .commandId       = UnitTesting::Commands::TestStructArgumentRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kTestNestedStructArgumentRequestEntry = {
    .commandId       = UnitTesting::Commands::TestNestedStructArgumentRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kTestListStructArgumentRequestEntry = {
    .commandId       = UnitTesting::Commands::TestListStructArgumentRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kTestListInt8UArgumentRequestEntry = {
    .commandId       = UnitTesting::Commands::TestListInt8UArgumentRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kTestNestedStructListArgumentRequestEntry = {
    .commandId       = UnitTesting::Commands::TestNestedStructListArgumentRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kTestListNestedStructListArgumentRequestEntry = {
    .commandId       = UnitTesting::Commands::TestListNestedStructListArgumentRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kTestListInt8UReverseRequestEntry = {
    .commandId       = UnitTesting::Commands::TestListInt8UReverseRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kTestEnumsRequestEntry = {
    .commandId       = UnitTesting::Commands::TestEnumsRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kTestNullableOptionalRequestEntry = {
    .commandId       = UnitTesting::Commands::TestNullableOptionalRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kTestComplexNullableOptionalRequestEntry = {
    .commandId       = UnitTesting::Commands::TestComplexNullableOptionalRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kSimpleStructEchoRequestEntry = {
    .commandId       = UnitTesting::Commands::SimpleStructEchoRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kTimedInvokeRequestEntry = {
    .commandId       = UnitTesting::Commands::TimedInvokeRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kTimed },
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kTestSimpleOptionalArgumentRequestEntry = {
    .commandId       = UnitTesting::Commands::TestSimpleOptionalArgumentRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kTestEmitTestEventRequestEntry = {
    .commandId       = UnitTesting::Commands::TestEmitTestEventRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kTestEmitTestFabricScopedEventRequestEntry = {
    .commandId       = UnitTesting::Commands::TestEmitTestFabricScopedEventRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kTestBatchHelperRequestEntry = {
    .commandId       = UnitTesting::Commands::TestBatchHelperRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kTestSecondBatchHelperRequestEntry = {
    .commandId       = UnitTesting::Commands::TestSecondBatchHelperRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kStringEchoRequestEntry = {
    .commandId       = UnitTesting::Commands::StringEchoRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kGlobalEchoRequestEntry = {
    .commandId       = UnitTesting::Commands::GlobalEchoRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kTestDifferentVendorMeiRequestEntry = {
    .commandId       = UnitTesting::Commands::TestDifferentVendorMeiRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace UnitTesting
} // namespace clusters
} // namespace app
} // namespace chip
