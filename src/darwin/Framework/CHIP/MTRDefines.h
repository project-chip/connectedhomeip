/**
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#pragma mark - Attribute macros

#define MTR_EXPORT __attribute__((visibility("default")))

#define MTR_HIDDEN __attribute__((visibility("hidden")))

#ifdef __cplusplus
#define MTR_EXTERN extern "C" MTR_EXPORT
#else
#define MTR_EXTERN extern MTR_EXPORT
#endif

#if __has_attribute(__swift_attr__)
#define MTR_SWIFT_DISFAVORED_OVERLOAD __attribute__((__swift_attr__("@_disfavoredOverload")))
#else
#define MTR_SWIFT_DISFAVORED_OVERLOAD
#endif

#pragma mark - Deprecation macros (can be overriden via build system)

#ifndef MTR_DEPRECATED
#define MTR_DEPRECATED(...) API_DEPRECATED(__VA_ARGS__) MTR_SWIFT_DISFAVORED_OVERLOAD
#endif

#ifndef MTR_DEPRECATED_WITH_REPLACEMENT
#define MTR_DEPRECATED_WITH_REPLACEMENT(...) API_DEPRECATED_WITH_REPLACEMENT(__VA_ARGS__) MTR_SWIFT_DISFAVORED_OVERLOAD
#endif

#ifndef MTR_NEWLY_DEPRECATED
#define MTR_NEWLY_DEPRECATED(message)
#endif

#ifndef MTR_NEWLY_AVAILABLE
#define MTR_NEWLY_AVAILABLE
#endif

#if !defined(MTR_ENABLE_PROVISIONAL)
#define MTR_ENABLE_PROVISIONAL 0
#endif

#if MTR_ENABLE_PROVISIONAL
#define MTR_PROVISIONALLY_AVAILABLE MTR_NEWLY_AVAILABLE
#else
#define MTR_PROVISIONALLY_AVAILABLE NS_UNAVAILABLE MTR_HIDDEN
#endif

#ifndef MTR_PER_CONTROLLER_STORAGE_ENABLED
#define MTR_PER_CONTROLLER_STORAGE_ENABLED 0
#endif

#pragma mark - Types

typedef NSData * MTRTLVBytes;
typedef NSData * MTRCSRDERBytes;
typedef NSData * MTRCertificateDERBytes;
typedef NSData * MTRCertificateTLVBytes;
