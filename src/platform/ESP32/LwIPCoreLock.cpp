/*
 *
 *    Copyright (c) 2018 Nest Labs, Inc.
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

#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>

namespace {

SemaphoreHandle_t LwIPCoreLock;

}

namespace nl {
namespace Weave {
namespace DeviceLayer {
namespace Internal {

WEAVE_ERROR InitLwIPCoreLock(void)
{
    if (LwIPCoreLock == NULL)
    {
        LwIPCoreLock = xSemaphoreCreateMutex();
        if (LwIPCoreLock == NULL)
        {
            WeaveLogError(DeviceLayer, "Failed to create LwIP core lock");
            return WEAVE_ERROR_NO_MEMORY;
        }
    }

    return WEAVE_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

extern "C" void lock_lwip_core()
{
    xSemaphoreTake(LwIPCoreLock, portMAX_DELAY);
}

extern "C" void unlock_lwip_core()
{
    xSemaphoreGive(LwIPCoreLock);
}
