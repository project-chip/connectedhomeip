/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
 *  To enable these credentias, compile the app with option
 *  "chip_build_device_attestation_credentials=true".
 */

#ifndef SL_PROVISION_VERSION_1_0
#define SL_PROVISION_VERSION_1_0 0
#endif

#ifndef SL_CREDENTIALS_DAC_OFFSET
#define SL_CREDENTIALS_DAC_OFFSET 0x0000
#endif

#ifndef SL_CREDENTIALS_DAC_SIZE
#define SL_CREDENTIALS_DAC_SIZE 0
#endif

#ifndef SL_CREDENTIALS_PAI_OFFSET
#define SL_CREDENTIALS_PAI_OFFSET 0x0200
#endif

#ifndef SL_CREDENTIALS_PAI_SIZE
#define SL_CREDENTIALS_PAI_SIZE 0
#endif

#ifndef SL_CREDENTIALS_CD_OFFSET
#define SL_CREDENTIALS_CD_OFFSET 0x0400
#endif

#ifndef SL_CREDENTIALS_CD_SIZE
#define SL_CREDENTIALS_CD_SIZE 0
#endif
