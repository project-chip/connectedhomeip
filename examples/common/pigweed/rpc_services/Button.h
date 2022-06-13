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
