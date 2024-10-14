/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeValueEncoder.h>
#include <app/MessageDef/AttributeDataIB.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ThreadNetworkDiagnostics {

CHIP_ERROR WriteThreadNetworkDiagnosticAttributeToTlv(AttributeId attributeId, app::AttributeValueEncoder & encoder);

} // namespace ThreadNetworkDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
