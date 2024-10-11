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

namespace chip {

template <typename T>
class ScopedChange;

/// Allows a value to only be changed within a scope.
///
/// Generally used to force determinism for unit test execution.
///
/// When a variable of this type is used, it should
/// only be changed via `ScopedChange`.
template <typename T>
class ScopedChangeOnly
{
public:
    explicit ScopedChangeOnly(T initial) : mValue(initial) {}
    operator T() const { return mValue; }

private:
    T mValue;

    // Expected to be used only by ScopedChange<T> only
    T & InternalMutableValue() { return mValue; }
    friend class ScopedChange<T>;
};

/// Allows a scoped mutation to occur on a variable.
///
/// When an instance of this class goes out of scope, the variable
/// will be reset to the value id had before the scoped change came
/// into effect.
///
/// While the ScopedChange is in scope it can be used to make changes
/// to the underlying value using the assignment operator.
///
/// Example usage:
///
/// int a = 123;
/// ScopedChangeOnly<int> b(234);
///
/// /* a == 123, b == 234 */
/// {
///    ScopedChange changeA(a, 321);
///    /* a == 321, b == 234 */
///    {
///       ScopedChange changeB(b, 10);
///       /* a == 321, b == 10 */
///    }
///    /* a == 321, b == 234 */
///    changeA = 333; // assignments within the ScopedChange are allowed
///    /* a == 333, b == 234 */
/// }
/// /* a == 123, b == 234 */
///
template <typename T>
class ScopedChange
{
public:
    ScopedChange(ScopedChangeOnly<T> & what) : mValue(what.InternalMutableValue()), mOriginal(what) {}
    ScopedChange(ScopedChangeOnly<T> & what, T value) : ScopedChange(what) { mValue = value; }

    ScopedChange(T & what) : mValue(what), mOriginal(what) {}
    ScopedChange(T & what, T value) : ScopedChange(what) { mValue = value; }

    ~ScopedChange() { mValue = mOriginal; }

    ScopedChange & operator=(T const & value)
    {
        mValue = value;
        return *this;
    }

    operator T() const { return mValue; }

private:
    T & mValue;
    T mOriginal;
};

} // namespace chip
