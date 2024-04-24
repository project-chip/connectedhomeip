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

#pragma mark - Overridable options

/**
 * @define `MTR_NO_AVAILABILITY`
 * Turns off availability annotations, to allow compiling a version of
 * Matter.framework for "local use", not as a system framework.
 */
#ifndef MTR_NO_AVAILABILITY
#define MTR_NO_AVAILABILITY 0
#endif

/**
 * @define `MTR_ENABLE_PROVISIONAL`
 * Makes provisional spec features available for evaluation / testing purposes.
 * The usual API / ABI stability guarantees DO NOT APPLY to provisional features.
 */
#ifndef MTR_ENABLE_PROVISIONAL
#define MTR_ENABLE_PROVISIONAL 0
#endif

/**
 * @define `MTR_ENABLE_UNSTABLE_API`
 * Makes unstbale APIs available for evaluation / testing purposes.
 * The usual API / ABI stability guarantees DO NOT APPLY to unstable APIs.
 */
#ifndef MTR_ENABLE_UNSTABLE_API
#define MTR_ENABLE_UNSTABLE_API 0
#endif

#pragma mark - Attribute macros

#define MTR_EXPORT __attribute__((visibility("default")))

#ifdef __cplusplus
#define MTR_EXTERN extern "C"
#else
#define MTR_EXTERN extern
#endif

#if __has_attribute(__swift_attr__)
#define MTR_SWIFT_DISFAVORED_OVERLOAD __attribute__((__swift_attr__("@_disfavoredOverload")))
#else
#define MTR_SWIFT_DISFAVORED_OVERLOAD
#endif

#pragma mark - Availability / deprecation

// clang-format off
#define _MTR_IMPLICIT_EXPORT                                    \
    _Pragma("clang diagnostic push")                            \
    _Pragma("clang diagnostic ignored \"-Wignored-attributes\"")\
    MTR_EXPORT                                                  \
    _Pragma("clang diagnostic pop")
// clang-format on

#define _MTR_DEPRECATED _MTR_IMPLICIT_EXPORT MTR_SWIFT_DISFAVORED_OVERLOAD
#define _MTR_UNAVAILABLE _MTR_IMPLICIT_EXPORT NS_UNAVAILABLE

// clang-format off
#if MTR_NO_AVAILABILITY
#define MTR_DEPRECATED(...)                     _MTR_DEPRECATED
#define MTR_DEPRECATED_WITH_REPLACEMENT(...)    _MTR_DEPRECATED
#define MTR_AVAILABLE(...)                      _MTR_IMPLICIT_EXPORT
#else
#define MTR_DEPRECATED(...)                     _MTR_DEPRECATED API_DEPRECATED(__VA_ARGS__)
#define MTR_DEPRECATED_WITH_REPLACEMENT(...)    _MTR_DEPRECATED API_DEPRECATED_WITH_REPLACEMENT(__VA_ARGS__)
#define MTR_AVAILABLE(...)                      _MTR_IMPLICIT_EXPORT API_AVAILABLE(__VA_ARGS__)
#endif // MTR_NO_AVAILABILITY
// clang-format on

#define MTR_NEWLY_DEPRECATED(message) _MTR_IMPLICIT_EXPORT
#define MTR_NEWLY_AVAILABLE _MTR_IMPLICIT_EXPORT

#if MTR_ENABLE_PROVISIONAL
#define MTR_PROVISIONALLY_AVAILABLE MTR_NEWLY_AVAILABLE
#else
#define MTR_PROVISIONALLY_AVAILABLE _MTR_UNAVAILABLE
#endif

#if MTR_ENABLE_UNSTABLE_API
#define MTR_UNSTABLE_API MTR_NEWLY_AVAILABLE
#else
#define MTR_UNSTABLE_API _MTR_UNAVAILABLE
#endif

#pragma mark - Types

typedef NSData * MTRTLVBytes;
typedef NSData * MTRCSRDERBytes;
typedef NSData * MTRCertificateDERBytes;
typedef NSData * MTRCertificateTLVBytes;
