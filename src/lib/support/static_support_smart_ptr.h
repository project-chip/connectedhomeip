/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
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
#pragma once

#include <lib/support/CodeUtils.h>

namespace chip {

/// A template that is able to provide the global instance
/// for some application-specific class
///
/// It works specifically together with CheckedGlobalInstanceReference
template <class T>
struct GlobalInstanceProvider
{
    static T * InstancePointer();
};

/// A class that looks like a smart pointer (overrides operator->)
///
/// However internally it only checks that the provided value is a given
/// Global instance.
///
/// The global instance should be provided via GlobalInstanceProvider, for
/// example
///
/// namespace chip {
///   template<>
///   Foo *GlobalInstanceProvider<Foo>::InstancePointer() {
///       return Foo::Instance();
///   }
/// } // namespace chip
///
/// The CheckedGlobalInstanceReference will have minimal size (1 byte because
/// comparing instance pointers has to work) and does not require alignment,
/// as opposed to sizeof(void*) usage for SimpleInstanceReferences
///
template <class T>
class CheckedGlobalInstanceReference
{
public:
    CheckedGlobalInstanceReference() = default;
    CheckedGlobalInstanceReference(T * e) { VerifyOrDie(e == GlobalInstanceProvider<T>::InstancePointer()); }
    CheckedGlobalInstanceReference & operator=(T * value)
    {
        VerifyOrDie(value == GlobalInstanceProvider<T>::InstancePointer());
        return *this;
    }

    inline T * operator->() { return GlobalInstanceProvider<T>::InstancePointer(); }
    inline const T * operator->() const { return GlobalInstanceProvider<T>::InstancePointer(); }
    inline operator bool() const { return true; }
};

/// A class that acts as a wrapper to a pointer and provides
/// operator-> overrides.
///
/// It provides the same interface as CheckedGlobalInstanceReference
/// however it does NOT use a global value.
///
/// The intended usage of these pair of classes is to compile-time decide
/// if global variables are to be used or if fully dynamic pointers are
/// allowed.
///
/// Example:
///   #if USE_GLOBALS
///     template<class T>
///     using PointerContainer = chip::CheckedGlobalInstanceReference<T>;
///   #else
///     template<class T>
///     using PointerContainer = chip::SimpleInstanceReference<T>;
///   #endif
template <class T>
class SimpleInstanceReference
{
public:
    SimpleInstanceReference() = default;
    SimpleInstanceReference(T * e) : mValue(e) {}
    SimpleInstanceReference & operator=(T * value)
    {
        mValue = value;
        return *this;
    }

    T * operator->() { return mValue; }
    const T * operator->() const { return mValue; }
    inline operator bool() const { return mValue != nullptr; }

private:
    T * mValue = nullptr;
};

} // namespace chip
