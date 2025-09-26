/*
 *    Copyright (c) 2021-2025 Project CHIP Authors
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

#include <app/persistence/AttributePersistenceProvider.h>

namespace chip {
namespace app {

/**
 * Instance getter for the global AttributePersistenceProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * @return The global AttributePersistenceProvider.  This must never be null.
 */
AttributePersistenceProvider * GetAttributePersistenceProvider();

/**
 * Instance setter for the global AttributePersistenceProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * If the `provider` is nullptr, the value is not changed.
 *
 * @param[in] aProvider the AttributePersistenceProvider implementation to use.
 */
void SetAttributePersistenceProvider(AttributePersistenceProvider * aProvider);

} // namespace app
} // namespace chip
