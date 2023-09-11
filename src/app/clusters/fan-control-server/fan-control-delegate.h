/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandResponseHelper.h>
#include <app/util/af.h>

namespace chip {
namespace app {
namespace Clusters {
namespace FanControl {

/** @brief
 *    Defines methods for implementing application-specific logic for the FanControl Cluster.
 */
class Delegate
{
public:
    /**
     * @brief
     *   This method handles the step command. This will happen as fast as possible.
     *
     *   @param[in]  aDirection     the direction in which the speed should step
     *   @param[in]  aWrap          whether the speed should wrap or not
     *   @param[in]  aLowestOff     whether the device should consider the lowest setting as off
     *
     *   @return Success On success.
     *   @return Other Value indicating it failed to execute the command.
     */
    virtual Protocols::InteractionModel::Status HandleStep(StepDirectionEnum aDirection, bool aWrap, bool aLowestOff) = 0;

    Delegate(EndpointId aEndpoint) : mEndpoint(aEndpoint) {}

    virtual ~Delegate() = default;

protected:
    EndpointId mEndpoint = 0;
};

} // namespace FanControl
} // namespace Clusters
} // namespace app
} // namespace chip
