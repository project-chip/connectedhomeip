/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WakeOnLan {

/** @brief
 *    Defines methods for implementing application-specific logic for the Wake on LAN Cluster.
 */
class Delegate
{
public:
    virtual CHIP_ERROR HandleGetMacAddress(app::AttributeValueEncoder & aEncoder) = 0;

    virtual ~Delegate() = default;
};

} // namespace WakeOnLan
} // namespace Clusters
} // namespace app
} // namespace chip
