/**
 *    Copyright (c) 2023-2024 Project CHIP Authors
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

#ifdef DEBUG
#define MTR_TESTABLE MTR_EXPORT
#define MTR_TESTABLE_DIRECT
#define MTR_TESTABLE_DIRECT_MEMBERS
#else
#define MTR_TESTABLE
#define MTR_TESTABLE_DIRECT MTR_DIRECT
#define MTR_TESTABLE_DIRECT_MEMBERS MTR_DIRECT_MEMBERS
#endif

// clang-format off
/// Creates a weak shadow copy of the variable `local`
#define mtr_weakify(local)                                      \
    __weak typeof(local) _mtr_weak_##local = local

/// Copies the weak shadow copy of `local` created by `mtr_weakify`
/// back into a strong variable of the same name.
#define mtr_strongify(local)                                    \
    _Pragma("clang diagnostic push")                            \
    _Pragma("clang diagnostic ignored \"-Wshadow\"")            \
    __strong typeof(local) _Nullable local = _mtr_weak_##local  \
    _Pragma("clang diagnostic pop")

/// Declares an unused local of unspecified type, to prevent accidental
/// references to a shadowed variable of the same name. Note that hiding
/// `self` does not prevent implicit references to self due to ivar access.
#define mtr_hide(local)                                         \
    _Pragma("clang diagnostic push")                            \
    _Pragma("clang diagnostic ignored \"-Wshadow\"")            \
    __attribute__((unused)) variable_hidden_by_mtr_hide local;  \
    _Pragma("clang diagnostic pop")
typedef struct {} variable_hidden_by_mtr_hide;
// clang-format on

// mtr_[un]likely(expr): Evaluates a boolean expression and hints to the compiler that it is [un]likely to be true.
#define mtr_likely(expr) __builtin_expect(!!(expr), 1)
#define mtr_unlikely(expr) __builtin_expect(!!(expr), 0)

// Default timed interaction timeout, in ms, if another one is not provided.
#define MTR_DEFAULT_TIMED_INTERACTION_TIMEOUT_MS 10000

// Useful building block for type-checking machinery.  Uses C-style cast so it
// can be used in .m files as well.
#define MTR_SAFE_CAST(object, classname) \
    ([object isKindOfClass:[classname class]] ? (classname *) (object) : nil)

#pragma mark - XPC Defines

#define MTR_SIMPLE_REMOTE_XPC_GETTER(XPC_CONNECTION, NAME, TYPE, DEFAULT_VALUE, GETTER_NAME, PREFIX)   \
                                                                                                       \
    -(TYPE) NAME                                                                                       \
    {                                                                                                  \
        __block TYPE outValue = DEFAULT_VALUE;                                                         \
                                                                                                       \
        NSXPCConnection * xpcConnection = XPC_CONNECTION;                                              \
                                                                                                       \
        @try {                                                                                         \
            [[xpcConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError * _Nonnull error) { \
                MTR_LOG_ERROR("%@ Error in %@ getter: %@", self, NSStringFromSelector(_cmd), error);   \
            }] PREFIX                                                                                  \
                GETTER_NAME:^(TYPE returnValue) {                                                      \
                    outValue = returnValue;                                                            \
                }];                                                                                    \
        } @catch (NSException * exception) {                                                           \
            MTR_LOG_ERROR("%@ Exception sending XPC messsage for %@ getter: %@", self,                 \
                NSStringFromSelector(_cmd), exception);                                                \
            outValue = DEFAULT_VALUE;                                                                  \
        }                                                                                              \
        return outValue;                                                                               \
    }

#define MTR_SIMPLE_REMOTE_XPC_COMMAND(XPC_CONNECTION, METHOD_SIGNATURE, ADDITIONAL_ARGUMENTS, PREFIX) \
                                                                                                      \
    -(void) METHOD_SIGNATURE                                                                          \
    {                                                                                                 \
        NSXPCConnection * xpcConnection = XPC_CONNECTION;                                             \
                                                                                                      \
        @try {                                                                                        \
            [[xpcConnection remoteObjectProxyWithErrorHandler:^(NSError * _Nonnull error) {           \
                MTR_LOG_ERROR("%@ Error in %@: %@", self, NSStringFromSelector(_cmd), error);         \
            }] PREFIX ADDITIONAL_ARGUMENTS];                                                          \
        } @catch (NSException * exception) {                                                          \
            MTR_LOG_ERROR("%@ Exception sending XPC messsage for %@: %@", self,                       \
                NSStringFromSelector(_cmd), exception);                                               \
        }                                                                                             \
    }

#define MTR_COMPLEX_REMOTE_XPC_GETTER(XPC_CONNECTION, SIGNATURE, TYPE, DEFAULT_VALUE, ADDITIONAL_ARGUMENTS, PREFIX) \
    -(TYPE) SIGNATURE                                                                                               \
    {                                                                                                               \
        __block TYPE outValue = DEFAULT_VALUE;                                                                      \
                                                                                                                    \
        NSXPCConnection * xpcConnection = XPC_CONNECTION;                                                           \
                                                                                                                    \
        @try {                                                                                                      \
            [[xpcConnection synchronousRemoteObjectProxyWithErrorHandler:^(NSError * _Nonnull error) {              \
                MTR_LOG_ERROR("%@ Error in %@: %@", self, NSStringFromSelector(_cmd), error);                       \
            }] PREFIX ADDITIONAL_ARGUMENTS:^(TYPE returnValue) {                                                    \
                outValue = returnValue;                                                                             \
            }];                                                                                                     \
        } @catch (NSException * exception) {                                                                        \
            MTR_LOG_ERROR("%@ Exception sending XPC messsage for %@: %@", self, NSStringFromSelector(_cmd),         \
                exception);                                                                                         \
            outValue = DEFAULT_VALUE;                                                                               \
        }                                                                                                           \
                                                                                                                    \
        return outValue;                                                                                            \
    }

#ifndef MTR_OPTIONAL_ATTRIBUTE
#if __has_feature(objc_arc)
#define MTR_OPTIONAL_ATTRIBUTE(ATTRIBUTE, VALUE, DICTIONARY)                                                                                       \
    {                                                                                                                                              \
        id valueToAdd = VALUE;                                                                                                                     \
        if (valueToAdd != nil) {                                                                                                                   \
            CFDictionarySetValue((CFMutableDictionaryRef) DICTIONARY, (CFStringRef) (__bridge const void *) ATTRIBUTE, (const void *) valueToAdd); \
        }                                                                                                                                          \
    }
#else
#define MTR_OPTIONAL_ATTRIBUTE(ATTRIBUTE, VALUE, DICTIONARY)                                                                              \
    {                                                                                                                                     \
        id valueToAdd = VALUE;                                                                                                            \
        if (valueToAdd != nil) {                                                                                                          \
            CFDictionarySetValue((CFMutableDictionaryRef) DICTIONARY, (CFStringRef) (const void *) ATTRIBUTE, (const void *) valueToAdd); \
        }                                                                                                                                 \
    }
#endif
#endif

#ifndef MTR_REMOVE_ATTRIBUTE
#define MTR_REMOVE_ATTRIBUTE(ATTRIBUTE, DICTIONARY)                                            \
    if (ATTRIBUTE != nil && DICTIONARY) {                                                      \
        CFDictionaryRemoveValue((CFMutableDictionaryRef) DICTIONARY, (CFStringRef) ATTRIBUTE); \
    }
#endif

#ifndef MTR_REQUIRED_ATTRIBUTE
#define MTR_REQUIRED_ATTRIBUTE(ATTRIBUTE, VALUE, DICTIONARY)                                                               \
    {                                                                                                                      \
        id valueToAdd = VALUE;                                                                                             \
        if (valueToAdd != nil) {                                                                                           \
            CFDictionarySetValue((CFMutableDictionaryRef) DICTIONARY, (CFStringRef) ATTRIBUTE, (const void *) valueToAdd); \
        } else {                                                                                                           \
            MTR_LOG_ERROR("Warning, missing %@ to add to %s", ATTRIBUTE, #DICTIONARY);                                     \
        }                                                                                                                  \
    }
#endif

#define MTR_YES_NO(x) ((x) ? @"YES" : @"NO")

#ifdef DEBUG
#define _MTR_ABSTRACT_METHOD_IMPL(message, ...)                                                                                                        \
    do {                                                                                                                                               \
        MTR_LOG_ERROR(message, __VA_ARGS__);                                                                                                           \
        @throw [NSException exceptionWithName:NSInternalInconsistencyException reason:[NSString stringWithFormat:@message, __VA_ARGS__] userInfo:nil]; \
    } while (0)
#else // DEBUG
#define _MTR_ABSTRACT_METHOD_IMPL(message, ...) \
    MTR_LOG_ERROR(message, __VA_ARGS__)
#endif // DEBUG

#define MTR_ABSTRACT_METHOD() \
    _MTR_ABSTRACT_METHOD_IMPL("%@ or some ancestor must implement %@", self.class, NSStringFromSelector(_cmd))

#pragma mark - Typedefs for some commonly used types.

/**
 * A data-value as defined in MTRBaseDevice.h.
 */
typedef NSDictionary<NSString *, id> * MTRDeviceDataValueDictionary;

/**
 * A response-value as defined in MTRBaseDevice.h.
 */
typedef NSDictionary<NSString *, id> * MTRDeviceResponseValueDictionary;
