/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
