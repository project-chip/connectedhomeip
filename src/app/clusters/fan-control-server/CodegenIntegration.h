/*
 *
 *    Copyright (c) 2022-2026 Project CHIP Authors
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

#pragma once

#include <app/clusters/fan-control-server/FanControlCluster.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip::app::Clusters::FanControl {

/**
 * Returns the code-driven Fan Control server cluster instance registered for `endpointId` when using the
 * codegen data model, or `nullptr` if the endpoint has no Fan Control server or the cluster is not registered.
 */
FanControlCluster * FindClusterOnEndpoint(EndpointId endpointId);

/**
 * Registers the application `FanControl::Delegate` for `aEndpoint` used by the codegen integration layer.
 *
 * The integration keeps one delegate pointer per Fan Control server endpoint. The code-driven cluster is
 * always wired to an internal `FanControlIntegrationDelegateWrapper` (see CodegenIntegration.cpp) that
 * holds that pointer and forwards `HandleStep` and optional notifications; when no application delegate
 * is registered, `HandleStep` returns Failure. Call this from application init (for example
 * `emberAfFanControlClusterInitCallback`) before or after the cluster is constructed; `Init` on the wrapper
 * updates the live target if the cluster instance already exists.
 *
 * @param aDelegate Application delegate, or `nullptr` to unbind.
 */
void SetDefaultDelegate(EndpointId aEndpoint, Delegate * aDelegate);

/** Returns the delegate registered with `SetDefaultDelegate` for `aEndpoint`, or `nullptr` if none or invalid. */
Delegate * GetDelegate(EndpointId aEndpoint);

/**
 * Live Fan Control attribute access for the code-driven server cluster (reads and writes `FanControlCluster` runtime state).
 *
 * For startup values stored in the ZAP/ember attribute store, use the generated `GetDefault` functions in
 * `app-common/zap-generated/attributes/Accessors.h`.
 */
namespace Attributes {

namespace FanMode {
Protocols::InteractionModel::Status Get(EndpointId endpoint, FanModeEnum * value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, FanModeEnum value);
} // namespace FanMode

namespace FanModeSequence {
Protocols::InteractionModel::Status Get(EndpointId endpoint, FanModeSequenceEnum * value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, FanModeSequenceEnum value);
} // namespace FanModeSequence

namespace PercentSetting {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<chip::Percent> & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::Percent value);
} // namespace PercentSetting

namespace PercentCurrent {
Protocols::InteractionModel::Status Get(EndpointId endpoint, chip::Percent * value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, chip::Percent value);
} // namespace PercentCurrent

namespace SpeedSetting {
Protocols::InteractionModel::Status Get(EndpointId endpoint, DataModel::Nullable<uint8_t> & value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, uint8_t value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, const DataModel::Nullable<uint8_t> & value);
} // namespace SpeedSetting

namespace SpeedCurrent {
Protocols::InteractionModel::Status Get(EndpointId endpoint, uint8_t * value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, uint8_t value);
} // namespace SpeedCurrent

namespace SpeedMax {
Protocols::InteractionModel::Status Get(EndpointId endpoint, uint8_t * value);
} // namespace SpeedMax

namespace FeatureMap {
Protocols::InteractionModel::Status Get(EndpointId endpoint, uint32_t * value);
} // namespace FeatureMap

namespace AirflowDirection {
Protocols::InteractionModel::Status Get(EndpointId endpoint, AirflowDirectionEnum * value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, AirflowDirectionEnum value);
} // namespace AirflowDirection

namespace RockSupport {
Protocols::InteractionModel::Status Get(EndpointId endpoint, BitMask<RockBitmap> * value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, BitMask<RockBitmap> value);
} // namespace RockSupport

namespace RockSetting {
Protocols::InteractionModel::Status Get(EndpointId endpoint, BitMask<RockBitmap> * value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, BitMask<RockBitmap> value);
} // namespace RockSetting

namespace WindSupport {
Protocols::InteractionModel::Status Get(EndpointId endpoint, BitMask<WindBitmap> * value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, BitMask<WindBitmap> value);
} // namespace WindSupport

namespace WindSetting {
Protocols::InteractionModel::Status Get(EndpointId endpoint, BitMask<WindBitmap> * value);
Protocols::InteractionModel::Status Set(EndpointId endpoint, BitMask<WindBitmap> value);
} // namespace WindSetting

} // namespace Attributes

} // namespace chip::app::Clusters::FanControl
