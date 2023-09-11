/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "window.h"

#include <stdint.h>
#include <string>

#include <app/data-model/Nullable.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>

#include <app-common/zap-generated/cluster-enums.h>

namespace example {
namespace Ui {
namespace Windows {

class BooleanState : public Window
{
public:
    BooleanState(chip::EndpointId endpointId, const char * title) : mEndpointId(endpointId), mTitle(title) {}

    void UpdateState() override;
    void Render() override;

private:
    const chip::EndpointId mEndpointId;
    const std::string mTitle;

    bool mState = false;
    chip::Optional<bool> mTargetState;
};

} // namespace Windows
} // namespace Ui
} // namespace example
