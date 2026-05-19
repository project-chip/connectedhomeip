/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 * @file Accessors-override.cpp
 *
 * Slim replacement for the full zap-generated Accessors.cpp that includes
 * only the attribute accessor functions needed by the TV Casting App.
 *
 * This file is a hand-maintained subset of:
 *   zzz_generated/app-common/app-common/zap-generated/attributes/Accessors.cpp
 *
 * It covers the 29 casting-relevant clusters:
 *
 *   Commissioning clusters (12):
 *     Identify, Groups, Descriptor, Binding, AccessControl,
 *     BasicInformation, OtaSoftwareUpdateProvider,
 *     OtaSoftwareUpdateRequestor, GeneralCommissioning,
 *     NetworkCommissioning, GeneralDiagnostics,
 *     AdministratorCommissioning
 *
 *   Casting clusters (17):
 *     OperationalCredentials, GroupKeyManagement, FixedLabel, UserLabel,
 *     RelativeHumidityMeasurement, WakeOnLan, Channel, TargetNavigator,
 *     MediaPlayback, MediaInput, LowPower, KeypadInput, ContentLauncher,
 *     AudioOutput, ApplicationLauncher, ApplicationBasic,
 *     ContentAppObserver
 *
 * All other clusters are omitted to reduce binary size (~826 KB savings).
 * Function signatures and implementations are identical to the full
 * generated file for the retained clusters.
 */

#include <app-common/zap-generated/attributes/Accessors.h>

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/attribute-table.h>
#include <app/util/ember-strings.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/support/logging/CHIPLogging.h>
#include <lib/support/odd-sized-integers.h>

namespace chip {
namespace app {
namespace Clusters {
// ---------------------------------------------------------------------------
// Commissioning clusters
// ---------------------------------------------------------------------------

namespace Identify {
namespace Attributes {} // namespace Attributes
} // namespace Identify

namespace Groups {
namespace Attributes {

namespace NameSupport {

Protocols::InteractionModel::Status Get(EndpointId endpoint, chip::BitMask<chip::app::Clusters::Groups::NameSupportBitmap> * value)
{
    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::Groups::NameSupportBitmap>>;
    Traits::StorageType temp;
    uint8_t * readable                         = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status = emberAfReadAttribute(endpoint, Clusters::Groups::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

} // namespace NameSupport

namespace FeatureMap {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint32_t * value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp;
    uint8_t * readable                         = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status = emberAfReadAttribute(endpoint, Clusters::Groups::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

} // namespace FeatureMap

namespace ClusterRevision {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint16_t * value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp;
    uint8_t * readable                         = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status = emberAfReadAttribute(endpoint, Clusters::Groups::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

} // namespace ClusterRevision

} // namespace Attributes
} // namespace Groups

namespace Descriptor {
namespace Attributes {} // namespace Attributes
} // namespace Descriptor

namespace Binding {
namespace Attributes {} // namespace Attributes
} // namespace Binding

namespace AccessControl {
namespace Attributes {} // namespace Attributes
} // namespace AccessControl

namespace BasicInformation {
namespace Attributes {} // namespace Attributes
} // namespace BasicInformation

namespace OtaSoftwareUpdateProvider {
namespace Attributes {

namespace FeatureMap {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint32_t * value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::OtaSoftwareUpdateProvider::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

} // namespace FeatureMap

namespace ClusterRevision {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint16_t * value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::OtaSoftwareUpdateProvider::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

} // namespace ClusterRevision

} // namespace Attributes
} // namespace OtaSoftwareUpdateProvider

namespace OtaSoftwareUpdateRequestor {
namespace Attributes {

namespace UpdatePossible {

Protocols::InteractionModel::Status Get(EndpointId endpoint, bool * value)
{
    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::OtaSoftwareUpdateRequestor::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, bool value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<bool>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::OtaSoftwareUpdateRequestor::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_BOOLEAN_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, bool value)
{
    using Traits = NumericAttributeTraits<bool>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::OtaSoftwareUpdateRequestor::Id, Id, writable, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
}

} // namespace UpdatePossible

namespace UpdateState {

Protocols::InteractionModel::Status Get(EndpointId endpoint,
                                        chip::app::Clusters::OtaSoftwareUpdateRequestor::UpdateStateEnum * value)
{
    using Traits = NumericAttributeTraits<chip::app::Clusters::OtaSoftwareUpdateRequestor::UpdateStateEnum>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::OtaSoftwareUpdateRequestor::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::app::Clusters::OtaSoftwareUpdateRequestor::UpdateStateEnum value,
                                        MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<chip::app::Clusters::OtaSoftwareUpdateRequestor::UpdateStateEnum>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::OtaSoftwareUpdateRequestor::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_ENUM8_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::app::Clusters::OtaSoftwareUpdateRequestor::UpdateStateEnum value)
{
    using Traits = NumericAttributeTraits<chip::app::Clusters::OtaSoftwareUpdateRequestor::UpdateStateEnum>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::OtaSoftwareUpdateRequestor::Id, Id, writable, ZCL_ENUM8_ATTRIBUTE_TYPE);
}

} // namespace UpdateState

namespace UpdateStateProgress {

Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<uint8_t> & value)
{
    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::OtaSoftwareUpdateRequestor::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (Traits::IsNullValue(temp))
    {
        value.SetNull();
    }
    else
    {
        value.SetNonNull() = Traits::StorageToWorking(temp);
    }
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint8_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint8_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ true, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::OtaSoftwareUpdateRequestor::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT8U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint8_t value)
{
    using Traits = NumericAttributeTraits<uint8_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ true, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::OtaSoftwareUpdateRequestor::Id, Id, writable, ZCL_INT8U_ATTRIBUTE_TYPE);
}

Protocols::InteractionModel::Status SetNull(EndpointId endpoint, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType value;
    Traits::SetNull(value);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(value);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::OtaSoftwareUpdateRequestor::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT8U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status SetNull(EndpointId endpoint)
{
    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType value;
    Traits::SetNull(value);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(value);
    return emberAfWriteAttribute(endpoint, Clusters::OtaSoftwareUpdateRequestor::Id, Id, writable, ZCL_INT8U_ATTRIBUTE_TYPE);
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, const chip::app::DataModel::Nullable<uint8_t> & value,
                                        MarkAttributeDirty markDirty)
{
    if (value.IsNull())
    {
        return SetNull(endpoint, markDirty);
    }

    return Set(endpoint, value.Value(), markDirty);
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, const chip::app::DataModel::Nullable<uint8_t> & value)
{
    if (value.IsNull())
    {
        return SetNull(endpoint);
    }

    return Set(endpoint, value.Value());
}

} // namespace UpdateStateProgress

namespace FeatureMap {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint32_t * value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::OtaSoftwareUpdateRequestor::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::OtaSoftwareUpdateRequestor::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_BITMAP32_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::OtaSoftwareUpdateRequestor::Id, Id, writable, ZCL_BITMAP32_ATTRIBUTE_TYPE);
}

} // namespace FeatureMap

namespace ClusterRevision {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint16_t * value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::OtaSoftwareUpdateRequestor::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::OtaSoftwareUpdateRequestor::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT16U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::OtaSoftwareUpdateRequestor::Id, Id, writable, ZCL_INT16U_ATTRIBUTE_TYPE);
}

} // namespace ClusterRevision

} // namespace Attributes
} // namespace OtaSoftwareUpdateRequestor

namespace GeneralCommissioning {
namespace Attributes {

namespace TCAcceptedVersion {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint16_t * value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::GeneralCommissioning::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

} // namespace TCAcceptedVersion

namespace TCMinRequiredVersion {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint16_t * value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::GeneralCommissioning::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

} // namespace TCMinRequiredVersion

namespace TCAcknowledgements {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint16_t * value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::GeneralCommissioning::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

} // namespace TCAcknowledgements

namespace TCAcknowledgementsRequired {

Protocols::InteractionModel::Status Get(EndpointId endpoint, bool * value)
{
    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::GeneralCommissioning::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

} // namespace TCAcknowledgementsRequired

namespace TCUpdateDeadline {

Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<uint32_t> & value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::GeneralCommissioning::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (Traits::IsNullValue(temp))
    {
        value.SetNull();
    }
    else
    {
        value.SetNonNull() = Traits::StorageToWorking(temp);
    }
    return status;
}

} // namespace TCUpdateDeadline

namespace RecoveryIdentifier {

Protocols::InteractionModel::Status Get(EndpointId endpoint, chip::MutableByteSpan & value)
{
    uint8_t zclString[8 + 1];
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::GeneralCommissioning::Id, Id, zclString, sizeof(zclString));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    size_t length = emberAfStringLength(zclString);
    if (length == NumericAttributeTraits<uint8_t>::kNullValue)
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }

