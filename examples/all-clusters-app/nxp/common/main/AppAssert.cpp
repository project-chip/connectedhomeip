/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    Copyright 2023 NXP
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

#include "fsl_debug_console.h"
#include <platform/CHIPDeviceLayer.h>

/*
 * Assert function implemented in the application layer.
 * This implementation would produce a reset.
 * But it could be extended with specific application contrains/requirements.
 */
void __assert_func(const char * file, int line, const char * func, const char * failedExpr)
{
    PRINTF("ASSERT ERROR \" %s \": file \"%s\" Line \"%d\" function name \"%s\" \n", failedExpr, file, line, func);
    chip::DeviceLayer::PlatformMgrImpl().Reset();
    while (1)
        ;
}
