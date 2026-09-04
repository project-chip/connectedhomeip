/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "LaundryWasherControlsCluster.h"

namespace chip::app::Clusters::LaundryWasherControls {

namespace LaundryWasherControlsServer {

/**
 * Set the delegate of laundry washer server at endpoint x
 * @param endpoint ID of the endpoint
 * @param delegate The delegate at the endpoint.
 * The delegate should be valid until the cluster on the endpoint is destroyed. This will probably happen at the end of the program.
 * @note This function can be called only after Server::Init is called
 */
void SetDelegate(EndpointId endpoint, Delegate & delegate);

/// Same as SetDelegate, kept for backward compatibility. Use SetDelegate instead.
/// @param delegate SHOULD NOT be nullptr.
void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);

/**
 * API to set/get the SpinSpeedCurrent attribute
 */
CHIP_ERROR SetSpinSpeedCurrent(EndpointId endpointId, DataModel::Nullable<uint8_t> spinSpeedCurrent);
CHIP_ERROR GetSpinSpeedCurrent(EndpointId endpointId, DataModel::Nullable<uint8_t> & spinSpeedCurrent);

/**
 * API to set/get the NumberOfRinses attribute
 */
CHIP_ERROR SetNumberOfRinses(EndpointId endpointId, NumberOfRinsesEnum newNumberOfRinses);
CHIP_ERROR GetNumberOfRinses(EndpointId endpointId, NumberOfRinsesEnum & numberOfRinses);

} // namespace LaundryWasherControlsServer

LaundryWasherControlsCluster * FindClusterOnEndpoint(EndpointId endpoint);

} // namespace chip::app::Clusters::LaundryWasherControls
