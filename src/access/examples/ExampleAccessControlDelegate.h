/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "access/AccessControl.h"
#include <lib/core/CHIPPersistentStorageDelegate.h>

namespace chip {
namespace Access {
namespace Examples {

/**
 * @brief Get a global instance of the access control delegate implemented in this module.
 *
 * NOTE: This function should be followed by an ::Init() method call. This function does
 *       not manage lifecycle considerations.
 *
 * @return a pointer to the AccessControl::Delegate singleton.
 */
AccessControl::Delegate * GetAccessControlDelegate();

} // namespace Examples
} // namespace Access
} // namespace chip
