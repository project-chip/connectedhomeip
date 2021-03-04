/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file defines the classes corresponding to CHIP Interaction Model Event Generatorr Delegate.
 *
 */

#pragma once

#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <messaging/ExchangeContext.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace app {
namespace reporting {
/**
 * @brief
 *   This class provides a delegate to write events
 */
class EventLoggingDelegate
{
public:
    virtual ~EventLoggingDelegate() {}

    /**
     *  @brief
     *    A function that supplies eventData element for the event logging subsystem.
     *
     *  Functions of this type are expected to provide the eventData
     *  element for the event logging subsystem. The functions of this
     *  type are called after the event subsystem has generated all
     *  required event metadata. The function is called with a
     *  chip::TLV::TLVWriter object into which it will emit a single TLV element
     *  tagged kTag_EventData; the value of that element MUST be a
     *  structure containing the event data. The event data itself must
     *  be structured using context tags.
     *
     *
     *  @param[inout] aWriter A reference to the chip::TLV::TLVWriter object to be
     *                         used for event data serialization.
     *
     *  @retval #CHIP_NO_ERROR  On success.
     *
     *  @retval other           An appropriate error signaling to the
     *                          caller that the serialization of event
     *                          data could not be completed. Errors from
     *                          calls to the aWriter should be propagated
     *                          without remapping. If the function
     *                          returns any type of error, the event
     *                          generation is aborted, and the event is not
     *                          written to the log.
     *
     */
    virtual CHIP_ERROR WriteEvent(chip::TLV::TLVWriter & aWriter) = 0;
};
} // namespace reporting
} // namespace app
} // namespace chip
