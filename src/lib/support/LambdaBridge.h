/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
    void Initialize(const Lambda & lambda)
    {
        // memcpy is used to move the lambda into the event queue, so it must be trivially copyable
        static_assert(std::is_trivially_copyable<Lambda>::value, "lambda must be trivially copyable");
        static_assert(sizeof(Lambda) <= CHIP_CONFIG_LAMBDA_EVENT_SIZE, "lambda too large");
        static_assert(CHIP_CONFIG_LAMBDA_EVENT_ALIGN % alignof(Lambda) == 0, "lambda align too large");

        // Implicit cast a capture-less lambda into a raw function pointer.
        mLambdaProxy = [](const LambdaStorage & body) { (*reinterpret_cast<const Lambda *>(&body))(); };
        ::memcpy(&mLambdaBody, &lambda, sizeof(Lambda));
    }

    void operator()() const { mLambdaProxy(mLambdaBody); }

private:
    using LambdaStorage = std::aligned_storage_t<CHIP_CONFIG_LAMBDA_EVENT_SIZE, CHIP_CONFIG_LAMBDA_EVENT_ALIGN>;
    void (*mLambdaProxy)(const LambdaStorage & body);
    LambdaStorage mLambdaBody;
};

static_assert(std::is_trivial<LambdaBridge>::value, "LambdaBridge is not trivial");

} // namespace chip