    VerifyOrReturnError(value.size() == 8, Protocols::InteractionModel::Status::InvalidDataType);
    memcpy(value.data(), &zclString[1], 8);
    value.reduce_size(length);
    return status;
}

} // namespace RecoveryIdentifier

namespace NetworkRecoveryReason {

Protocols::InteractionModel::Status
Get(EndpointId endpoint, DataModel::Nullable<chip::app::Clusters::GeneralCommissioning::NetworkRecoveryReasonEnum> & value)
{
    using Traits = NumericAttributeTraits<chip::app::Clusters::GeneralCommissioning::NetworkRecoveryReasonEnum>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::GeneralCommissioning::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (Traits::IsNullValue(temp))
    {
        value.SetNull();
    }
    else
    {
        value.SetNonNull() = Traits::StorageToWorking(temp);
    }
    return status;
}

} // namespace NetworkRecoveryReason

namespace IsCommissioningWithoutPower {

Protocols::InteractionModel::Status Get(EndpointId endpoint, bool * value)
{
    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::GeneralCommissioning::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

} // namespace IsCommissioningWithoutPower

} // namespace Attributes
} // namespace GeneralCommissioning

namespace NetworkCommissioning {
namespace Attributes {} // namespace Attributes
} // namespace NetworkCommissioning

