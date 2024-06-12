/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef OTA_H
#define OTA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <platform/stm32/FactoryDataProvider.h>
#include <platform/stm32/OTAImageProcessorImpl.h>
#include <stdint.h>

/*! Attribute structure */
typedef struct
{
    uint16_t vendorId;             /*! VendorId info from image header */
    uint16_t productId;            /*! ProductId info from image header */
    uint32_t softwareVersion;      /*! Software version of the binary */
    uint32_t minApplicableVersion; /*! Minimum running software version to be compatible with the OTA image */
    uint32_t maxApplicableVersion; /*! Maximum running software version to be compatible with the OTA image */
} Ota_ImageHeader_t;

void InitializeOTARequestor(void);
bool OtaHeaderValidation(Ota_ImageHeader_t imageHeader);
void TriggerOTAQuery(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* OTA_H */
