// DO NOT EDIT - Generated file
//
// Application configuration for UnitTesting based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/UnitTesting/AttributeIds.h>
#include <clusters/UnitTesting/CommandIds.h>
#include <clusters/UnitTesting/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace UnitTesting {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint1EnabledAttributes[] = {
    Attributes::Bitmap16::Id,
    Attributes::Bitmap32::Id,
    Attributes::Bitmap64::Id,
    Attributes::Bitmap8::Id,
    Attributes::Boolean::Id,
    Attributes::CharString::Id,
    Attributes::ClusterErrorBoolean::Id,
    Attributes::ClusterRevision::Id,
    Attributes::Enum16::Id,
    Attributes::Enum8::Id,
    Attributes::EnumAttr::Id,
    Attributes::EpochS::Id,
    Attributes::EpochUs::Id,
    Attributes::FailureInt32U::Id,
    Attributes::FeatureMap::Id,
    Attributes::FloatDouble::Id,
    Attributes::FloatSingle::Id,
    Attributes::GeneralErrorBoolean::Id,
    Attributes::GlobalEnum::Id,
    Attributes::GlobalStruct::Id,
    Attributes::Int16s::Id,
    Attributes::Int16u::Id,
    Attributes::Int24s::Id,
    Attributes::Int24u::Id,
    Attributes::Int32s::Id,
    Attributes::Int32u::Id,
    Attributes::Int40s::Id,
    Attributes::Int40u::Id,
    Attributes::Int48s::Id,
    Attributes::Int48u::Id,
    Attributes::Int56s::Id,
    Attributes::Int56u::Id,
    Attributes::Int64s::Id,
    Attributes::Int64u::Id,
    Attributes::Int8s::Id,
    Attributes::Int8u::Id,
    Attributes::ListFabricScoped::Id,
    Attributes::ListInt8u::Id,
    Attributes::ListLongOctetString::Id,
    Attributes::ListNullablesAndOptionalsStruct::Id,
    Attributes::ListOctetString::Id,
    Attributes::ListStructOctetString::Id,
    Attributes::LongCharString::Id,
    Attributes::LongOctetString::Id,
    Attributes::MeiInt8u::Id,
    Attributes::NullableBitmap16::Id,
    Attributes::NullableBitmap32::Id,
    Attributes::NullableBitmap64::Id,
    Attributes::NullableBitmap8::Id,
    Attributes::NullableBoolean::Id,
    Attributes::NullableCharString::Id,
    Attributes::NullableEnum16::Id,
    Attributes::NullableEnum8::Id,
    Attributes::NullableEnumAttr::Id,
    Attributes::NullableFloatDouble::Id,
    Attributes::NullableFloatSingle::Id,
    Attributes::NullableGlobalEnum::Id,
    Attributes::NullableGlobalStruct::Id,
    Attributes::NullableInt16s::Id,
    Attributes::NullableInt16u::Id,
    Attributes::NullableInt24s::Id,
    Attributes::NullableInt24u::Id,
    Attributes::NullableInt32s::Id,
    Attributes::NullableInt32u::Id,
    Attributes::NullableInt40s::Id,
    Attributes::NullableInt40u::Id,
    Attributes::NullableInt48s::Id,
    Attributes::NullableInt48u::Id,
    Attributes::NullableInt56s::Id,
    Attributes::NullableInt56u::Id,
    Attributes::NullableInt64s::Id,
    Attributes::NullableInt64u::Id,
    Attributes::NullableInt8s::Id,
    Attributes::NullableInt8u::Id,
    Attributes::NullableOctetString::Id,
    Attributes::NullableRangeRestrictedInt16s::Id,
    Attributes::NullableRangeRestrictedInt16u::Id,
    Attributes::NullableRangeRestrictedInt8s::Id,
    Attributes::NullableRangeRestrictedInt8u::Id,
    Attributes::NullableStruct::Id,
    Attributes::OctetString::Id,
    Attributes::RangeRestrictedInt16s::Id,
    Attributes::RangeRestrictedInt16u::Id,
    Attributes::RangeRestrictedInt8s::Id,
    Attributes::RangeRestrictedInt8u::Id,
    Attributes::ReadFailureCode::Id,
    Attributes::StructAttr::Id,
    Attributes::TimedWriteBoolean::Id,
    Attributes::VendorId::Id,
    Attributes::WriteOnlyInt8u::Id,
};

