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

namespace chip {
namespace app {
namespace Clusters {
namespace GeneralCommissioning {

/// Returns the singleton instance of the general commissioning cluster, if enabled.
///
/// This is a temporary integration point for ember-based apps.
GeneralCommissioningCluster * Instance();

} // namespace GeneralCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
