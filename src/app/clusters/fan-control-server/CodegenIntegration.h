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

namespace chip::app::Clusters::FanControl {

/**
 * Returns the code-driven Fan Control server cluster instance registered for `endpointId` when using the
 * codegen data model, or `nullptr` if the endpoint has no Fan Control server or the cluster is not registered.
 */
FanControlCluster * FindClusterOnEndpoint(EndpointId endpointId);

/**
 * Registers the application `FanControl::Delegate` for `aEndpoint` used by the codegen integration layer.
 *
 * The integration keeps one delegate pointer per Fan Control server endpoint. That pointer is passed into
 * `FanControlCluster` when the cluster instance is created, and drives application-specific behavior (for
 * example the Step command via `Delegate::HandleStep`, and optional notifications such as
 * `OnFanDriveStateChanged`). Call this from application init (for example `emberAfFanControlClusterInitCallback`)
 * before or after the cluster is constructed, if the cluster already exists, the running instance is updated.
 *
 * @param aDelegate Application delegate, or `nullptr` if none.
 */
void SetDefaultDelegate(EndpointId aEndpoint, Delegate * aDelegate);

/** Returns the delegate registered with `SetDefaultDelegate` for `aEndpoint`, or `nullptr` if none or invalid. */
Delegate * GetDelegate(EndpointId aEndpoint);

} // namespace chip::app::Clusters::FanControl