inline constexpr CommandId kEndpoint1EnabledCommands[] = {
    Commands::GlobalEchoRequest::Id,
    Commands::GlobalEchoResponse::Id,
    Commands::SimpleStructEchoRequest::Id,
    Commands::SimpleStructResponse::Id,
    Commands::StringEchoRequest::Id,
    Commands::StringEchoResponse::Id,
    Commands::Test::Id,
    Commands::TestAddArguments::Id,
    Commands::TestAddArgumentsResponse::Id,
    Commands::TestBatchHelperRequest::Id,
    Commands::TestDifferentVendorMeiRequest::Id,
    Commands::TestDifferentVendorMeiResponse::Id,
    Commands::TestEmitTestEventRequest::Id,
    Commands::TestEmitTestEventResponse::Id,
    Commands::TestEmitTestFabricScopedEventRequest::Id,
    Commands::TestEmitTestFabricScopedEventResponse::Id,
    Commands::TestEnumsRequest::Id,
    Commands::TestEnumsResponse::Id,
    Commands::TestListInt8UArgumentRequest::Id,
    Commands::TestListInt8UReverseRequest::Id,
    Commands::TestListInt8UReverseResponse::Id,
    Commands::TestListNestedStructListArgumentRequest::Id,
    Commands::TestListStructArgumentRequest::Id,
    Commands::TestNestedStructArgumentRequest::Id,
    Commands::TestNestedStructListArgumentRequest::Id,
    Commands::TestNotHandled::Id,
    Commands::TestNullableOptionalRequest::Id,
    Commands::TestNullableOptionalResponse::Id,
    Commands::TestSecondBatchHelperRequest::Id,
    Commands::TestSimpleOptionalArgumentRequest::Id,
    Commands::TestSpecific::Id,
    Commands::TestSpecificResponse::Id,
    Commands::TestStructArgumentRequest::Id,
    Commands::TimedInvokeRequest::Id,
};

} // namespace detail

using FeatureBitmapType = Clusters::StaticApplicationConfig::NoFeatureFlagsDefined;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 1,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint1EnabledAttributes),
        .enabledCommands = Span<const CommandId>(detail::kEndpoint1EnabledCommands),
    },
} };

// If a specific attribute is supported at all across all endpoint static instantiations
inline constexpr bool IsAttributeEnabledOnSomeEndpoint(AttributeId attributeId) {
  switch (attributeId) {
    case Attributes::Bitmap16::Id:
    case Attributes::Bitmap32::Id:
    case Attributes::Bitmap64::Id:
    case Attributes::Bitmap8::Id:
    case Attributes::Boolean::Id:
    case Attributes::CharString::Id:
    case Attributes::ClusterErrorBoolean::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::Enum16::Id:
    case Attributes::Enum8::Id:
    case Attributes::EnumAttr::Id:
    case Attributes::EpochS::Id:
    case Attributes::EpochUs::Id:
    case Attributes::FailureInt32U::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::FloatDouble::Id:
    case Attributes::FloatSingle::Id:
    case Attributes::GeneralErrorBoolean::Id:
    case Attributes::GlobalEnum::Id:
    case Attributes::GlobalStruct::Id:
    case Attributes::Int16s::Id:
    case Attributes::Int16u::Id:
    case Attributes::Int24s::Id:
    case Attributes::Int24u::Id:
    case Attributes::Int32s::Id:
    case Attributes::Int32u::Id:
    case Attributes::Int40s::Id:
    case Attributes::Int40u::Id:
    case Attributes::Int48s::Id:
    case Attributes::Int48u::Id:
    case Attributes::Int56s::Id:
    case Attributes::Int56u::Id:
    case Attributes::Int64s::Id:
    case Attributes::Int64u::Id:
    case Attributes::Int8s::Id:
    case Attributes::Int8u::Id:
    case Attributes::ListFabricScoped::Id:
    case Attributes::ListInt8u::Id:
    case Attributes::ListLongOctetString::Id:
    case Attributes::ListNullablesAndOptionalsStruct::Id:
    case Attributes::ListOctetString::Id:
    case Attributes::ListStructOctetString::Id:
    case Attributes::LongCharString::Id:
    case Attributes::LongOctetString::Id:
    case Attributes::MeiInt8u::Id:
    case Attributes::NullableBitmap16::Id:
    case Attributes::NullableBitmap32::Id:
    case Attributes::NullableBitmap64::Id:
    case Attributes::NullableBitmap8::Id:
    case Attributes::NullableBoolean::Id:
    case Attributes::NullableCharString::Id:
    case Attributes::NullableEnum16::Id:
    case Attributes::NullableEnum8::Id:
    case Attributes::NullableEnumAttr::Id:
    case Attributes::NullableFloatDouble::Id:
    case Attributes::NullableFloatSingle::Id:
    case Attributes::NullableGlobalEnum::Id:
    case Attributes::NullableGlobalStruct::Id:
    case Attributes::NullableInt16s::Id:
    case Attributes::NullableInt16u::Id:
    case Attributes::NullableInt24s::Id:
    case Attributes::NullableInt24u::Id:
    case Attributes::NullableInt32s::Id:
    case Attributes::NullableInt32u::Id:
    case Attributes::NullableInt40s::Id:
    case Attributes::NullableInt40u::Id:
    case Attributes::NullableInt48s::Id:
    case Attributes::NullableInt48u::Id:
    case Attributes::NullableInt56s::Id:
    case Attributes::NullableInt56u::Id:
    case Attributes::NullableInt64s::Id:
    case Attributes::NullableInt64u::Id:
    case Attributes::NullableInt8s::Id:
    case Attributes::NullableInt8u::Id:
    case Attributes::NullableOctetString::Id:
    case Attributes::NullableRangeRestrictedInt16s::Id:
    case Attributes::NullableRangeRestrictedInt16u::Id:
    case Attributes::NullableRangeRestrictedInt8s::Id:
    case Attributes::NullableRangeRestrictedInt8u::Id:
    case Attributes::NullableStruct::Id:
    case Attributes::OctetString::Id:
    case Attributes::RangeRestrictedInt16s::Id:
    case Attributes::RangeRestrictedInt16u::Id:
    case Attributes::RangeRestrictedInt8s::Id:
    case Attributes::RangeRestrictedInt8u::Id:
    case Attributes::ReadFailureCode::Id:
    case Attributes::StructAttr::Id:
    case Attributes::TimedWriteBoolean::Id:
    case Attributes::VendorId::Id:
    case Attributes::WriteOnlyInt8u::Id:
      return true;
    default:
      return false;
  }
}