namespace GeneralDiagnostics {
namespace Attributes {

namespace TestEventTriggersEnabled {

Protocols::InteractionModel::Status Get(EndpointId endpoint, bool * value)
{
    using Traits = NumericAttributeTraits<bool>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::GeneralDiagnostics::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

} // namespace TestEventTriggersEnabled

namespace FeatureMap {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint32_t * value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::GeneralDiagnostics::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

} // namespace FeatureMap

} // namespace Attributes
} // namespace GeneralDiagnostics

namespace AdministratorCommissioning {
namespace Attributes {

namespace FeatureMap {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint32_t * value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::AdministratorCommissioning::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

} // namespace FeatureMap

} // namespace Attributes
} // namespace AdministratorCommissioning

// ---------------------------------------------------------------------------
// Casting clusters
// ---------------------------------------------------------------------------

namespace OperationalCredentials {
namespace Attributes {} // namespace Attributes
} // namespace OperationalCredentials

namespace GroupKeyManagement {
namespace Attributes {} // namespace Attributes
} // namespace GroupKeyManagement

namespace FixedLabel {
namespace Attributes {} // namespace Attributes
} // namespace FixedLabel

namespace UserLabel {
namespace Attributes {} // namespace Attributes
} // namespace UserLabel

namespace RelativeHumidityMeasurement {
namespace Attributes {

namespace MeasuredValue {

Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<uint16_t> & value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::RelativeHumidityMeasurement::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (Traits::IsNullValue(temp))
    {
        value.SetNull();
    }
    else
    {
        value.SetNonNull() = Traits::StorageToWorking(temp);
    }
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ true, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::RelativeHumidityMeasurement::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT16U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ true, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::RelativeHumidityMeasurement::Id, Id, writable, ZCL_INT16U_ATTRIBUTE_TYPE);
}

Protocols::InteractionModel::Status SetNull(EndpointId endpoint, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType value;
    Traits::SetNull(value);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(value);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::RelativeHumidityMeasurement::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT16U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status SetNull(EndpointId endpoint)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType value;
    Traits::SetNull(value);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(value);
    return emberAfWriteAttribute(endpoint, Clusters::RelativeHumidityMeasurement::Id, Id, writable, ZCL_INT16U_ATTRIBUTE_TYPE);
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, const chip::app::DataModel::Nullable<uint16_t> & value,
                                        MarkAttributeDirty markDirty)
{
    if (value.IsNull())
    {
        return SetNull(endpoint, markDirty);
    }

    return Set(endpoint, value.Value(), markDirty);
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, const chip::app::DataModel::Nullable<uint16_t> & value)
{
    if (value.IsNull())
    {
        return SetNull(endpoint);
    }

    return Set(endpoint, value.Value());
}

} // namespace MeasuredValue

namespace MinMeasuredValue {

Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<uint16_t> & value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::RelativeHumidityMeasurement::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (Traits::IsNullValue(temp))
    {
        value.SetNull();
    }
    else
    {
        value.SetNonNull() = Traits::StorageToWorking(temp);
    }
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ true, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::RelativeHumidityMeasurement::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT16U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ true, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::RelativeHumidityMeasurement::Id, Id, writable, ZCL_INT16U_ATTRIBUTE_TYPE);
}

Protocols::InteractionModel::Status SetNull(EndpointId endpoint, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType value;
    Traits::SetNull(value);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(value);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::RelativeHumidityMeasurement::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT16U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status SetNull(EndpointId endpoint)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType value;
    Traits::SetNull(value);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(value);
    return emberAfWriteAttribute(endpoint, Clusters::RelativeHumidityMeasurement::Id, Id, writable, ZCL_INT16U_ATTRIBUTE_TYPE);
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, const chip::app::DataModel::Nullable<uint16_t> & value,
                                        MarkAttributeDirty markDirty)
{
    if (value.IsNull())
    {
        return SetNull(endpoint, markDirty);
    }

    return Set(endpoint, value.Value(), markDirty);
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, const chip::app::DataModel::Nullable<uint16_t> & value)
{
    if (value.IsNull())
    {
        return SetNull(endpoint);
    }

    return Set(endpoint, value.Value());
}

} // namespace MinMeasuredValue

namespace MaxMeasuredValue {

Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<uint16_t> & value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::RelativeHumidityMeasurement::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (Traits::IsNullValue(temp))
    {
        value.SetNull();
    }
    else
    {
        value.SetNonNull() = Traits::StorageToWorking(temp);
    }
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ true, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::RelativeHumidityMeasurement::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT16U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ true, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::RelativeHumidityMeasurement::Id, Id, writable, ZCL_INT16U_ATTRIBUTE_TYPE);
}

Protocols::InteractionModel::Status SetNull(EndpointId endpoint, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType value;
    Traits::SetNull(value);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(value);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::RelativeHumidityMeasurement::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT16U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status SetNull(EndpointId endpoint)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType value;
    Traits::SetNull(value);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(value);
    return emberAfWriteAttribute(endpoint, Clusters::RelativeHumidityMeasurement::Id, Id, writable, ZCL_INT16U_ATTRIBUTE_TYPE);
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, const chip::app::DataModel::Nullable<uint16_t> & value,
                                        MarkAttributeDirty markDirty)
{
    if (value.IsNull())
    {
        return SetNull(endpoint, markDirty);
    }

    return Set(endpoint, value.Value(), markDirty);
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, const chip::app::DataModel::Nullable<uint16_t> & value)
{
    if (value.IsNull())
    {
        return SetNull(endpoint);
    }

    return Set(endpoint, value.Value());
}

} // namespace MaxMeasuredValue

namespace Tolerance {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint16_t * value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::RelativeHumidityMeasurement::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::RelativeHumidityMeasurement::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT16U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::RelativeHumidityMeasurement::Id, Id, writable, ZCL_INT16U_ATTRIBUTE_TYPE);
}

} // namespace Tolerance

namespace FeatureMap {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint32_t * value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::RelativeHumidityMeasurement::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::RelativeHumidityMeasurement::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_BITMAP32_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::RelativeHumidityMeasurement::Id, Id, writable, ZCL_BITMAP32_ATTRIBUTE_TYPE);
}

} // namespace FeatureMap

namespace ClusterRevision {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint16_t * value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::RelativeHumidityMeasurement::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::RelativeHumidityMeasurement::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT16U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::RelativeHumidityMeasurement::Id, Id, writable, ZCL_INT16U_ATTRIBUTE_TYPE);
}

} // namespace ClusterRevision

} // namespace Attributes
} // namespace RelativeHumidityMeasurement

namespace WakeOnLan {
namespace Attributes {

namespace MACAddress {

Protocols::InteractionModel::Status Get(EndpointId endpoint, chip::MutableCharSpan & value)
{
    uint8_t zclString[12 + 1];
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::WakeOnLan::Id, Id, zclString, sizeof(zclString));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    size_t length = emberAfStringLength(zclString);
    if (length == NumericAttributeTraits<uint8_t>::kNullValue)
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }

