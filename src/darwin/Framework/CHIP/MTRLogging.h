/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <os/log.h>

#define MTR_LOG_FAULT(format, ...) os_log_fault(OS_LOG_DEFAULT, format, ##__VA_ARGS__)
#define MTR_LOG_ERROR(format, ...) os_log_error(OS_LOG_DEFAULT, format, ##__VA_ARGS__)
#define MTR_LOG_INFO(format, ...) os_log_info(OS_LOG_DEFAULT, format, ##__VA_ARGS__)
#define MTR_LOG_DEBUG(format, ...) os_log_debug(OS_LOG_DEFAULT, format, ##__VA_ARGS__)

#define MTR_LOG_METHOD_ENTRY()                                                                                                     \
    ({ os_log_debug(OS_LOG_DEFAULT, "[<%@: %p> %@]", NSStringFromClass([self class]), self, NSStringFromSelector(_cmd)); })
