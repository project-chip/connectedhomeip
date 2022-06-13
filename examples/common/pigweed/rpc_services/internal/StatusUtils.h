
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
#include "pw_status/status.h"

#define RETURN_STATUS_IF_NOT_OK(expr)                                                                                              \
    do                                                                                                                             \
    {                                                                                                                              \
        pw::Status __status = chip::rpc::EmberStatusToPwStatus(expr);                                                              \
        if (!__status.ok())                                                                                                        \
        {                                                                                                                          \
            return __status;                                                                                                       \
        }                                                                                                                          \
    } while (false)

namespace chip {
namespace rpc {

constexpr pw::Status EmberStatusToPwStatus(EmberAfStatus ember_status)
{
    switch (ember_status)
    {
    case EMBER_ZCL_STATUS_SUCCESS:
        return pw::OkStatus();
    case EMBER_ZCL_STATUS_NOT_FOUND:
        return pw::Status::NotFound();
    case EMBER_ZCL_STATUS_NOT_AUTHORIZED:
        return pw::Status::PermissionDenied();
    default:
        return pw::Status::Unknown();
    }
}

} // namespace rpc
} // namespace chip
