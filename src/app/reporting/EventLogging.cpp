/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
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

#include "EventLogging.h"
#include "EventLoggingTypes.h"
#include "LoggingManagement.h"
#include <app/MessageDef/EventDataElement.h>
#include <core/CHIPCore.h>
#include <core/CHIPEncoding.h>
#include <core/CHIPSafeCasts.h>
#include <core/CHIPTLV.h>
#include <stdarg.h>
#include <support/Base64.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/logging/CHIPLogging.h>

using namespace chip::TLV;

namespace chip {
namespace app {
namespace reporting {

/**
 * @brief
 *   A helper function that translates an already serialized eventdata element into the event buffer.
 *
 * @param[inout] aWriter The writer to use for writing out the event
 *
 * @param[in] aDataTag   A context tag for the TLV we're copying out.  Unused here,
 *                        but required by the typedef for EventWriterFunct.
 *
 * @param[in] appData     A pointer to the TLVReader that holds serialized event data.
 *
 * @retval #CHIP_NO_ERROR On success.
 *
 * @retval other          Other errors that mey be returned from the aWriter.
 *
 */
static CHIP_ERROR EventWriterTLVCopy(TLVWriter & aWriter, void * appData)
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    TLVReader * reader = static_cast<TLVReader *>(appData);

    VerifyOrExit(reader != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);

    err = reader->Next();
    SuccessOrExit(err);

    err = aWriter.CopyElement(chip::TLV::ContextTag(chip::app::EventDataElement::kCsTag_Data), *reader);

exit:
    return err;
}

chip::EventNumber LogEvent(const EventSchema & aSchema, TLVReader & aData)
{
    return LogEvent(aSchema, aData, NULL);
}

chip::EventNumber LogEvent(const EventSchema & aSchema, TLVReader & aData, const EventOptions * apOptions)
{
    return LogEvent(aSchema, EventWriterTLVCopy, &aData, apOptions);
}

chip::EventNumber LogEvent(const EventSchema & aSchema, EventWriterFunct aEventWriter, void * apAppData)
{
    return LogEvent(aSchema, aEventWriter, apAppData, NULL);
}

chip::EventNumber LogEvent(const EventSchema & aSchema, EventWriterFunct aEventWriter, void * apAppData,
                           const EventOptions * apOptions)
{

    LoggingManagement & logManager = LoggingManagement::GetInstance();

    return logManager.LogEvent(aSchema, aEventWriter, apAppData, apOptions);
}

} // namespace reporting
} // namespace app
} // namespace chip
