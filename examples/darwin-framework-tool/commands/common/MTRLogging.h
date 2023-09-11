/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CHIPLogging_h
#define CHIPLogging_h

#import <os/log.h>

#if DEBUG
#define MTR_LOG_DEBUG(format, ...) os_log(OS_LOG_DEFAULT, format, ##__VA_ARGS__)
#define MTR_LOG_ERROR(format, ...) os_log(OS_LOG_DEFAULT, format, ##__VA_ARGS__)
#define MTR_LOG_METHOD_ENTRY()                                                                                                     \
    ({ os_log(OS_LOG_DEFAULT, "[<%@: %p> %@]", NSStringFromClass([self class]), self, NSStringFromSelector(_cmd)); })

#else
#define MTR_LOG_DEBUG(...)
#define MTR_LOG_ERROR(...)
#define MTR_LOG_METHOD_ENTRY() ({})

#endif

#endif /* CHIPLogging_h */
