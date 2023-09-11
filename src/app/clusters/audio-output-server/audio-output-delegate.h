/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>

#include <app/AttributeAccessInterface.h>
#include <app/util/af.h>
#include <list>

namespace chip {
namespace app {
namespace Clusters {
namespace AudioOutput {

/** @brief
 *    Defines methods for implementing application-specific logic for the Audio Output Cluster.
 */
class Delegate
{
public:
    virtual uint8_t HandleGetCurrentOutput()                                            = 0;
    virtual CHIP_ERROR HandleGetOutputList(app::AttributeValueEncoder & aEncoder)       = 0;
    virtual bool HandleRenameOutput(const uint8_t & index, const chip::CharSpan & name) = 0;
    virtual bool HandleSelectOutput(const uint8_t & index)                              = 0;

    virtual ~Delegate() = default;
};

} // namespace AudioOutput
} // namespace Clusters
} // namespace app
} // namespace chip
