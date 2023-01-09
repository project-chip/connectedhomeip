/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <app/util/basic-types.h>

namespace chip {
namespace app {
namespace Clusters {
namespace BasicInformation {
/**
 * Check whether LocalConfigDisabled is set (on endpoint 0, which is the only
 * place the Basic Information cluster exists and can have the attribute be
 * set).
 */
bool IsLocalConfigDisabled();
} // namespace BasicInformation
} // namespace Clusters
} // namespace app
} // namespace chip
