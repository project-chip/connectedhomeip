// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef CHIP_BASE_COMPILER_ABSTRACTION_H
#define CHIP_BASE_COMPILER_ABSTRACTION_H

#ifdef __cplusplus
extern "C" {
#endif

// Fallback for compilers that do not support querying for attributes.
#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

// Fallback for compilers that do not support querying for builtins.
#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

// Fallback for compilers that do not support querying for extensions.
#ifndef __has_extension
#define __has_extension(x) 0
#endif

// Fallback for compilers that do not support querying for features.
#ifndef __has_feature
#define __has_feature(x) 0
#endif

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

// Fallback for compilers that do not support the nullability feature.
#if !__has_feature(nullability)
#define _Nonnull
#define _Nullable
#endif

// Fallback for compilers that do not support the inline keyword.
#if defined(_MSC_VER)
#define inline __inline
#endif

// Fallback for compilers that do not support the __func__ definition.
#if defined(_MSC_VER)
#define __func__ __FUNCTION__
#endif

// Fallback if CHIP_FILE is not defined. __FILE__ may include absolute paths.
#ifndef CHIP_FILE
#define CHIP_FILE __FILE__
#endif

//----------------------------------------------------------------------------------------------------------------------

/**
 * Specifies the minimum alignment of a variable or field.
 *
 * @param      numBytes             Minimum alignment in bytes.
 */
/**@{*/
#if defined(_MSC_VER)
#define CHIP_ALIGNAS(numBytes) __declspec(align(numBytes))
#elif __has_attribute(aligned) || (defined(__GNUC__) && (__GNUC__ >= 5 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2))) ||              \
    defined(__CC_ARM) || defined(__ICCARM__)

#define CHIP_ALIGNAS(numBytes) __attribute__((aligned(numBytes)))
#else
// This attribute is not optional.
#endif
/**@}*/

/**
 * Indicates that a function has no side effects and that the return value depends solely on its parameters.
 *
 * - The function may NOT read global variables and may NOT dereference pointer arguments.
 */
/**@{*/
#if __has_attribute(const) || (defined(__GNUC__) && (__GNUC__ >= 3 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 5))) ||                  \
    defined(__CC_ARM) || defined(__ICCARM__)

#define CHIP_CONST __attribute__((const))
#else
#define CHIP_CONST
#endif
/**@}*/

/**
 * Marks code as deprecated. A warning is emitted if the code is used.
 */
/**@{*/
#if __has_attribute(deprecated) || (defined(__GNUC__) && (__GNUC__ >= 5 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))) ||             \
    defined(__CC_ARM) || defined(__ICCARM__)

#define CHIP_DEPRECATED __attribute__((deprecated))
#else
#define CHIP_DEPRECATED
#endif
/**@}*/

/**
 * Marks code as deprecated. A warning is emitted if the code is used.
 *
 * @param      message              Message to show as part of the warning.
 */
/**@{*/
#if __has_attribute(deprecated) || (defined(__GNUC__) && (__GNUC__ >= 5 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))) ||             \
    defined(__CC_ARM) || defined(__ICCARM__)

#define CHIP_DEPRECATED_MSG(message) __attribute__((deprecated(message)))
#else
#define CHIP_DEPRECATED_MSG(message)
#endif
/**@}*/

/**
 * Marks an enumerator as deprecated. A warning is emitted if it is used.
 *
 * @param      message              Message to show as part of the warning.
 */
/**@{*/
#if __has_extension(enumerator_attributes) || (defined(__GNUC__) && (__GNUC__ >= 6))

#define CHIP_DEPRECATED_ENUM_MSG(message) __attribute__((deprecated(message)))
#else
#define CHIP_DEPRECATED_ENUM_MSG(message)
#endif

/**
 * Emits a warning if calls to the function meet the given criteria.
 *
 * - Warning is only emitted if the criteria can be evaluated at compile-time.
 */
/**@{*/
#if __has_attribute(diagnose_if)
#define CHIP_DIAGNOSE_WARNING(...) __attribute__((diagnose_if(__VA_ARGS__, "warning")))
#else
#define CHIP_DIAGNOSE_WARNING(...)
#endif
/**@}*/

/**
 * Emits an error if calls to the function meet the given criteria.
 *
 * - Error is only emitted if the criteria can be evaluated at compile-time.
 */
/**@{*/
#if __has_attribute(diagnose_if)
#define CHIP_DIAGNOSE_ERROR(...) __attribute__((diagnose_if(__VA_ARGS__, "error")))
#else
#define CHIP_DIAGNOSE_ERROR(...)
#endif
/**@}*/

/**
 * Marks a function to take printf-like arguments. If there is a mismatch between the format string and subsequent
 * function parameter types a warning is emitted.
 *
 * - Argument numbers start at 1.
 *
 * @param      formatArg            Argument number of the format string.
 * @param      firstVarArg          Argument number where variadic arguments start (or 0 if not used).
 */
/**@{*/
#if __has_attribute(format) || (defined(__GNUC__) && (__GNUC__ >= 5 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))) || defined(__CC_ARM)

#define CHIP_PRINTFLIKE(formatArg, firstVarArg) __attribute__((format(printf, formatArg, firstVarArg)))
#else
#define CHIP_PRINTFLIKE(formatArg, firstVarArg)
#endif
/**@}*/

/**
 * Indicates that a pointer argument does not escape.
 *
 * - No reference that is derived from the pointer parameter value will survive after the function returns.
 */
/**@{*/
#if __has_attribute(noescape)
#define CHIP_NOESCAPE __attribute__((noescape))
#else
#define CHIP_NOESCAPE
#endif
/**@}*/

/**
 * Marks a function that never returns once it is called.
 */
/**@{*/
#if defined(_MSC_VER)
#define CHIP_NORETURN __declspec(noreturn)
#elif __has_attribute(noreturn) || (defined(__GNUC__) && (__GNUC__ >= 5 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2))) ||             \
    defined(__CC_ARM) || defined(__ICCARM__)

#define CHIP_NORETURN __attribute__((noreturn))
#else
// This attribute is not optional.
#endif

/**
 * Indicates that a function has no side effects.
 *
 * - The function may read global variables and may dereference pointer arguments.
 */
/**@{*/
#if __has_attribute(pure) || (defined(__GNUC__) && __GNUC__ >= 3) || defined(__CC_ARM) || defined(__ICCARM__)

#define CHIP_NO_SIDE_EFFECTS __attribute__((pure))
#else
#define CHIP_NO_SIDE_EFFECTS
#endif
/**@}*/

/**
 * Marks a declaration as unused.
 */
/**@{*/
#if __has_attribute(unused) || (defined(__GNUC__) && (__GNUC__ >= 5 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2))) ||                 \
    defined(__CC_ARM) || defined(__ICCARM__)

#define CHIP_UNUSED __attribute__((unused))
#else
#define CHIP_UNUSED
#endif
/**@}*/

/**
 * Warns the callers of a function if the result is never used.
 */
/**@{*/
#if __has_attribute(warn_unused_result) || (defined(__GNUC__) && (__GNUC__ >= 5 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2))) ||     \
    defined(__CC_ARM)

#define CHIP_RESULT_USE_CHECK __attribute__((warn_unused_result))
#else
#define CHIP_RESULT_USE_CHECK
#endif
/**@}*/

//----------------------------------------------------------------------------------------------------------------------

/**
 * Poisons an identifier so that it can no longer be used.
 *
 * @param      identifier           Identifier to poison.
 */
/**@{*/
#if !defined(CHIP_DISALLOW_USE_IGNORED) || !CHIP_DISALLOW_USE_IGNORED
#if defined(__clang__) || (defined(__GNUC__) && (__GNUC__ >= 5 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)))
/**@cond */
#define CHIP_DISALLOW_USE_(directive) _Pragma(#directive)
/**@endcond */
#define CHIP_DISALLOW_USE(identifier) CHIP_DISALLOW_USE_(GCC poison identifier)
#else
#define CHIP_DISALLOW_USE(identifier)
#endif
#else
#define CHIP_DISALLOW_USE(identifier)
#endif
/**@}*/

//----------------------------------------------------------------------------------------------------------------------

/**
 * Returns the offset of a member within a structure.
 *
 * @param      type                 Standard-layout data structure or union type.
 * @param      member               Member within the type of which the offset is retrieved.
 *
 * @return Offset of the given member in bytes from the beginning of an object of the specified type.
 */
/**@{*/
#if __has_builtin(__builtin_offsetof) || defined(__GNUC__)

#define CHIP_OFFSETOF(type, member) __builtin_offsetof(type, member)
#else
#define CHIP_OFFSETOF(type, member) offsetof(type, member)
#endif
/**@}*/

//----------------------------------------------------------------------------------------------------------------------

/**
 * @defgroup CompilerDiagnostics Managing compiler diagnostics.
 *
 * These macros may be used to control compiler diagnostics locally.
 *
 * **Example**

   @code{.c}

   CHIP_DIAGNOSTIC_PUSH
   CHIP_DIAGNOSTIC_IGNORED_CLANG("-Wfoo")
   // Code that would trigger -Wfoo when compiling with Clang.
   CHIP_DIAGNOSTIC_POP

   @endcode
 */
/**@{*/

/**
 * Saves the current compiler diagnostic state.
 */
/**@{*/
#if defined(_MSC_VER)
#define CHIP_DIAGNOSTIC_PUSH __pragma(warning(push))
#elif defined(__CC_ARM)
#define CHIP_DIAGNOSTIC_PUSH _Pragma("push")
#elif defined(__clang__) || (defined(__GNUC__) && (__GNUC__ >= 5 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)))
#define CHIP_DIAGNOSTIC_PUSH _Pragma("GCC diagnostic push")
#else
#define CHIP_DIAGNOSTIC_PUSH
#endif
/**@}*/

/**
 * Restores the saved compiler diagnostic state.
 */
/**@{*/
#if defined(_MSC_VER)
#define CHIP_DIAGNOSTIC_POP __pragma(warning(pop))
#elif defined(__CC_ARM)
#define CHIP_DIAGNOSTIC_POP _Pragma("pop")
#elif defined(__clang__) || (defined(__GNUC__) && (__GNUC__ >= 5 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)))
#define CHIP_DIAGNOSTIC_POP _Pragma("GCC diagnostic pop")
#else
#define CHIP_DIAGNOSTIC_POP
#endif
/**@}*/

/**
 * Ignores a Clang compiler warning.
 *
 * @param      warning              Warning to ignore. Must be passed as string, e.g. "-Wfoo".
 */
/**@{*/
#if defined(__clang__)
/**@cond */
#define CHIP_DIAGNOSTIC_IGNORED_CLANG_(directive) _Pragma(#directive)
/**@endcond */
#define CHIP_DIAGNOSTIC_IGNORED_CLANG(warning) CHIP_DIAGNOSTIC_IGNORED_CLANG_(GCC diagnostic ignored warning)
#else
#define CHIP_DIAGNOSTIC_IGNORED_CLANG(warning)
#endif
/**@}*/

/**
 * Ignores a GCC compiler warning.
 *
 * @param      warning              Warning to ignore. Must be passed as string, e.g. "-Wfoo".
 */
/**@{*/
#if (defined(__GNUC__) && (__GNUC__ >= 5 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))) && !defined(__clang)
/**@cond */
#define CHIP_DIAGNOSTIC_IGNORED_GCC_(directive) _Pragma(#directive)
/**@endcond */
#define CHIP_DIAGNOSTIC_IGNORED_GCC(warning) CHIP_DIAGNOSTIC_IGNORED_GCC_(GCC diagnostic ignored warning)
#else
#define CHIP_DIAGNOSTIC_IGNORED_GCC(warning)
#endif
/**@}*/

/**
 * Ignores an ArmCC compiler warning.
 *
 * @param      warning              Warning to ignore. Numeric warning code must be passed, e.g. 42.
 */
/**@{*/
#if defined(__CC_ARM)
/**@cond */
#define CHIP_DIAGNOSTIC_IGNORED_ARMCC_(directive) _Pragma(#directive)
/**@endcond */
#define CHIP_DIAGNOSTIC_IGNORED_ARMCC(warning) CHIP_DIAGNOSTIC_IGNORED_ARMCC_(diag_suppress warning)
#else
#define CHIP_DIAGNOSTIC_IGNORED_ARMCC(warning)
#endif
/**@}*/

/**
 * Ignores an IAR compiler warning.
 *
 * @param      warning              Warning to ignore. Warning code must be passed, e.g. Pm42.
 */
/**@{*/
#if defined(__ICCARM__)
/**@cond */
#define CHIP_DIAGNOSTIC_IGNORED_ICCARM_(directive) _Pragma(#directive)
/**@endcond */
#define CHIP_DIAGNOSTIC_IGNORED_ICCARM(warning) CHIP_DIAGNOSTIC_IGNORED_ICCARM_(diag_suppress = warning)
#else
#define CHIP_DIAGNOSTIC_IGNORED_ICCARM(warning)
#endif
/**@}*/

/**
 * Restores default settings for an IAR compiler warning.
 *
 * @param      warning              Warning to restore to default settings for. Warning code must be passed, e.g. Pm42.
 */
/**@{*/
#if defined(__ICCARM__)
/**@cond */
#define CHIP_DIAGNOSTIC_RESTORE_ICCARM_(directive) _Pragma(#directive)
/**@endcond */
#define CHIP_DIAGNOSTIC_RESTORE_ICCARM(warning) CHIP_DIAGNOSTIC_IGNORED_ICCARM_(diag_default = warning)
#else
#define CHIP_DIAGNOSTIC_RESTORE_ICCARM(warning)
#endif
/**@}*/

/**
 * Ignores an MSVC compiler warning.
 *
 * @param      warning_             Warning to ignore. Numeric warning code must be passed, e.g. 42.
 */
/**@{*/
#if defined(_MSC_VER)
#define CHIP_DIAGNOSTIC_IGNORED_MSVC(warning_) __pragma(warning(disable : warning_))
#else
#define CHIP_DIAGNOSTIC_IGNORED_MSVC(warning_)
#endif
/**@}*/

/**@}*/

//----------------------------------------------------------------------------------------------------------------------

/**
 * Static assertion.
 *
 * @param      condition            Condition to statically check. If it fails, an error will be thrown when compiling.
 * @param      identifier           Identifier to display as part of the compilation error.
 */
#define CHIP_STATIC_ASSERT(condition, identifier) typedef char CHIPStaticAssert_##identifier[(condition) ? 1 : -1]

//----------------------------------------------------------------------------------------------------------------------

/**
 * @defgroup PackedEnum Support for packed enumerations.
 *
 * - Standard C defines enum as int but some compilers provide options to change this behaviour globally.
 *   Compiling the library using different options than the application would result in ABI incompatibility.
 *   Therefore, all enumerations in public interfaces are marked as packed using the CHIP_ENUM macros.
 *   This overrides the globally selected rules and ensures that short enums are used.
 *
 * **Example**

   @code{.c}

   CHIP_ENUM_BEGIN(uint8_t, CHIPFoo) {
       kCHIPFoo_Xxx = 1,
       kCHIPFoo_Yyy
   } CHIP_ENUM_END(uint8_t, CHIPFoo)

   @endcode
 */
/**@{*/

/**
 * Begins definition of a packed enumeration.
 *
 * @param      type                 Smallest integer type that can encode all values of the enumeration.
 * @param      name                 Name of the enumeration.
 */
/**@{*/
#if defined(_MSC_VER)
#define CHIP_ENUM_BEGIN(type, name)                                                                                                \
    typedef type name;                                                                                                             \
    enum
#else
#define CHIP_ENUM_BEGIN(type, name) typedef enum __attribute__((packed))
#endif
/**@}*/

/**
 * Completes definition of a packed enumeration.
 *
 * @param      type                 Smallest integer type that can encode all values of the enumeration.
 * @param      name                 Name of the enumeration.
 */
/**@{*/
#if defined(_MSC_VER)
#define CHIP_ENUM_END(type, name)                                                                                                  \
    ;                                                                                                                              \
    CHIP_STATIC_ASSERT(sizeof(name) == sizeof(type), name)
#else
#define CHIP_ENUM_END(type, name)                                                                                                  \
    name;                                                                                                                          \
    CHIP_STATIC_ASSERT(sizeof(name) == sizeof(type), name)
#endif
/**@}*/

/**@}*/

/**
 * @defgroup OptionSets Support for option sets.
 *
 * **Example**

   @code{.c}

   CHIP_OPTIONS_BEGIN(uint8_t, CHIPFoo) {
       kCHIPFoo_None = 0,
       kCHIPFoo_Xxx = 1 << 0,
       kCHIPFoo_Yyy = 1 << 1
   } CHIP_OPTIONS_END(uint8_t, CHIPFoo)

   @endcode
 */
/**@{*/

/**
 * Begins definition of a packed option set.
 *
 * @param      type                 Smallest integer type that can encode all values of the option set.
 * @param      name                 Name of the option set.
 */
/**@{*/
#if defined(_MSC_VER)
#define CHIP_OPTIONS_BEGIN(type, name)                                                                                             \
    typedef type name;                                                                                                             \
    enum
#else
#define CHIP_OPTIONS_BEGIN(type, name)                                                                                             \
    typedef type name;                                                                                                             \
    enum __attribute__((packed))
#endif
/**@}*/

/**
 * Completes definition of a packed option set.
 *
 * @param      type                 Smallest integer type that can encode all values of the option set.
 * @param      name                 Name of the option set.
 */
/**@{*/
#if defined(_MSC_VER)
#define CHIP_OPTIONS_END(type, name)                                                                                               \
    ;                                                                                                                              \
    CHIP_STATIC_ASSERT(sizeof(name) == sizeof(type), name)
#else
#define CHIP_OPTIONS_END(type, name)                                                                                               \
    ;                                                                                                                              \
    CHIP_STATIC_ASSERT(sizeof(name) == sizeof(type), name)
#endif
/**@}*/

/**@}*/

//----------------------------------------------------------------------------------------------------------------------

/**
 * Defines an opaque type that is 8-byte aligned.
 *
 * @param      numBytes             Size of opaque type.
 *
 * **Example**

   @code{.c}

   // Public header.
   typedef CHIP_OPAQUE(80) CHIPFooRef;

   // Private header.
   typedef struct {
       // ...
   } CHIPFoo;
   CHIP_STATIC_ASSERT(sizeof (CHIPFooRef) >= sizeof (CHIPFoo), CHIPFoo);

   // Private sources.
   CHIPFooRef *foo_;
   CHIPFoo *foo = (CHIPFoo *) foo_;

   @endcode
 */
#if defined(_MSC_VER)
#define CHIP_OPAQUE(numBytes)                                                                                                      \
    struct                                                                                                                         \
    {                                                                                                                              \
        CHIP_ALIGNAS(8) char privateData[numBytes];                                                                                \
    }
#else
#define CHIP_OPAQUE(numBytes)                                                                                                      \
    struct                                                                                                                         \
    {                                                                                                                              \
        char privateData[numBytes] CHIP_ALIGNAS(8);                                                                                \
    }
#endif

//----------------------------------------------------------------------------------------------------------------------

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
