/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace Access {

struct RequestPath
{
    // NOTE: eventually this will likely also contain node, for proxying
    ClusterId cluster   = 0;
    EndpointId endpoint = 0;
};

} // namespace Access
} // namespace chip
