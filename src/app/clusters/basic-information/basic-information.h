/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
