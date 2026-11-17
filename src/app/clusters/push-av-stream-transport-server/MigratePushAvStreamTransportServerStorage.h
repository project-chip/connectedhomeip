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

#pragma once

#include <app/persistence/AttributePersistenceMigration.h>

namespace chip::app::Clusters::PushAvStreamTransport {

/**
 * Moves the CurrentConnections value stored by older firmware through SafeAttributePersistenceProvider
 * into `dstProvider`. The old value is deleted after it is read.
 */
CHIP_ERROR MigratePushAvStreamTransportServerStorage(EndpointId endpointId, SafeAttributePersistenceProvider & safeProvider,
                                                     AttributePersistenceProvider & dstProvider);

} // namespace chip::app::Clusters::PushAvStreamTransport
