/*******************************************************************************
 * File Name: cycfg_notices.h
 *
 * Description:
 * Contains warnings and errors that occurred while generating code for the
 * design.
 * This file was automatically generated and should not be modified.
 * Configurator Backend 3.10.0
 * device-db 4.100.0.4304
 * mtb-pdl-cat1 3.9.0.29592
 *
 ********************************************************************************
 * Copyright 2024 Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ********************************************************************************/

#if !defined(CYCFG_NOTICES_H)
#define CYCFG_NOTICES_H

#ifdef CY_SUPPORTS_DEVICE_VALIDATION
#ifndef CY8C624ABZI_S2D44
#error                                                                                                                             \
    "Unexpected MPN; expected DEVICE:=CY8C624ABZI-S2D44. There may be an inconsistency between the *.modus file and the makefile target configuration device sets."
#endif
#endif

#ifdef CY_SUPPORTS_COMPLETE_DEVICE_VALIDATION
#ifndef CY8C624ABZI_S2D44
#error                                                                                                                             \
    "Unexpected MPN; expected DEVICE:=CY8C624ABZI-S2D44, ADDITIONAL_DEVICES:=CYW43012C0WKWBG. There may be an inconsistency between the *.modus file and the makefile target configuration device sets."
#endif
#ifndef CYW43012C0WKWBG
#error                                                                                                                             \
    "Unexpected MPN; expected DEVICE:=CY8C624ABZI-S2D44, ADDITIONAL_DEVICES:=CYW43012C0WKWBG. There may be an inconsistency between the *.modus file and the makefile target configuration device sets."
#endif
#endif

#endif /* CYCFG_NOTICES_H */
