/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/core/CHIPPersistentStorageDelegate.h>

namespace chip {
namespace audit {

/**
 * @brief Execute a storage API compliance audit to validate PersistentStorageDelegate implemented in a
 *        final integration.
 *
 * This audit runs a device-based test that validates the internals of whatever back-end is used to implement
 * the PersistentStorageDelegate interface, which is used across the board. If it fails, the device may not
 * function as expected and subtly fail important functionality of the Matter stack.
 *
 * The audit uses ChipLogError against the `Automation` module for assertions failure details..
 *
 * @param storage - Reference to the PersistentStorageDelegate that would be passed in chip::Server::InitParams
 *                  or chip::Controller::FactoryInitParams.
 *
 * @return true on success, otherwise false on failure
 */
bool ExecutePersistentStorageApiAudit(PersistentStorageDelegate & storage);

/**
 * @brief Execute a storage API load test audit to validate bare-minimum storage requirements
 *        for spec-mandated minimums.
 *
 * This audit runs a device-based test that validates the storage of a large number of
 * persistent storage values, consistent with a "full loaded" configuration of just
 * the minima required by spec.
 *
 * If it fails, the device may not function as expected and subtly fail important functionality of
 * the Matter stack. Furthermore, if it fails, it MAY FAIL at runtime later. This audit DOES NOT
 * GUARANTEE the fitness overall of the implementation. It is mostly used as a helpful "smoke test"
 * for conditions that are quite complex to reproduce at runtime in integration tests.
 *
 * Overall the audit attempts to:
 *   - Store the equivalent of 5 full fabric tables
 *     - Each with full Access Control List with the required minimum-to-be-supported entries
 *   - Store additional metadata known to be required by the SDK, depending on some
 *     feature flags.
 *
 * The contents are not real, but the sizing of the keys and values is representative of
 * worst case.
 *
 * Because this audit does not have access to internal constants of all the underlying modules,
 * consider this starting version to be a "point in time" check. Refinements will be done
 * via integration tests.
 *
 * The audit uses ChipLogError against the `Automation` module for assertions failure details.
 *
 * @param storage - Reference to the PersistentStorageDelegate that would be passed in chip::Server::InitParams
 *                  or chip::Controller::FactoryInitParams.
 *
 * @return true on success, otherwise false on failure
 */
bool ExecutePersistentStorageLoadTestAudit(PersistentStorageDelegate & storage);

} // namespace audit
} // namespace chip
