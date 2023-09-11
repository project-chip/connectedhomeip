/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>

#include <app/CommandResponseHelper.h>
#include <app/util/af.h>

namespace chip {
namespace app {
namespace Clusters {
namespace KeypadInput {

/** @brief
 *    Defines methods for implementing application-specific logic for the Keypad Input Cluster.
 */
class Delegate
{
public:
    virtual void HandleSendKey(CommandResponseHelper<Commands::SendKeyResponse::Type> & helper, const CecKeyCode & keyCode) = 0;

    bool HasFeature(chip::EndpointId endpoint, Feature feature);

    virtual uint32_t GetFeatureMap(chip::EndpointId endpoint) = 0;

    virtual ~Delegate() = default;
};

} // namespace KeypadInput
} // namespace Clusters
} // namespace app
} // namespace chip