    VerifyOrReturnError(value.size() == 12, Protocols::InteractionModel::Status::InvalidDataType);
    memcpy(value.data(), &zclString[1], 12);
    value.reduce_size(length);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::CharSpan value, MarkAttributeDirty markDirty)
{

    static_assert(12 < NumericAttributeTraits<uint8_t>::kNullValue, "value.size() might be too big");
    VerifyOrReturnError(value.size() <= 12, Protocols::InteractionModel::Status::ConstraintError);
    uint8_t zclString[12 + 1];
    auto length = static_cast<uint8_t>(value.size());
    Encoding::Put8(zclString, length);
    memcpy(&zclString[1], value.data(), value.size());
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::WakeOnLan::Id, Id),
                                 EmberAfWriteDataInput(zclString, ZCL_CHAR_STRING_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::CharSpan value)
{

    static_assert(12 < NumericAttributeTraits<uint8_t>::kNullValue, "value.size() might be too big");
    VerifyOrReturnError(value.size() <= 12, Protocols::InteractionModel::Status::ConstraintError);
    uint8_t zclString[12 + 1];
    auto length = static_cast<uint8_t>(value.size());
    Encoding::Put8(zclString, length);
    memcpy(&zclString[1], value.data(), value.size());
    return emberAfWriteAttribute(endpoint, Clusters::WakeOnLan::Id, Id, zclString, ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
}

} // namespace MACAddress

namespace LinkLocalAddress {

Protocols::InteractionModel::Status Get(EndpointId endpoint, chip::MutableByteSpan & value)
{
    uint8_t zclString[16 + 1];
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::WakeOnLan::Id, Id, zclString, sizeof(zclString));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    size_t length = emberAfStringLength(zclString);
    if (length == NumericAttributeTraits<uint8_t>::kNullValue)
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }

    VerifyOrReturnError(value.size() == 16, Protocols::InteractionModel::Status::InvalidDataType);
    memcpy(value.data(), &zclString[1], 16);
    value.reduce_size(length);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::ByteSpan value, MarkAttributeDirty markDirty)
{

    static_assert(16 < NumericAttributeTraits<uint8_t>::kNullValue, "value.size() might be too big");
    VerifyOrReturnError(value.size() <= 16, Protocols::InteractionModel::Status::ConstraintError);
    uint8_t zclString[16 + 1];
    auto length = static_cast<uint8_t>(value.size());
    Encoding::Put8(zclString, length);
    memcpy(&zclString[1], value.data(), value.size());
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::WakeOnLan::Id, Id),
                                 EmberAfWriteDataInput(zclString, ZCL_OCTET_STRING_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::ByteSpan value)
{

    static_assert(16 < NumericAttributeTraits<uint8_t>::kNullValue, "value.size() might be too big");
    VerifyOrReturnError(value.size() <= 16, Protocols::InteractionModel::Status::ConstraintError);
    uint8_t zclString[16 + 1];
    auto length = static_cast<uint8_t>(value.size());
    Encoding::Put8(zclString, length);
    memcpy(&zclString[1], value.data(), value.size());
    return emberAfWriteAttribute(endpoint, Clusters::WakeOnLan::Id, Id, zclString, ZCL_OCTET_STRING_ATTRIBUTE_TYPE);
}

} // namespace LinkLocalAddress

namespace FeatureMap {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint32_t * value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::WakeOnLan::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::WakeOnLan::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_BITMAP32_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::WakeOnLan::Id, Id, writable, ZCL_BITMAP32_ATTRIBUTE_TYPE);
}

} // namespace FeatureMap

namespace ClusterRevision {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint16_t * value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::WakeOnLan::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::WakeOnLan::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT16U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::WakeOnLan::Id, Id, writable, ZCL_INT16U_ATTRIBUTE_TYPE);
}

} // namespace ClusterRevision

} // namespace Attributes
} // namespace WakeOnLan

namespace Channel {
namespace Attributes {

namespace FeatureMap {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint32_t * value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp;
    uint8_t * readable                         = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status = emberAfReadAttribute(endpoint, Clusters::Channel::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::Channel::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_BITMAP32_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::Channel::Id, Id, writable, ZCL_BITMAP32_ATTRIBUTE_TYPE);
}

} // namespace FeatureMap

namespace ClusterRevision {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint16_t * value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp;
    uint8_t * readable                         = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status = emberAfReadAttribute(endpoint, Clusters::Channel::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::Channel::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT16U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::Channel::Id, Id, writable, ZCL_INT16U_ATTRIBUTE_TYPE);
}

} // namespace ClusterRevision

} // namespace Attributes
} // namespace Channel

namespace TargetNavigator {
namespace Attributes {

namespace CurrentTarget {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint8_t * value)
{
    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::TargetNavigator::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint8_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint8_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::TargetNavigator::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT8U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint8_t value)
{
    using Traits = NumericAttributeTraits<uint8_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::TargetNavigator::Id, Id, writable, ZCL_INT8U_ATTRIBUTE_TYPE);
}

} // namespace CurrentTarget

namespace FeatureMap {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint32_t * value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::TargetNavigator::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::TargetNavigator::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_BITMAP32_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::TargetNavigator::Id, Id, writable, ZCL_BITMAP32_ATTRIBUTE_TYPE);
}

} // namespace FeatureMap

namespace ClusterRevision {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint16_t * value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::TargetNavigator::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::TargetNavigator::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT16U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::TargetNavigator::Id, Id, writable, ZCL_INT16U_ATTRIBUTE_TYPE);
}

} // namespace ClusterRevision

} // namespace Attributes
} // namespace TargetNavigator

