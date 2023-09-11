/**
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Matter/MTRDefines.h>

#if __has_attribute(objc_direct)
#define MTR_DIRECT __attribute__((objc_direct))
#else
#define MTR_DIRECT
#endif

#if __has_attribute(objc_direct_members)
#define MTR_DIRECT_MEMBERS __attribute__((objc_direct_members))
#else
#define MTR_DIRECT_MEMBERS
#endif
