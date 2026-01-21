/*
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <functional>
#include <new>
#include <type_traits>

#include <lib/core/CHIPConfig.h>
#include <lib/support/CodeUtils.h>

namespace chip {

class LambdaBridge
{
public:
    // Use initialize instead of constructor because this class has to be trivial
    template <typename Lambda>
    void Initialize(const Lambda & lambda)
    {
        mLambdaPtr = new (std::nothrow) Lambda(lambda);

        VerifyOrReturn(mLambdaPtr != nullptr);

        mCaller = [](void * context, bool call) {
            auto lambdaPtr = static_cast<Lambda *>(context);

            if (call)
            {
                (*lambdaPtr)();
            }

            delete lambdaPtr;
        };
    }

    void operator()() const
    {
        // Intentionally not verifying mCaller and mLambdaPtr to fail noisily
        (*mCaller)(mLambdaPtr, true);
    }

    bool IsInitialized() const { return mCaller != nullptr && mLambdaPtr != nullptr; }

    void Take(LambdaBridge && from)
    {
        mLambdaPtr = from.mLambdaPtr;
        mCaller    = from.mCaller;

        from.mLambdaPtr = nullptr;
        from.mCaller    = nullptr;
    }

    void Destroy()
    {
        VerifyOrReturn(IsInitialized());

        mCaller(mLambdaPtr, false);

        mLambdaPtr = nullptr;
        mCaller    = nullptr;
    }

private:
    void * mLambdaPtr;
    void (*mCaller)(void * context, bool call);
};

static_assert(std::is_trivial<LambdaBridge>::value, "LambdaBridge is not trivial");
} // namespace chip