namespace MediaPlayback {
namespace Attributes {

namespace CurrentState {

Protocols::InteractionModel::Status Get(EndpointId endpoint, chip::app::Clusters::MediaPlayback::PlaybackStateEnum * value)
{
    using Traits = NumericAttributeTraits<chip::app::Clusters::MediaPlayback::PlaybackStateEnum>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::MediaPlayback::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::app::Clusters::MediaPlayback::PlaybackStateEnum value,
                                        MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<chip::app::Clusters::MediaPlayback::PlaybackStateEnum>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::MediaPlayback::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_ENUM8_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::app::Clusters::MediaPlayback::PlaybackStateEnum value)
{
    using Traits = NumericAttributeTraits<chip::app::Clusters::MediaPlayback::PlaybackStateEnum>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::MediaPlayback::Id, Id, writable, ZCL_ENUM8_ATTRIBUTE_TYPE);
}

} // namespace CurrentState

namespace StartTime {

Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<uint64_t> & value)
{
    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::MediaPlayback::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (Traits::IsNullValue(temp))
    {
        value.SetNull();
    }
    else
    {
        value.SetNonNull() = Traits::StorageToWorking(temp);
    }
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint64_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint64_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ true, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::MediaPlayback::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_EPOCH_US_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint64_t value)
{
    using Traits = NumericAttributeTraits<uint64_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ true, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::MediaPlayback::Id, Id, writable, ZCL_EPOCH_US_ATTRIBUTE_TYPE);
}

Protocols::InteractionModel::Status SetNull(EndpointId endpoint, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType value;
    Traits::SetNull(value);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(value);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::MediaPlayback::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_EPOCH_US_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status SetNull(EndpointId endpoint)
{
    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType value;
    Traits::SetNull(value);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(value);
    return emberAfWriteAttribute(endpoint, Clusters::MediaPlayback::Id, Id, writable, ZCL_EPOCH_US_ATTRIBUTE_TYPE);
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, const chip::app::DataModel::Nullable<uint64_t> & value,
                                        MarkAttributeDirty markDirty)
{
    if (value.IsNull())
    {
        return SetNull(endpoint, markDirty);
    }

    return Set(endpoint, value.Value(), markDirty);
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, const chip::app::DataModel::Nullable<uint64_t> & value)
{
    if (value.IsNull())
    {
        return SetNull(endpoint);
    }

    return Set(endpoint, value.Value());
}

} // namespace StartTime

namespace Duration {

Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<uint64_t> & value)
{
    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::MediaPlayback::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (Traits::IsNullValue(temp))
    {
        value.SetNull();
    }
    else
    {
        value.SetNonNull() = Traits::StorageToWorking(temp);
    }
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint64_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint64_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ true, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::MediaPlayback::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT64U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint64_t value)
{
    using Traits = NumericAttributeTraits<uint64_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ true, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::MediaPlayback::Id, Id, writable, ZCL_INT64U_ATTRIBUTE_TYPE);
}

Protocols::InteractionModel::Status SetNull(EndpointId endpoint, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType value;
    Traits::SetNull(value);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(value);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::MediaPlayback::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT64U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status SetNull(EndpointId endpoint)
{
    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType value;
    Traits::SetNull(value);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(value);
    return emberAfWriteAttribute(endpoint, Clusters::MediaPlayback::Id, Id, writable, ZCL_INT64U_ATTRIBUTE_TYPE);
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, const chip::app::DataModel::Nullable<uint64_t> & value,
                                        MarkAttributeDirty markDirty)
{
    if (value.IsNull())
    {
        return SetNull(endpoint, markDirty);
    }

    return Set(endpoint, value.Value(), markDirty);
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, const chip::app::DataModel::Nullable<uint64_t> & value)
{
    if (value.IsNull())
    {
        return SetNull(endpoint);
    }

    return Set(endpoint, value.Value());
}

} // namespace Duration

namespace PlaybackSpeed {

Protocols::InteractionModel::Status Get(EndpointId endpoint, float * value)
{
    using Traits = NumericAttributeTraits<float>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::MediaPlayback::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, float value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<float>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::MediaPlayback::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_SINGLE_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, float value)
{
    using Traits = NumericAttributeTraits<float>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::MediaPlayback::Id, Id, writable, ZCL_SINGLE_ATTRIBUTE_TYPE);
}

} // namespace PlaybackSpeed

namespace SeekRangeEnd {

Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<uint64_t> & value)
{
    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::MediaPlayback::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (Traits::IsNullValue(temp))
    {
        value.SetNull();
    }
    else
    {
        value.SetNonNull() = Traits::StorageToWorking(temp);
    }
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint64_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint64_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ true, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::MediaPlayback::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT64U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint64_t value)
{
    using Traits = NumericAttributeTraits<uint64_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ true, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::MediaPlayback::Id, Id, writable, ZCL_INT64U_ATTRIBUTE_TYPE);
}

Protocols::InteractionModel::Status SetNull(EndpointId endpoint, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType value;
    Traits::SetNull(value);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(value);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::MediaPlayback::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT64U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status SetNull(EndpointId endpoint)
{
    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType value;
    Traits::SetNull(value);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(value);
    return emberAfWriteAttribute(endpoint, Clusters::MediaPlayback::Id, Id, writable, ZCL_INT64U_ATTRIBUTE_TYPE);
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, const chip::app::DataModel::Nullable<uint64_t> & value,
                                        MarkAttributeDirty markDirty)
{
    if (value.IsNull())
    {
        return SetNull(endpoint, markDirty);
    }

    return Set(endpoint, value.Value(), markDirty);
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, const chip::app::DataModel::Nullable<uint64_t> & value)
{
    if (value.IsNull())
    {
        return SetNull(endpoint);
    }

    return Set(endpoint, value.Value());
}

} // namespace SeekRangeEnd

