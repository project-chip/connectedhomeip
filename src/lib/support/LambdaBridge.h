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

#include <string.h>
#include <type_traits>

#include <lib/core/CHIPConfig.h>

namespace chip {

class LambdaBridge
{
public:
    // Use initialize instead of constructor because this class has to be trivial
    template <typename Lambda>
    void Initialize(const Lambda & lambda, CHIP_ERROR * Error_Value = nullptr)
    {
        // LambdaBridge accepts either Lambdas without arguments or those with `CHIP_ERROR *` as argument.
        mpError = Error_Value;

        // memcpy is used to move the lambda into the event queue, so it must be trivially copyable
        static_assert(std::is_trivially_copyable<Lambda>::value, "lambda must be trivially copyable");
        static_assert(sizeof(Lambda) <= CHIP_CONFIG_LAMBDA_EVENT_SIZE, "lambda too large");
        static_assert(CHIP_CONFIG_LAMBDA_EVENT_ALIGN % alignof(Lambda) == 0, "lambda align too large");

        // Implicit cast a capture-less lambda into a raw function pointer.
        mLambdaProxy = [](const LambdaStorage & body, CHIP_ERROR * apError) {
            // Check if lambda has CHIP_ERROR * as argument, if not, call it without arguments
            if constexpr (std::is_invocable<Lambda, CHIP_ERROR *>::value)
            {
                // Call the lambda with CHIP_ERROR* argument
                (*reinterpret_cast<const Lambda *>(&body))(apError);
            }
            else
            {
                // Call the lambda with no arguments
                (*reinterpret_cast<const Lambda *>(&body))();
            }
        };

        ::memcpy(&mLambdaBody, &lambda, sizeof(Lambda));
    }

    void operator()() const { mLambdaProxy(mLambdaBody, mpError); }

private:
    using LambdaStorage = std::aligned_storage_t<CHIP_CONFIG_LAMBDA_EVENT_SIZE, CHIP_CONFIG_LAMBDA_EVENT_ALIGN>;
    void (*mLambdaProxy)(const LambdaStorage & body, CHIP_ERROR * mpError);
    LambdaStorage mLambdaBody;
    CHIP_ERROR * mpError;
};

static_assert(std::is_trivial<LambdaBridge>::value, "LambdaBridge is not trivial");

} // namespace chip
