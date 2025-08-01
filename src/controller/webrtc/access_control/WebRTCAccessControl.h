/**
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace Controller {
namespace AccessControl {

/**
 * Initialize the access control module with the specified WebRTC endpoint ID.
 * Must be called on the Matter task queue.
 *
 * @param endpointId The endpoint ID where the WebRTC Transport Requestor cluster is hosted
 */
void InitAccessControl(chip::EndpointId endpointId);

} // namespace AccessControl
} // namespace Controller
} // namespace chip
