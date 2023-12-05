/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#ifndef CHIPLogging_h
#define CHIPLogging_h

#import <os/log.h>

#ifdef DEBUG
#define MTR_LOG_DEBUG(format, ...) os_log(OS_LOG_DEFAULT, format, ##__VA_ARGS__)
#define MTR_LOG_ERROR(format, ...) os_log(OS_LOG_DEFAULT, format, ##__VA_ARGS__)
#define MTR_LOG_METHOD_ENTRY() \
    ({ os_log(OS_LOG_DEFAULT, "[<%@: %p> %@]", NSStringFromClass([self class]), self, NSStringFromSelector(_cmd)); })

#else
#define MTR_LOG_DEBUG(...)
#define MTR_LOG_ERROR(...)
#define MTR_LOG_METHOD_ENTRY() ({})

#endif

#endif /* CHIPLogging_h */
