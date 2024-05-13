
/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include "app/util/attribute-storage.h"
#include "protocols/interaction_model/StatusCode.h"
#include "pw_status/status.h"

#define RETURN_STATUS_IF_NOT_OK(expr)                                                                                              \
    do                                                                                                                             \
    {                                                                                                                              \
        pw::Status __status = chip::rpc::ToPwStatus(expr);                                                                         \
        if (!__status.ok())                                                                                                        \
        {                                                                                                                          \
            return __status;                                                                                                       \
        }                                                                                                                          \
    } while (false)

namespace chip {
namespace rpc {

constexpr pw::Status ToPwStatus(Protocols::InteractionModel::Status ember_status)
{
    switch (ember_status)
    {
    case Protocols::InteractionModel::Status::Success:
        return pw::OkStatus();
    case Protocols::InteractionModel::Status::NotFound:
        return pw::Status::NotFound();
    case Protocols::InteractionModel::Status::UnsupportedAccess:
        return pw::Status::PermissionDenied();
    default:
        return pw::Status::Unknown();
    }
}

constexpr pw::Status ToPwStatus(CHIP_ERROR chip_error_status)
{
    switch (chip_error_status.AsInteger())
    {
    case CHIP_NO_ERROR.AsInteger():
        return pw::OkStatus();
    case CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute).AsInteger():
    case CHIP_IM_GLOBAL_STATUS(UnsupportedCommand).AsInteger():
    case CHIP_IM_GLOBAL_STATUS(UnsupportedEndpoint).AsInteger():
    case CHIP_IM_GLOBAL_STATUS(UnsupportedEvent).AsInteger():
        return pw::Status::NotFound();
    case CHIP_IM_GLOBAL_STATUS(UnsupportedAccess).AsInteger():
        return pw::Status::PermissionDenied();
    case CHIP_IM_GLOBAL_STATUS(InvalidAction).AsInteger():
    case CHIP_IM_GLOBAL_STATUS(InvalidCommand).AsInteger():
        return pw::Status::InvalidArgument();
    default:
        return pw::Status::Unknown();
    }
}

} // namespace rpc
} // namespace chip
