/*
 *
 *    Copyright (c) 2019 Google LLC
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

/**
 *    @file
 *          Utilities for working with the Nordic nRF5 SDK.
 */

#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>
#include <Weave/DeviceLayer/nRF5/nRF5Utils.h>

namespace nl {
namespace Weave {
namespace DeviceLayer {
namespace Internal {

/**
 * Register a text error formatter for nRF SDK errors.
 */
void RegisterNRFErrorFormatter(void)
{
    static ErrorFormatter sNRFErrorFormatter =
    {
        FormatNRFError,
        NULL
    };

    RegisterErrorFormatter(&sNRFErrorFormatter);
}

/**
 * Given an nRF SDK error, returns a human-readable NULL-terminated C string
 * describing the error.
 *
 * @param[in] buf                   Buffer into which the error string will be placed.
 * @param[in] bufSize               Size of the supplied buffer in bytes.
 * @param[in] err                   The error to be described.
 *
 * @return true                     If a descriptions string was written into the supplied buffer.
 * @return false                    If the supplied error was not an nRF SDK error.
 *
 */
bool FormatNRFError(char * buf, uint16_t bufSize, int32_t err)
{
    const char * subsys = NULL;

    if (err >= WEAVE_DEVICE_CONFIG_NRF5_ERROR_MIN && err <= WEAVE_DEVICE_CONFIG_NRF5_ERROR_MAX)
    {
        subsys = "nRF";
    }

    else if (err >= WEAVE_DEVICE_CONFIG_NRF5_FDS_ERROR_MIN && err <= WEAVE_DEVICE_CONFIG_NRF5_FDS_ERROR_MAX)
    {
        subsys = "nRF-FDS";
    }

    else
    {
        return false;
    }

    nl::FormatError(buf, bufSize, subsys, err, NULL);

    return true;
}


} // namespace Internal
} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

