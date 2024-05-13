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

#include <lib/core/TLV.h>

namespace chip {
namespace app {
/**
 * An EventLoggingDelegate is used to fill event log data with cluster-specific information.
 *
 * Allows application to append any type of TLV data as part of an event log entry. Events
 * have a standard header applicable to all events and this class provides the
 * ability to add additional data past such standard header.
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
     *  @param[in,out] aWriter A reference to the chip::TLV::TLVWriter object to be
     *                         used for event data serialization.
     *
     *  @retval #CHIP_NO_ERROR  On success.
     *
     *  @retval other           An appropriate error signaling to the
     *                          caller that the serialization of event
     *                          data could not be completed.
     *
     */
    virtual CHIP_ERROR WriteEvent(chip::TLV::TLVWriter & aWriter) = 0;
};
} // namespace app
} // namespace chip
