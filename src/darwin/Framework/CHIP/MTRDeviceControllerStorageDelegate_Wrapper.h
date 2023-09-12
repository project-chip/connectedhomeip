/**
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <Matter/MTRDefines.h>

#if MTR_PER_CONTROLLER_STORAGE_ENABLED
#error Should be including Matter/MTRDeviceControllerStorageDelegate.h
#endif // MTR_PER_CONTROLLER_STORAGE_ENABLED

#define MTR_INTERNAL_INCLUDE
#import <Matter/MTRDeviceControllerStorageDelegate.h>
#undef MTR_INTERNAL_INCLUDE
