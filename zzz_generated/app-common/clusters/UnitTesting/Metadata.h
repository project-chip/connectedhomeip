// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster UnitTesting (cluster code: 4294048773/0xFFF1FC05)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/UnitTesting/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace UnitTesting {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace Boolean {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Boolean::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Boolean
namespace Bitmap8 {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Bitmap8::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Bitmap8
namespace Bitmap16 {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Bitmap16::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Bitmap16
namespace Bitmap32 {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Bitmap32::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Bitmap32
namespace Bitmap64 {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Bitmap64::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Bitmap64
namespace Int8u {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Int8u::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Int8u
namespace Int16u {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Int16u::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Int16u
namespace Int24u {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Int24u::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Int24u
namespace Int32u {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Int32u::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Int32u
namespace Int40u {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Int40u::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Int40u
namespace Int48u {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Int48u::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Int48u
namespace Int56u {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Int56u::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Int56u
namespace Int64u {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Int64u::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Int64u
namespace Int8s {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Int8s::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Int8s
namespace Int16s {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Int16s::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Int16s
namespace Int24s {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Int24s::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Int24s
namespace Int32s {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Int32s::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Int32s
namespace Int40s {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Int40s::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Int40s
namespace Int48s {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Int48s::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Int48s
namespace Int56s {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Int56s::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Int56s
namespace Int64s {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Int64s::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Int64s
namespace Enum8 {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Enum8::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Enum8
namespace Enum16 {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Enum16::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Enum16
namespace FloatSingle {
inline constexpr DataModel::AttributeEntry kMetadataEntry(FloatSingle::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace FloatSingle
namespace FloatDouble {
inline constexpr DataModel::AttributeEntry kMetadataEntry(FloatDouble::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace FloatDouble
namespace OctetString {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OctetString::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace OctetString
namespace ListInt8u {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ListInt8u::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace ListInt8u
namespace ListOctetString {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ListOctetString::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace ListOctetString
namespace ListStructOctetString {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ListStructOctetString::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace ListStructOctetString
namespace LongOctetString {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LongOctetString::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace LongOctetString
namespace CharString {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CharString::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace CharString
namespace LongCharString {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LongCharString::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace LongCharString
namespace EpochUs {
inline constexpr DataModel::AttributeEntry kMetadataEntry(EpochUs::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace EpochUs
namespace EpochS {
inline constexpr DataModel::AttributeEntry kMetadataEntry(EpochS::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace EpochS
namespace VendorId {
inline constexpr DataModel::AttributeEntry kMetadataEntry(VendorId::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace VendorId
namespace ListNullablesAndOptionalsStruct {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ListNullablesAndOptionalsStruct::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace ListNullablesAndOptionalsStruct
namespace EnumAttr {
inline constexpr DataModel::AttributeEntry kMetadataEntry(EnumAttr::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace EnumAttr
namespace StructAttr {
inline constexpr DataModel::AttributeEntry kMetadataEntry(StructAttr::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace StructAttr
namespace RangeRestrictedInt8u {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RangeRestrictedInt8u::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace RangeRestrictedInt8u
namespace RangeRestrictedInt8s {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RangeRestrictedInt8s::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace RangeRestrictedInt8s
namespace RangeRestrictedInt16u {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RangeRestrictedInt16u::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace RangeRestrictedInt16u
namespace RangeRestrictedInt16s {
inline constexpr DataModel::AttributeEntry kMetadataEntry(RangeRestrictedInt16s::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace RangeRestrictedInt16s
namespace ListLongOctetString {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ListLongOctetString::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace ListLongOctetString
namespace ListFabricScoped {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ListFabricScoped::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace ListFabricScoped
namespace TimedWriteBoolean {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(TimedWriteBoolean::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kTimed),
                   Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace TimedWriteBoolean
namespace GeneralErrorBoolean {
inline constexpr DataModel::AttributeEntry kMetadataEntry(GeneralErrorBoolean::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace GeneralErrorBoolean
namespace ClusterErrorBoolean {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ClusterErrorBoolean::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace ClusterErrorBoolean
namespace GlobalEnum {
inline constexpr DataModel::AttributeEntry kMetadataEntry(GlobalEnum::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace GlobalEnum
namespace GlobalStruct {
inline constexpr DataModel::AttributeEntry kMetadataEntry(GlobalStruct::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace GlobalStruct
namespace UnsupportedAttributeRequiringAdminPrivilege {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UnsupportedAttributeRequiringAdminPrivilege::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kAdminister, Access::Privilege::kAdminister);
} // namespace UnsupportedAttributeRequiringAdminPrivilege
namespace Unsupported {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Unsupported::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace Unsupported
namespace ReadFailureCode {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ReadFailureCode::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace ReadFailureCode
namespace FailureInt32U {
inline constexpr DataModel::AttributeEntry kMetadataEntry(FailureInt32U::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace FailureInt32U
namespace NullableBoolean {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableBoolean::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableBoolean
namespace NullableBitmap8 {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableBitmap8::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableBitmap8
namespace NullableBitmap16 {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableBitmap16::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableBitmap16
namespace NullableBitmap32 {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableBitmap32::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableBitmap32
namespace NullableBitmap64 {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableBitmap64::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableBitmap64
namespace NullableInt8u {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableInt8u::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableInt8u
namespace NullableInt16u {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableInt16u::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableInt16u
namespace NullableInt24u {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableInt24u::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableInt24u
namespace NullableInt32u {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableInt32u::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableInt32u
namespace NullableInt40u {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableInt40u::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableInt40u
namespace NullableInt48u {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableInt48u::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableInt48u
namespace NullableInt56u {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableInt56u::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableInt56u
namespace NullableInt64u {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableInt64u::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableInt64u
namespace NullableInt8s {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableInt8s::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableInt8s
namespace NullableInt16s {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableInt16s::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableInt16s
namespace NullableInt24s {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableInt24s::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableInt24s
namespace NullableInt32s {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableInt32s::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableInt32s
namespace NullableInt40s {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableInt40s::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableInt40s
namespace NullableInt48s {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableInt48s::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableInt48s
namespace NullableInt56s {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableInt56s::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableInt56s
namespace NullableInt64s {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableInt64s::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableInt64s
namespace NullableEnum8 {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableEnum8::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableEnum8
namespace NullableEnum16 {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableEnum16::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableEnum16
namespace NullableFloatSingle {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableFloatSingle::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableFloatSingle
namespace NullableFloatDouble {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableFloatDouble::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableFloatDouble
namespace NullableOctetString {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableOctetString::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableOctetString
namespace NullableCharString {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableCharString::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableCharString
namespace NullableEnumAttr {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableEnumAttr::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableEnumAttr
namespace NullableStruct {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableStruct::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableStruct
namespace NullableRangeRestrictedInt8u {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableRangeRestrictedInt8u::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kOperate);
} // namespace NullableRangeRestrictedInt8u
namespace NullableRangeRestrictedInt8s {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableRangeRestrictedInt8s::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kOperate);
} // namespace NullableRangeRestrictedInt8s
namespace NullableRangeRestrictedInt16u {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableRangeRestrictedInt16u::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kOperate);
} // namespace NullableRangeRestrictedInt16u
namespace NullableRangeRestrictedInt16s {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableRangeRestrictedInt16s::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kOperate);
} // namespace NullableRangeRestrictedInt16s
namespace WriteOnlyInt8u {
inline constexpr DataModel::AttributeEntry kMetadataEntry(WriteOnlyInt8u::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          std::nullopt, Access::Privilege::kOperate);
} // namespace WriteOnlyInt8u
namespace NullableGlobalEnum {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableGlobalEnum::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableGlobalEnum
namespace NullableGlobalStruct {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NullableGlobalStruct::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NullableGlobalStruct
namespace MeiInt8u {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MeiInt8u::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace MeiInt8u
constexpr std::array<DataModel::AttributeEntry, 85> kMandatoryMetadata = {
    Boolean::kMetadataEntry,
    Bitmap8::kMetadataEntry,
    Bitmap16::kMetadataEntry,
    Bitmap32::kMetadataEntry,
    Bitmap64::kMetadataEntry,
    Int8u::kMetadataEntry,
    Int16u::kMetadataEntry,
    Int24u::kMetadataEntry,
    Int32u::kMetadataEntry,
    Int40u::kMetadataEntry,
    Int48u::kMetadataEntry,
    Int56u::kMetadataEntry,
    Int64u::kMetadataEntry,
    Int8s::kMetadataEntry,
    Int16s::kMetadataEntry,
    Int24s::kMetadataEntry,
    Int32s::kMetadataEntry,
    Int40s::kMetadataEntry,
    Int48s::kMetadataEntry,
    Int56s::kMetadataEntry,
    Int64s::kMetadataEntry,
    Enum8::kMetadataEntry,
    Enum16::kMetadataEntry,
    FloatSingle::kMetadataEntry,
    FloatDouble::kMetadataEntry,
    OctetString::kMetadataEntry,
    ListInt8u::kMetadataEntry,
    ListOctetString::kMetadataEntry,
    ListStructOctetString::kMetadataEntry,
    LongOctetString::kMetadataEntry,
    CharString::kMetadataEntry,
    LongCharString::kMetadataEntry,
    EpochUs::kMetadataEntry,
    EpochS::kMetadataEntry,
    VendorId::kMetadataEntry,
    ListNullablesAndOptionalsStruct::kMetadataEntry,
    EnumAttr::kMetadataEntry,
    StructAttr::kMetadataEntry,
    RangeRestrictedInt8u::kMetadataEntry,
    RangeRestrictedInt8s::kMetadataEntry,
    RangeRestrictedInt16u::kMetadataEntry,
    RangeRestrictedInt16s::kMetadataEntry,
    ListLongOctetString::kMetadataEntry,
    ListFabricScoped::kMetadataEntry,
    TimedWriteBoolean::kMetadataEntry,
    GeneralErrorBoolean::kMetadataEntry,
    ClusterErrorBoolean::kMetadataEntry,
    GlobalEnum::kMetadataEntry,
    GlobalStruct::kMetadataEntry,
    NullableBoolean::kMetadataEntry,
    NullableBitmap8::kMetadataEntry,
    NullableBitmap16::kMetadataEntry,
    NullableBitmap32::kMetadataEntry,
    NullableBitmap64::kMetadataEntry,
    NullableInt8u::kMetadataEntry,
    NullableInt16u::kMetadataEntry,
    NullableInt24u::kMetadataEntry,
    NullableInt32u::kMetadataEntry,
    NullableInt40u::kMetadataEntry,
    NullableInt48u::kMetadataEntry,
    NullableInt56u::kMetadataEntry,
    NullableInt64u::kMetadataEntry,
    NullableInt8s::kMetadataEntry,
    NullableInt16s::kMetadataEntry,
    NullableInt24s::kMetadataEntry,
    NullableInt32s::kMetadataEntry,
    NullableInt40s::kMetadataEntry,
    NullableInt48s::kMetadataEntry,
    NullableInt56s::kMetadataEntry,
    NullableInt64s::kMetadataEntry,
    NullableEnum8::kMetadataEntry,
    NullableEnum16::kMetadataEntry,
    NullableFloatSingle::kMetadataEntry,
    NullableFloatDouble::kMetadataEntry,
    NullableOctetString::kMetadataEntry,
    NullableCharString::kMetadataEntry,
    NullableEnumAttr::kMetadataEntry,
    NullableStruct::kMetadataEntry,
    NullableRangeRestrictedInt8u::kMetadataEntry,
    NullableRangeRestrictedInt8s::kMetadataEntry,
    NullableRangeRestrictedInt16u::kMetadataEntry,
    NullableRangeRestrictedInt16s::kMetadataEntry,
    NullableGlobalEnum::kMetadataEntry,
    NullableGlobalStruct::kMetadataEntry,
    MeiInt8u::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {

namespace Test {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Test::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Test
namespace TestNotHandled {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(TestNotHandled::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace TestNotHandled
namespace TestSpecific {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(TestSpecific::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace TestSpecific
namespace TestUnknownCommand {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(TestUnknownCommand::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace TestUnknownCommand
namespace TestAddArguments {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(TestAddArguments::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace TestAddArguments
namespace TestSimpleArgumentRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(TestSimpleArgumentRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace TestSimpleArgumentRequest
namespace TestStructArrayArgumentRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(TestStructArrayArgumentRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace TestStructArrayArgumentRequest
namespace TestStructArgumentRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(TestStructArgumentRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace TestStructArgumentRequest
namespace TestNestedStructArgumentRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(TestNestedStructArgumentRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace TestNestedStructArgumentRequest
namespace TestListStructArgumentRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(TestListStructArgumentRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace TestListStructArgumentRequest
namespace TestListInt8UArgumentRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(TestListInt8UArgumentRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace TestListInt8UArgumentRequest
namespace TestNestedStructListArgumentRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(TestNestedStructListArgumentRequest::Id,
                                                                BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace TestNestedStructListArgumentRequest
namespace TestListNestedStructListArgumentRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(TestListNestedStructListArgumentRequest::Id,
                                                                BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace TestListNestedStructListArgumentRequest
namespace TestListInt8UReverseRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(TestListInt8UReverseRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace TestListInt8UReverseRequest
namespace TestEnumsRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(TestEnumsRequest::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace TestEnumsRequest
namespace TestNullableOptionalRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(TestNullableOptionalRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace TestNullableOptionalRequest
namespace TestComplexNullableOptionalRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(TestComplexNullableOptionalRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace TestComplexNullableOptionalRequest
namespace SimpleStructEchoRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(SimpleStructEchoRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace SimpleStructEchoRequest
namespace TimedInvokeRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(TimedInvokeRequest::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kOperate);
} // namespace TimedInvokeRequest
namespace TestSimpleOptionalArgumentRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(TestSimpleOptionalArgumentRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace TestSimpleOptionalArgumentRequest
namespace TestEmitTestEventRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(TestEmitTestEventRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace TestEmitTestEventRequest
namespace TestEmitTestFabricScopedEventRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(TestEmitTestFabricScopedEventRequest::Id,
                                                                BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace TestEmitTestFabricScopedEventRequest
namespace TestBatchHelperRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(TestBatchHelperRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace TestBatchHelperRequest
namespace TestSecondBatchHelperRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(TestSecondBatchHelperRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace TestSecondBatchHelperRequest
namespace StringEchoRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(StringEchoRequest::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace StringEchoRequest
namespace GlobalEchoRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(GlobalEchoRequest::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace GlobalEchoRequest
namespace TestCheckCommandFlags {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(
    TestCheckCommandFlags::Id,
    BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped, DataModel::CommandQualityFlags::kTimed),
    Access::Privilege::kOperate);
} // namespace TestCheckCommandFlags
namespace TestDifferentVendorMeiRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(TestDifferentVendorMeiRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace TestDifferentVendorMeiRequest

} // namespace Commands

namespace Events {
namespace TestEvent {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace TestEvent
namespace TestFabricScopedEvent {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace TestFabricScopedEvent
namespace TestDifferentVendorMeiEvent {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace TestDifferentVendorMeiEvent

} // namespace Events
} // namespace UnitTesting
} // namespace Clusters
} // namespace app
} // namespace chip
