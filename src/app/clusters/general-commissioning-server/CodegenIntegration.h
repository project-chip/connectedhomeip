/*
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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

#include <app/clusters/general-commissioning-server/general-commissioning-cluster.h>

namespace chip::app::Clusters::GeneralCommissioning {

// Get the instance of the cluster if one was created (generally devices have on on the root endpoint)
// will return nullptr if the cluster has not yet been created;
GeneralCommissioningCluster *Instance();

// This will ensure that the cluster has been created for the root endpoint. This method exists because
// dependencies: some clusters require an `Instance` to be available inside their startul, like the network
// commissioning cluster requiring a general commissioning cluster.
void EnsureCreated();

} // namespace chip::app::Clusters::GeneralCommissioning
