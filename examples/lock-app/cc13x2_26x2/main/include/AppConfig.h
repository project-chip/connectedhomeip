/*
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// Logging
#ifdef __cplusplus
extern "C" {
#endif

int cc13x2_26x2LogInit(void);
void cc13x2_26x2Log(const char * aFormat, ...);
#define PLAT_LOG(...) cc13x2_26x2Log(__VA_ARGS__);

#ifdef __cplusplus
}
#endif
#endif // APP_CONFIG_H
