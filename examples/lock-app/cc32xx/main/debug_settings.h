/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
