/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Matter/MTRLogging.h>

#include <lib/support/logging/CHIPLogging.h>

#define MTR_LOG_ERROR(msg, ...) ChipLogError(NotSpecified, msg, ##__VA_ARGS__)
#define MTR_LOG_DEFAULT(msg, ...) ChipLogProgress(NotSpecified, msg, ##__VA_ARGS__)
#define MTR_LOG_INFO(msg, ...) ChipLogDetail(NotSpecified, msg, ##__VA_ARGS__)
#define MTR_LOG_DEBUG(msg, ...) ChipLogDetail(NotSpecified, msg, ##__VA_ARGS__) // same as INFO
