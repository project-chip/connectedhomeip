/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>

namespace chip {
namespace app {
namespace Clusters {
namespace LowPower {

/** @brief
 *    Defines methods for implementing application-specific logic for the Low Power Cluster.
 */
class Delegate
{
public:
    virtual bool HandleSleep() = 0;

    virtual ~Delegate() = default;
};

} // namespace LowPower
} // namespace Clusters
} // namespace app
} // namespace chip
