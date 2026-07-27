/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

/* Extra definitions required for the OTA requestor to work with the codegen data model.
 */

#include <app/clusters/ota-requestor/OTARequestorInterface.h>

#pragma once

namespace chip {

// An internal callback to assist with backwards compatibility for codegen. This callback exists because the global requestor
// instance can be set before or after when the OTARequestorCluster instance in CodegenIntegration is created. The callback is used
// to ensure that regardless of the order the OTARequestorCluster instance can forward commands to the OTARequestorInterface
// implementation. The implementation of SetRequestorInstance must call this (if not null) after setting the requestor instance.
// THIS MUST ONLY BE SET IN CodegenIntegration.cpp!
extern void (*gInternalOnSetRequestorInstance)(OTARequestorInterface * instance);

} // namespace chip
