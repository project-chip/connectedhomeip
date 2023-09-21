/**
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

/**
 * MTR_NO_AVAILABILITY can be used to turn off availability
 * annotations, to allow compiling a version of Matter.framework for "local
 * use", not as a system framework.
 */
#if !defined(MTR_NO_AVAILABILITY)
#define MTR_NO_AVAILABILITY 0
#endif

#if MTR_NO_AVAILABILITY
#define MTR_DEPRECATED(...) MTR_SWIFT_DISFAVORED_OVERLOAD
#define MTR_DEPRECATED_WITH_REPLACEMENT(...) MTR_SWIFT_DISFAVORED_OVERLOAD
#define MTR_AVAILABLE(...)
#else
#define MTR_DEPRECATED(...) API_DEPRECATED(__VA_ARGS__) MTR_SWIFT_DISFAVORED_OVERLOAD
#define MTR_DEPRECATED_WITH_REPLACEMENT(...) API_DEPRECATED_WITH_REPLACEMENT(__VA_ARGS__) MTR_SWIFT_DISFAVORED_OVERLOAD
#define MTR_AVAILABLE(...) API_AVAILABLE(__VA_ARGS__)
#endif // MTR_NO_AVAILABILITY

#define MTR_NEWLY_DEPRECATED(message)

#define MTR_NEWLY_AVAILABLE

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
