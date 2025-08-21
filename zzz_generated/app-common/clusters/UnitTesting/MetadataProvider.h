// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster UnitTesting (cluster code: 4294048773/0xFFF1FC05)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/UnitTesting/Ids.h>
#include <clusters/UnitTesting/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::UnitTesting::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::UnitTesting::Attributes;
        switch (attributeId)
        {
        case Boolean::Id:
            return Boolean::kMetadataEntry;
        case Bitmap8::Id:
            return Bitmap8::kMetadataEntry;
        case Bitmap16::Id:
            return Bitmap16::kMetadataEntry;
        case Bitmap32::Id:
            return Bitmap32::kMetadataEntry;
        case Bitmap64::Id:
            return Bitmap64::kMetadataEntry;
        case Int8u::Id:
            return Int8u::kMetadataEntry;
        case Int16u::Id:
            return Int16u::kMetadataEntry;
        case Int24u::Id:
            return Int24u::kMetadataEntry;
        case Int32u::Id:
            return Int32u::kMetadataEntry;
        case Int40u::Id:
            return Int40u::kMetadataEntry;
        case Int48u::Id:
            return Int48u::kMetadataEntry;
        case Int56u::Id:
            return Int56u::kMetadataEntry;
        case Int64u::Id:
            return Int64u::kMetadataEntry;
        case Int8s::Id:
            return Int8s::kMetadataEntry;
        case Int16s::Id:
            return Int16s::kMetadataEntry;
        case Int24s::Id:
            return Int24s::kMetadataEntry;
        case Int32s::Id:
            return Int32s::kMetadataEntry;
        case Int40s::Id:
            return Int40s::kMetadataEntry;
        case Int48s::Id:
            return Int48s::kMetadataEntry;
        case Int56s::Id:
            return Int56s::kMetadataEntry;
        case Int64s::Id:
            return Int64s::kMetadataEntry;
        case Enum8::Id:
            return Enum8::kMetadataEntry;
        case Enum16::Id:
            return Enum16::kMetadataEntry;
        case FloatSingle::Id:
            return FloatSingle::kMetadataEntry;
        case FloatDouble::Id:
            return FloatDouble::kMetadataEntry;
        case OctetString::Id:
            return OctetString::kMetadataEntry;
        case ListInt8u::Id:
            return ListInt8u::kMetadataEntry;
        case ListOctetString::Id:
            return ListOctetString::kMetadataEntry;
        case ListStructOctetString::Id:
            return ListStructOctetString::kMetadataEntry;
        case LongOctetString::Id:
            return LongOctetString::kMetadataEntry;
        case CharString::Id:
            return CharString::kMetadataEntry;
        case LongCharString::Id:
            return LongCharString::kMetadataEntry;
        case EpochUs::Id:
            return EpochUs::kMetadataEntry;
        case EpochS::Id:
            return EpochS::kMetadataEntry;
        case VendorId::Id:
            return VendorId::kMetadataEntry;
        case ListNullablesAndOptionalsStruct::Id:
            return ListNullablesAndOptionalsStruct::kMetadataEntry;
        case EnumAttr::Id:
            return EnumAttr::kMetadataEntry;
        case StructAttr::Id:
            return StructAttr::kMetadataEntry;
        case RangeRestrictedInt8u::Id:
            return RangeRestrictedInt8u::kMetadataEntry;
        case RangeRestrictedInt8s::Id:
            return RangeRestrictedInt8s::kMetadataEntry;
        case RangeRestrictedInt16u::Id:
            return RangeRestrictedInt16u::kMetadataEntry;
        case RangeRestrictedInt16s::Id:
            return RangeRestrictedInt16s::kMetadataEntry;
        case ListLongOctetString::Id:
            return ListLongOctetString::kMetadataEntry;
        case ListFabricScoped::Id:
            return ListFabricScoped::kMetadataEntry;
        case TimedWriteBoolean::Id:
            return TimedWriteBoolean::kMetadataEntry;
        case GeneralErrorBoolean::Id:
            return GeneralErrorBoolean::kMetadataEntry;
        case ClusterErrorBoolean::Id:
            return ClusterErrorBoolean::kMetadataEntry;
        case GlobalEnum::Id:
            return GlobalEnum::kMetadataEntry;
        case GlobalStruct::Id:
            return GlobalStruct::kMetadataEntry;
        case UnsupportedAttributeRequiringAdminPrivilege::Id:
            return UnsupportedAttributeRequiringAdminPrivilege::kMetadataEntry;
        case Unsupported::Id:
            return Unsupported::kMetadataEntry;
        case ReadFailureCode::Id:
            return ReadFailureCode::kMetadataEntry;
        case FailureInt32U::Id:
            return FailureInt32U::kMetadataEntry;
        case NullableBoolean::Id:
            return NullableBoolean::kMetadataEntry;
        case NullableBitmap8::Id:
            return NullableBitmap8::kMetadataEntry;
        case NullableBitmap16::Id:
            return NullableBitmap16::kMetadataEntry;
        case NullableBitmap32::Id:
            return NullableBitmap32::kMetadataEntry;
        case NullableBitmap64::Id:
            return NullableBitmap64::kMetadataEntry;
        case NullableInt8u::Id:
            return NullableInt8u::kMetadataEntry;
        case NullableInt16u::Id:
            return NullableInt16u::kMetadataEntry;
        case NullableInt24u::Id:
            return NullableInt24u::kMetadataEntry;
        case NullableInt32u::Id:
            return NullableInt32u::kMetadataEntry;
        case NullableInt40u::Id:
            return NullableInt40u::kMetadataEntry;
        case NullableInt48u::Id:
            return NullableInt48u::kMetadataEntry;
        case NullableInt56u::Id:
            return NullableInt56u::kMetadataEntry;
        case NullableInt64u::Id:
            return NullableInt64u::kMetadataEntry;
        case NullableInt8s::Id:
            return NullableInt8s::kMetadataEntry;
        case NullableInt16s::Id:
            return NullableInt16s::kMetadataEntry;
        case NullableInt24s::Id:
            return NullableInt24s::kMetadataEntry;
        case NullableInt32s::Id:
            return NullableInt32s::kMetadataEntry;
        case NullableInt40s::Id:
            return NullableInt40s::kMetadataEntry;
        case NullableInt48s::Id:
            return NullableInt48s::kMetadataEntry;
        case NullableInt56s::Id:
            return NullableInt56s::kMetadataEntry;
        case NullableInt64s::Id:
            return NullableInt64s::kMetadataEntry;
        case NullableEnum8::Id:
            return NullableEnum8::kMetadataEntry;
        case NullableEnum16::Id:
            return NullableEnum16::kMetadataEntry;
        case NullableFloatSingle::Id:
            return NullableFloatSingle::kMetadataEntry;
        case NullableFloatDouble::Id:
            return NullableFloatDouble::kMetadataEntry;
        case NullableOctetString::Id:
            return NullableOctetString::kMetadataEntry;
        case NullableCharString::Id:
            return NullableCharString::kMetadataEntry;
        case NullableEnumAttr::Id:
            return NullableEnumAttr::kMetadataEntry;
        case NullableStruct::Id:
            return NullableStruct::kMetadataEntry;
        case NullableRangeRestrictedInt8u::Id:
            return NullableRangeRestrictedInt8u::kMetadataEntry;
        case NullableRangeRestrictedInt8s::Id:
            return NullableRangeRestrictedInt8s::kMetadataEntry;
        case NullableRangeRestrictedInt16u::Id:
            return NullableRangeRestrictedInt16u::kMetadataEntry;
        case NullableRangeRestrictedInt16s::Id:
            return NullableRangeRestrictedInt16s::kMetadataEntry;
        case WriteOnlyInt8u::Id:
            return WriteOnlyInt8u::kMetadataEntry;
        case NullableGlobalEnum::Id:
            return NullableGlobalEnum::kMetadataEntry;
        case NullableGlobalStruct::Id:
            return NullableGlobalStruct::kMetadataEntry;
        case MeiInt8u::Id:
            return MeiInt8u::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::UnitTesting::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::UnitTesting::Commands;
        switch (commandId)
        {
        case Test::Id:
            return Test::kMetadataEntry;
        case TestNotHandled::Id:
            return TestNotHandled::kMetadataEntry;
        case TestSpecific::Id:
            return TestSpecific::kMetadataEntry;
        case TestUnknownCommand::Id:
            return TestUnknownCommand::kMetadataEntry;
        case TestAddArguments::Id:
            return TestAddArguments::kMetadataEntry;
        case TestSimpleArgumentRequest::Id:
            return TestSimpleArgumentRequest::kMetadataEntry;
        case TestStructArrayArgumentRequest::Id:
            return TestStructArrayArgumentRequest::kMetadataEntry;
        case TestStructArgumentRequest::Id:
            return TestStructArgumentRequest::kMetadataEntry;
        case TestNestedStructArgumentRequest::Id:
            return TestNestedStructArgumentRequest::kMetadataEntry;
        case TestListStructArgumentRequest::Id:
            return TestListStructArgumentRequest::kMetadataEntry;
        case TestListInt8UArgumentRequest::Id:
            return TestListInt8UArgumentRequest::kMetadataEntry;
        case TestNestedStructListArgumentRequest::Id:
            return TestNestedStructListArgumentRequest::kMetadataEntry;
        case TestListNestedStructListArgumentRequest::Id:
            return TestListNestedStructListArgumentRequest::kMetadataEntry;
        case TestListInt8UReverseRequest::Id:
            return TestListInt8UReverseRequest::kMetadataEntry;
        case TestEnumsRequest::Id:
            return TestEnumsRequest::kMetadataEntry;
        case TestNullableOptionalRequest::Id:
            return TestNullableOptionalRequest::kMetadataEntry;
        case TestComplexNullableOptionalRequest::Id:
            return TestComplexNullableOptionalRequest::kMetadataEntry;
        case SimpleStructEchoRequest::Id:
            return SimpleStructEchoRequest::kMetadataEntry;
        case TimedInvokeRequest::Id:
            return TimedInvokeRequest::kMetadataEntry;
        case TestSimpleOptionalArgumentRequest::Id:
            return TestSimpleOptionalArgumentRequest::kMetadataEntry;
        case TestEmitTestEventRequest::Id:
            return TestEmitTestEventRequest::kMetadataEntry;
        case TestEmitTestFabricScopedEventRequest::Id:
            return TestEmitTestFabricScopedEventRequest::kMetadataEntry;
        case TestBatchHelperRequest::Id:
            return TestBatchHelperRequest::kMetadataEntry;
        case TestSecondBatchHelperRequest::Id:
            return TestSecondBatchHelperRequest::kMetadataEntry;
        case StringEchoRequest::Id:
            return StringEchoRequest::kMetadataEntry;
        case GlobalEchoRequest::Id:
            return GlobalEchoRequest::kMetadataEntry;
        case TestCheckCommandFlags::Id:
            return TestCheckCommandFlags::kMetadataEntry;
        case TestDifferentVendorMeiRequest::Id:
            return TestDifferentVendorMeiRequest::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