namespace SeekRangeStart {

Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<uint64_t> & value)
{
    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::MediaPlayback::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (Traits::IsNullValue(temp))
    {
        value.SetNull();
    }
    else
    {
        value.SetNonNull() = Traits::StorageToWorking(temp);
    }
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint64_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint64_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ true, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::MediaPlayback::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT64U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint64_t value)
{
    using Traits = NumericAttributeTraits<uint64_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ true, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::MediaPlayback::Id, Id, writable, ZCL_INT64U_ATTRIBUTE_TYPE);
}

Protocols::InteractionModel::Status SetNull(EndpointId endpoint, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType value;
    Traits::SetNull(value);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(value);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::MediaPlayback::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT64U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status SetNull(EndpointId endpoint)
{
    using Traits = NumericAttributeTraits<uint64_t>;
    Traits::StorageType value;
    Traits::SetNull(value);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(value);
    return emberAfWriteAttribute(endpoint, Clusters::MediaPlayback::Id, Id, writable, ZCL_INT64U_ATTRIBUTE_TYPE);
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, const chip::app::DataModel::Nullable<uint64_t> & value,
                                        MarkAttributeDirty markDirty)
{
    if (value.IsNull())
    {
        return SetNull(endpoint, markDirty);
    }

    return Set(endpoint, value.Value(), markDirty);
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, const chip::app::DataModel::Nullable<uint64_t> & value)
{
    if (value.IsNull())
    {
        return SetNull(endpoint);
    }

    return Set(endpoint, value.Value());
}

} // namespace SeekRangeStart

namespace FeatureMap {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint32_t * value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::MediaPlayback::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::MediaPlayback::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_BITMAP32_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::MediaPlayback::Id, Id, writable, ZCL_BITMAP32_ATTRIBUTE_TYPE);
}

} // namespace FeatureMap

namespace ClusterRevision {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint16_t * value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::MediaPlayback::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::MediaPlayback::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT16U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::MediaPlayback::Id, Id, writable, ZCL_INT16U_ATTRIBUTE_TYPE);
}

} // namespace ClusterRevision

} // namespace Attributes
} // namespace MediaPlayback

namespace MediaInput {
namespace Attributes {

namespace CurrentInput {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint8_t * value)
{
    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::MediaInput::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint8_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint8_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::MediaInput::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT8U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint8_t value)
{
    using Traits = NumericAttributeTraits<uint8_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::MediaInput::Id, Id, writable, ZCL_INT8U_ATTRIBUTE_TYPE);
}

} // namespace CurrentInput

namespace FeatureMap {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint32_t * value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::MediaInput::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::MediaInput::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_BITMAP32_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::MediaInput::Id, Id, writable, ZCL_BITMAP32_ATTRIBUTE_TYPE);
}

} // namespace FeatureMap

namespace ClusterRevision {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint16_t * value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::MediaInput::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::MediaInput::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT16U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::MediaInput::Id, Id, writable, ZCL_INT16U_ATTRIBUTE_TYPE);
}

} // namespace ClusterRevision

} // namespace Attributes
} // namespace MediaInput

namespace LowPower {
namespace Attributes {

namespace FeatureMap {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint32_t * value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp;
    uint8_t * readable                         = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status = emberAfReadAttribute(endpoint, Clusters::LowPower::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::LowPower::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_BITMAP32_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::LowPower::Id, Id, writable, ZCL_BITMAP32_ATTRIBUTE_TYPE);
}

} // namespace FeatureMap

namespace ClusterRevision {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint16_t * value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp;
    uint8_t * readable                         = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status = emberAfReadAttribute(endpoint, Clusters::LowPower::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::LowPower::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT16U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::LowPower::Id, Id, writable, ZCL_INT16U_ATTRIBUTE_TYPE);
}

} // namespace ClusterRevision

} // namespace Attributes
} // namespace LowPower

namespace KeypadInput {
namespace Attributes {

namespace FeatureMap {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint32_t * value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::KeypadInput::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::KeypadInput::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_BITMAP32_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::KeypadInput::Id, Id, writable, ZCL_BITMAP32_ATTRIBUTE_TYPE);
}

} // namespace FeatureMap

namespace ClusterRevision {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint16_t * value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::KeypadInput::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::KeypadInput::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT16U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::KeypadInput::Id, Id, writable, ZCL_INT16U_ATTRIBUTE_TYPE);
}

} // namespace ClusterRevision

} // namespace Attributes
} // namespace KeypadInput

namespace ContentLauncher {
namespace Attributes {

namespace SupportedStreamingProtocols {

Protocols::InteractionModel::Status Get(EndpointId endpoint,
                                        chip::BitMask<chip::app::Clusters::ContentLauncher::SupportedProtocolsBitmap> * value)
{
    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::ContentLauncher::SupportedProtocolsBitmap>>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::ContentLauncher::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint,
                                        chip::BitMask<chip::app::Clusters::ContentLauncher::SupportedProtocolsBitmap> value,
                                        MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::ContentLauncher::SupportedProtocolsBitmap>>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::ContentLauncher::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_BITMAP32_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint,
                                        chip::BitMask<chip::app::Clusters::ContentLauncher::SupportedProtocolsBitmap> value)
{
    using Traits = NumericAttributeTraits<chip::BitMask<chip::app::Clusters::ContentLauncher::SupportedProtocolsBitmap>>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::ContentLauncher::Id, Id, writable, ZCL_BITMAP32_ATTRIBUTE_TYPE);
}

} // namespace SupportedStreamingProtocols

namespace FeatureMap {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint32_t * value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::ContentLauncher::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::ContentLauncher::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_BITMAP32_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::ContentLauncher::Id, Id, writable, ZCL_BITMAP32_ATTRIBUTE_TYPE);
}

} // namespace FeatureMap

