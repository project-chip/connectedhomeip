/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>

#include <app/AttributeAccessInterface.h>
#include <app/CommandResponseHelper.h>
#include <app/util/af.h>
#include <list>

namespace chip {
namespace app {
namespace Clusters {
namespace TargetNavigator {

/** @brief
 *    Defines methods for implementing application-specific logic for the Target Navigator Cluster.
 */
class Delegate
{
public:
    virtual CHIP_ERROR HandleGetTargetList(app::AttributeValueEncoder & aEncoder)     = 0;
    virtual uint8_t HandleGetCurrentTarget()                                          = 0;
    virtual void HandleNavigateTarget(CommandResponseHelper<Commands::NavigateTargetResponse::Type> & helper,
                                      const uint64_t & target, const CharSpan & data) = 0;

    virtual ~Delegate() = default;
};

} // namespace TargetNavigator
} // namespace Clusters
} // namespace app
} // namespace chip
