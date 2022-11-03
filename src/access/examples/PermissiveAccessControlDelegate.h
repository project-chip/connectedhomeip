/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "access/AccessControl.h"

namespace chip {
namespace Access {
namespace Examples {

AccessControl::Delegate * GetPermissiveAccessControlDelegate();

} // namespace Examples
} // namespace Access
} // namespace chip