namespace ClusterRevision {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint16_t * value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::ContentLauncher::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::ContentLauncher::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT16U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::ContentLauncher::Id, Id, writable, ZCL_INT16U_ATTRIBUTE_TYPE);
}

} // namespace ClusterRevision

} // namespace Attributes
} // namespace ContentLauncher

namespace AudioOutput {
namespace Attributes {

namespace CurrentOutput {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint8_t * value)
{
    using Traits = NumericAttributeTraits<uint8_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::AudioOutput::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint8_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint8_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::AudioOutput::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT8U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint8_t value)
{
    using Traits = NumericAttributeTraits<uint8_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::AudioOutput::Id, Id, writable, ZCL_INT8U_ATTRIBUTE_TYPE);
}

} // namespace CurrentOutput

namespace FeatureMap {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint32_t * value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::AudioOutput::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::AudioOutput::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_BITMAP32_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::AudioOutput::Id, Id, writable, ZCL_BITMAP32_ATTRIBUTE_TYPE);
}

} // namespace FeatureMap

namespace ClusterRevision {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint16_t * value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::AudioOutput::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::AudioOutput::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT16U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::AudioOutput::Id, Id, writable, ZCL_INT16U_ATTRIBUTE_TYPE);
}

} // namespace ClusterRevision

} // namespace Attributes
} // namespace AudioOutput

namespace ApplicationLauncher {
namespace Attributes {

namespace FeatureMap {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint32_t * value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::ApplicationLauncher::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::ApplicationLauncher::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_BITMAP32_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::ApplicationLauncher::Id, Id, writable, ZCL_BITMAP32_ATTRIBUTE_TYPE);
}

} // namespace FeatureMap

namespace ClusterRevision {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint16_t * value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::ApplicationLauncher::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::ApplicationLauncher::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT16U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::ApplicationLauncher::Id, Id, writable, ZCL_INT16U_ATTRIBUTE_TYPE);
}

} // namespace ClusterRevision

} // namespace Attributes
} // namespace ApplicationLauncher

namespace ApplicationBasic {
namespace Attributes {

namespace VendorName {

Protocols::InteractionModel::Status Get(EndpointId endpoint, chip::MutableCharSpan & value)
{
    uint8_t zclString[32 + 1];
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::ApplicationBasic::Id, Id, zclString, sizeof(zclString));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    size_t length = emberAfStringLength(zclString);
    if (length == NumericAttributeTraits<uint8_t>::kNullValue)
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }

    VerifyOrReturnError(value.size() == 32, Protocols::InteractionModel::Status::InvalidDataType);
    memcpy(value.data(), &zclString[1], 32);
    value.reduce_size(length);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::CharSpan value, MarkAttributeDirty markDirty)
{

    static_assert(32 < NumericAttributeTraits<uint8_t>::kNullValue, "value.size() might be too big");
    VerifyOrReturnError(value.size() <= 32, Protocols::InteractionModel::Status::ConstraintError);
    uint8_t zclString[32 + 1];
    auto length = static_cast<uint8_t>(value.size());
    Encoding::Put8(zclString, length);
    memcpy(&zclString[1], value.data(), value.size());
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::ApplicationBasic::Id, Id),
                                 EmberAfWriteDataInput(zclString, ZCL_CHAR_STRING_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::CharSpan value)
{

    static_assert(32 < NumericAttributeTraits<uint8_t>::kNullValue, "value.size() might be too big");
    VerifyOrReturnError(value.size() <= 32, Protocols::InteractionModel::Status::ConstraintError);
    uint8_t zclString[32 + 1];
    auto length = static_cast<uint8_t>(value.size());
    Encoding::Put8(zclString, length);
    memcpy(&zclString[1], value.data(), value.size());
    return emberAfWriteAttribute(endpoint, Clusters::ApplicationBasic::Id, Id, zclString, ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
}

} // namespace VendorName

namespace VendorID {

Protocols::InteractionModel::Status Get(EndpointId endpoint, chip::VendorId * value)
{
    using Traits = NumericAttributeTraits<chip::VendorId>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::ApplicationBasic::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::VendorId value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<chip::VendorId>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::ApplicationBasic::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_VENDOR_ID_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::VendorId value)
{
    using Traits = NumericAttributeTraits<chip::VendorId>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::ApplicationBasic::Id, Id, writable, ZCL_VENDOR_ID_ATTRIBUTE_TYPE);
}

} // namespace VendorID

namespace ApplicationName {

Protocols::InteractionModel::Status Get(EndpointId endpoint, chip::MutableCharSpan & value)
{
    uint8_t zclString[256 + 2];
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::ApplicationBasic::Id, Id, zclString, sizeof(zclString));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    size_t length = emberAfLongStringLength(zclString);
    if (length == NumericAttributeTraits<uint16_t>::kNullValue)
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }

    VerifyOrReturnError(value.size() == 256, Protocols::InteractionModel::Status::InvalidDataType);
    memcpy(value.data(), &zclString[2], 256);
    value.reduce_size(length);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::CharSpan value, MarkAttributeDirty markDirty)
{

    static_assert(256 < NumericAttributeTraits<uint16_t>::kNullValue, "value.size() might be too big");
    VerifyOrReturnError(value.size() <= 256, Protocols::InteractionModel::Status::ConstraintError);
    uint8_t zclString[256 + 2];
    auto length = static_cast<uint16_t>(value.size());
    Encoding::LittleEndian::Put16(zclString, length);
    memcpy(&zclString[2], value.data(), value.size());
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::ApplicationBasic::Id, Id),
                                 EmberAfWriteDataInput(zclString, ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::CharSpan value)
{

    static_assert(256 < NumericAttributeTraits<uint16_t>::kNullValue, "value.size() might be too big");
    VerifyOrReturnError(value.size() <= 256, Protocols::InteractionModel::Status::ConstraintError);
    uint8_t zclString[256 + 2];
    auto length = static_cast<uint16_t>(value.size());
    Encoding::LittleEndian::Put16(zclString, length);
    memcpy(&zclString[2], value.data(), value.size());
    return emberAfWriteAttribute(endpoint, Clusters::ApplicationBasic::Id, Id, zclString, ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE);
}

} // namespace ApplicationName

