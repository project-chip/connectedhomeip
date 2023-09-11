/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "button_service/button_service.rpc.pb.h"

namespace chip {
namespace rpc {

class Button : public pw_rpc::nanopb::Button::Service<Button>
{
public:
    virtual ~Button() = default;

    virtual pw::Status Event(const chip_rpc_ButtonEvent & request, pw_protobuf_Empty & response)
    {
        return pw::Status::Unimplemented();
    }
};

} // namespace rpc
} // namespace chip
