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

//*****************************************************************************
// Includes
//*****************************************************************************
// Standard includes
#ifndef DEBUG_SETTINGS_H
#define DEBUG_SETTINGS_H

// Select the print method used in the main app
#define D_DEBUG_METHOD D_USER_DEFINED

// Select Sevrity Color
#define _FATAL_CLR_ _CLR_B_RED_
#define _ERROR_CLR_ _CLR_RED_
#define _WARNING_CLR_ _CLR_MAGENTA_
#define _INFO_CLR_ _CLR_GREEN_
#define _DEBUG_CLR_ _CLR_YELLOW_
#define _TRACE_CLR_ _CLR_RESET_

#if (D_DEBUG_METHOD == D_USER_DEFINED)
extern void cc32xxLog(const char * msg, ...);
#define PRINTF(...) cc32xxLog(__VA_ARGS__);
#endif

#endif // DEBUG_SETTINGS_H