// If a specific command is supported at all across all endpoint static instantiations
inline constexpr bool IsCommandEnabledOnSomeEndpoint(CommandId commandId) {
  switch (commandId) {
    case Commands::GlobalEchoRequest::Id:
    case Commands::GlobalEchoResponse::Id:
    case Commands::SimpleStructEchoRequest::Id:
    case Commands::SimpleStructResponse::Id:
    case Commands::StringEchoRequest::Id:
    case Commands::StringEchoResponse::Id:
    case Commands::Test::Id:
    case Commands::TestAddArguments::Id:
    case Commands::TestAddArgumentsResponse::Id:
    case Commands::TestBatchHelperRequest::Id:
    case Commands::TestDifferentVendorMeiRequest::Id:
    case Commands::TestDifferentVendorMeiResponse::Id:
    case Commands::TestEmitTestEventRequest::Id:
    case Commands::TestEmitTestEventResponse::Id:
    case Commands::TestEmitTestFabricScopedEventRequest::Id:
    case Commands::TestEmitTestFabricScopedEventResponse::Id:
    case Commands::TestEnumsRequest::Id:
    case Commands::TestEnumsResponse::Id:
    case Commands::TestListInt8UArgumentRequest::Id:
    case Commands::TestListInt8UReverseRequest::Id:
    case Commands::TestListInt8UReverseResponse::Id:
    case Commands::TestListNestedStructListArgumentRequest::Id:
    case Commands::TestListStructArgumentRequest::Id:
    case Commands::TestNestedStructArgumentRequest::Id:
    case Commands::TestNestedStructListArgumentRequest::Id:
    case Commands::TestNotHandled::Id:
    case Commands::TestNullableOptionalRequest::Id:
    case Commands::TestNullableOptionalResponse::Id:
    case Commands::TestSecondBatchHelperRequest::Id:
    case Commands::TestSimpleOptionalArgumentRequest::Id:
    case Commands::TestSpecific::Id:
    case Commands::TestSpecificResponse::Id:
    case Commands::TestStructArgumentRequest::Id:
    case Commands::TimedInvokeRequest::Id:
      return true;
    default:
      return false;
  }
}

} // namespace StaticApplicationConfig
} // namespace UnitTesting
} // namespace Clusters
} // namespace app
} // namespace chip

