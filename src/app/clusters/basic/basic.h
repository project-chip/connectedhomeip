/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/util/basic-types.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Basic {
/**
 * Check whether LocalConfigDisabled is set (on endpoint 0, which is the only
 * place the Basic Information cluster exists and can have the attribute be
 * set).
 */
bool IsLocalConfigDisabled();
} // namespace Basic
} // namespace Clusters
} // namespace app
} // namespace chip

/** @brief Basic Cluster Server Init
 *
 * This function is called at startup for a given endpoint to initialize
 * attributes of the Basic Cluster.
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 */
void emberAfBasicClusterServerInitCallback(chip::EndpointId endpoint);
