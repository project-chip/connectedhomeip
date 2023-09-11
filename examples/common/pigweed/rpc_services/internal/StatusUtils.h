
/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
    case EMBER_ZCL_STATUS_UNSUPPORTED_ACCESS:
        return pw::Status::PermissionDenied();
    default:
        return pw::Status::Unknown();
    }
}

} // namespace rpc
} // namespace chip