namespace ProductID {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint16_t * value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::ApplicationBasic::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::ApplicationBasic::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT16U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::ApplicationBasic::Id, Id, writable, ZCL_INT16U_ATTRIBUTE_TYPE);
}

} // namespace ProductID

namespace Status {

Protocols::InteractionModel::Status Get(EndpointId endpoint, chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum * value)
{
    using Traits = NumericAttributeTraits<chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::ApplicationBasic::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum value,
                                        MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::ApplicationBasic::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_ENUM8_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum value)
{
    using Traits = NumericAttributeTraits<chip::app::Clusters::ApplicationBasic::ApplicationStatusEnum>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::ApplicationBasic::Id, Id, writable, ZCL_ENUM8_ATTRIBUTE_TYPE);
}

} // namespace Status

namespace ApplicationVersion {

Protocols::InteractionModel::Status Get(EndpointId endpoint, chip::MutableCharSpan & value)
{
    uint8_t zclString[32 + 1];
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::ApplicationBasic::Id, Id, zclString, sizeof(zclString));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    size_t length = emberAfStringLength(zclString);
    if (length == NumericAttributeTraits<uint8_t>::kNullValue)
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }

    VerifyOrReturnError(value.size() == 32, Protocols::InteractionModel::Status::InvalidDataType);
    memcpy(value.data(), &zclString[1], 32);
    value.reduce_size(length);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::CharSpan value, MarkAttributeDirty markDirty)
{

    static_assert(32 < NumericAttributeTraits<uint8_t>::kNullValue, "value.size() might be too big");
    VerifyOrReturnError(value.size() <= 32, Protocols::InteractionModel::Status::ConstraintError);
    uint8_t zclString[32 + 1];
    auto length = static_cast<uint8_t>(value.size());
    Encoding::Put8(zclString, length);
    memcpy(&zclString[1], value.data(), value.size());
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::ApplicationBasic::Id, Id),
                                 EmberAfWriteDataInput(zclString, ZCL_CHAR_STRING_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::CharSpan value)
{

    static_assert(32 < NumericAttributeTraits<uint8_t>::kNullValue, "value.size() might be too big");
    VerifyOrReturnError(value.size() <= 32, Protocols::InteractionModel::Status::ConstraintError);
    uint8_t zclString[32 + 1];
    auto length = static_cast<uint8_t>(value.size());
    Encoding::Put8(zclString, length);
    memcpy(&zclString[1], value.data(), value.size());
    return emberAfWriteAttribute(endpoint, Clusters::ApplicationBasic::Id, Id, zclString, ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
}

} // namespace ApplicationVersion

namespace FeatureMap {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint32_t * value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::ApplicationBasic::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::ApplicationBasic::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_BITMAP32_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::ApplicationBasic::Id, Id, writable, ZCL_BITMAP32_ATTRIBUTE_TYPE);
}

} // namespace FeatureMap

namespace ClusterRevision {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint16_t * value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::ApplicationBasic::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::ApplicationBasic::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT16U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::ApplicationBasic::Id, Id, writable, ZCL_INT16U_ATTRIBUTE_TYPE);
}

} // namespace ClusterRevision

} // namespace Attributes
} // namespace ApplicationBasic

namespace ContentAppObserver {
namespace Attributes {

namespace FeatureMap {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint32_t * value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::ContentAppObserver::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::ContentAppObserver::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_BITMAP32_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint32_t value)
{
    using Traits = NumericAttributeTraits<uint32_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::ContentAppObserver::Id, Id, writable, ZCL_BITMAP32_ATTRIBUTE_TYPE);
}

} // namespace FeatureMap

namespace ClusterRevision {

Protocols::InteractionModel::Status Get(EndpointId endpoint, uint16_t * value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    Traits::StorageType temp;
    uint8_t * readable = Traits::ToAttributeStoreRepresentation(temp);
    Protocols::InteractionModel::Status status =
        emberAfReadAttribute(endpoint, Clusters::ContentAppObserver::Id, Id, readable, sizeof(temp));
    VerifyOrReturnError(Protocols::InteractionModel::Status::Success == status, status);
    if (!Traits::CanRepresentValue(/* isNullable = */ false, temp))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    *value = Traits::StorageToWorking(temp);
    return status;
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value, MarkAttributeDirty markDirty)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(ConcreteAttributePath(endpoint, Clusters::ContentAppObserver::Id, Id),
                                 EmberAfWriteDataInput(writable, ZCL_INT16U_ATTRIBUTE_TYPE).SetMarkDirty(markDirty));
}

Protocols::InteractionModel::Status Set(EndpointId endpoint, uint16_t value)
{
    using Traits = NumericAttributeTraits<uint16_t>;
    if (!Traits::CanRepresentValue(/* isNullable = */ false, value))
    {
        return Protocols::InteractionModel::Status::ConstraintError;
    }
    Traits::StorageType storageValue;
    Traits::WorkingToStorage(value, storageValue);
    uint8_t * writable = Traits::ToAttributeStoreRepresentation(storageValue);
    return emberAfWriteAttribute(endpoint, Clusters::ContentAppObserver::Id, Id, writable, ZCL_INT16U_ATTRIBUTE_TYPE);
}

} // namespace ClusterRevision

} // namespace Attributes
} // namespace ContentAppObserver

} // namespace Clusters
} // namespace app
} // namespace chip
