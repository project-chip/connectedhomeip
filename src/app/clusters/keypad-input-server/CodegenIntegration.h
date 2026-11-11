/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
/****************************************************************************
 * @file
 * @brief Routines for the Keypad Input plugin, the
 *server implementation of the Keypad Input cluster.
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include <app/clusters/keypad-input-server/KeypadInputCluster.h>
#include <app/clusters/keypad-input-server/keypad-input-delegate.h>

namespace chip {
namespace app {
namespace Clusters {
namespace KeypadInput {

/// Sets the delegate used by the Keypad Input cluster on `endpoint`.
///
/// `endpoint` must already be enabled; otherwise the call is ignored. The cluster is created when the
/// endpoint is enabled, so `emberAfKeypadInputClusterInitCallback` is a valid place for this call.
///
/// The delegate is kept while the endpoint is disabled and enabled again. The delegate must stay valid
/// until it is replaced or the endpoint is permanently removed. Passing nullptr clears the delegate.
///
/// On endpoints that belong to a ContentApp (when CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED is set), the
/// ContentApp delegate takes precedence over the delegate set here.
void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);

/// Returns the Keypad Input cluster instance on `endpoint`, or nullptr if there is none.
KeypadInputCluster * FindClusterOnEndpoint(EndpointId endpoint);

} // namespace KeypadInput
} // namespace Clusters
} // namespace app
} // namespace chip
