/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifndef _SL_WFX_BOARD_H_
#define _SL_WFX_BOARD_H_
/*
 * Pull in the right board PINS
 */
#if defined(EFR32MG12_BRD4161A) || defined(BRD4161A) || defined(EFR32MG12_BRD4162A) || defined(BRD4162A) ||                        \
    defined(EFR32MG12_BRD4163A) || defined(BRD4163A) || defined(EFR32MG12_BRD4164A) || defined(BRD4164A) ||                        \
    defined(EFR32MG12_BRD4170A) || defined(BRD4170A)
#include "brd4161a.h"
#elif defined(EFR32MG24_BRD4186C) || defined(BRD4186C) || defined(EFR32MG24_BRD4186A) || defined(BRD4186A)
#include "brd4186c.h"
#elif defined(EFR32MG24_BRD4187C) || defined(BRD4187C) || defined(EFR32MG24_BRD4187A) || defined(BRD4187A)
#include "brd4187c.h"
#else
#error "Need SPI Pins"
#endif /* EFR32MG12_BRD4161A */
#endif /* _SL_WFX_BOARD_H_ */
