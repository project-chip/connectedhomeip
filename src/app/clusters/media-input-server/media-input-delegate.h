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
namespace MediaInput {

/** @brief
 *    Defines methods for implementing application-specific logic for the Media Input Cluster.
 */
class Delegate
{
public:
    // no easy way to handle the return memory of app::Clusters::MediaInput::Structs::InputInfoStruct::Type list, so encoder is used
    virtual CHIP_ERROR HandleGetInputList(app::AttributeValueEncoder & aEncoder)     = 0;
    virtual uint8_t HandleGetCurrentInput()                                          = 0;
    virtual bool HandleSelectInput(const uint8_t index)                              = 0;
    virtual bool HandleShowInputStatus()                                             = 0;
    virtual bool HandleHideInputStatus()                                             = 0;
    virtual bool HandleRenameInput(const uint8_t index, const chip::CharSpan & name) = 0;

    virtual ~Delegate() = default;
};

} // namespace MediaInput
} // namespace Clusters
} // namespace app
} // namespace chip
