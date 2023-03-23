/*
 *    Copyright (c) 2020 Project CHIP Authors
<<<<<<< HEAD:examples/pump-controller-app/cc13x4_26x4/main/include/AppConfig.h
 *    Copyright (c) 2019 Google LLC.
=======
 *    Copyright (c) 2022 Silabs.
>>>>>>> refs/tags/v1.0.0.2:examples/platform/silabs/SiWx917/siwx917_utils.h
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

<<<<<<< HEAD:examples/pump-controller-app/cc13x4_26x4/main/include/AppConfig.h
// Logging
=======
// SiWx917 Logging
>>>>>>> refs/tags/v1.0.0.2:examples/platform/silabs/SiWx917/siwx917_utils.h
#ifdef __cplusplus
extern "C" {
#endif

<<<<<<< HEAD:examples/pump-controller-app/cc13x4_26x4/main/include/AppConfig.h
int cc13xx_26xxLogInit(void);
void cc13xx_26xxLog(const char * aFormat, ...);
#define PLAT_LOG(...) cc13xx_26xxLog(__VA_ARGS__);
=======
void silabsInitLog(void);

void efr32Log(const char * aFormat, ...);
#define SILABS_LOG(...) efr32Log(__VA_ARGS__);
void appError(int err);
>>>>>>> refs/tags/v1.0.0.2:examples/platform/silabs/SiWx917/siwx917_utils.h

#ifdef __cplusplus
}

#include <lib/core/CHIPError.h>
void appError(CHIP_ERROR error);
#endif
#endif // APP_CONFIG_H
