/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
/**
 *    @file
 *      This file defines messages in CHIP interaction model
 *
 */

#pragma once

#ifndef _CHIP_INTERACTION_MODEL_MESSAGE_DEF_H
#define _CHIP_INTERACTION_MODEL_MESSAGE_DEF_H

#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <util/basic-types.h>

namespace chip {
namespace app {
/**
 *  @brief
 *    The CHIP interaction model message types.
 *
 *  These values are called out in CHIP Interaction Model: Encoding Specification
 *
 */
enum
{
    kMsgType_SubscribeRequest      = 0x01,
    kMsgType_ReadRequest           = 0x02,
    kMsgType_ReportData            = 0x03,
    kMsgType_WriteRequest          = 0x04,
    kMsgType_WriteResponse         = 0x05,
    kMsgType_InvokeCommandRequest  = 0x06,
    kMsgType_InvokeCommandResponse = 0x07,
};

}; // namespace app
}; // namespace chip

#endif // _CHIP_INTERACTION_MODEL_MESSAGE_DEF_H
