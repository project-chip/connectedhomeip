/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/util/CHIPDeviceCallbacksMgr.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

namespace chip {
namespace app {
namespace {

using SuccessCallback = void (*)(void * context, uint64_t value);
using FailureCallback = void (*)(void * context, CHIP_ERROR error);

void ShouldGetSingleResponseCallback(nlTestSuite * testSuite, void * apContext)
{
    auto & callbacks = CHIPDeviceCallbacksMgr::GetInstance();

    static constexpr NodeId kTestNodeId     = 0x9b3780f93739918d;
    static constexpr uint8_t kTestSequence  = 0x8b;
    static constexpr uint64_t kTestArgument = 0x973f2e7071e0c12a;
    static constexpr CHIP_ERROR kTestError  = CHIP_ERROR_INVALID_ARGUMENT;

    struct CallbackContext
    {
        nlTestSuite * const testSuite;
    } callbackContext = { testSuite };

    const auto onSuccess = [](void * opaqueContext, uint64_t value) {
        CallbackContext & context = *static_cast<CallbackContext *>(opaqueContext);

        NL_TEST_ASSERT(context.testSuite, value == kTestArgument);
    };

    const auto onFailure = [](void * opaqueContext, CHIP_ERROR error) {
        CallbackContext & context = *static_cast<CallbackContext *>(opaqueContext);

        NL_TEST_ASSERT(context.testSuite, error == kTestError);
    };

    Callback::Callback<SuccessCallback> successCallback{ onSuccess, &callbackContext };
    Callback::Callback<FailureCallback> failureCallback{ onFailure, &callbackContext };

    {
        Callback::Cancelable * successCancelable = nullptr;
        Callback::Cancelable * failureCancelable = nullptr;
        CHIP_ERROR error = callbacks.GetResponseCallback(kTestNodeId, kTestSequence, &successCancelable, &failureCancelable);
        NL_TEST_ASSERT(testSuite, error == CHIP_ERROR_KEY_NOT_FOUND);
    }

    {
        CHIP_ERROR error =
            callbacks.AddResponseCallback(kTestNodeId, kTestSequence, successCallback.Cancel(), failureCallback.Cancel());
        NL_TEST_ASSERT(testSuite, error == CHIP_NO_ERROR);
    }

    {
        Callback::Cancelable * successCancelable = nullptr;
        Callback::Cancelable * failureCancelable = nullptr;
        CHIP_ERROR error = callbacks.GetResponseCallback(kTestNodeId, kTestSequence, &successCancelable, &failureCancelable);
        NL_TEST_ASSERT(testSuite, error == CHIP_NO_ERROR);

        auto outSuccessCallback = decltype(successCallback)::FromCancelable(successCancelable);
        auto outFailureCallback = decltype(failureCallback)::FromCancelable(failureCancelable);

        NL_TEST_ASSERT(testSuite, outSuccessCallback == &successCallback);
        NL_TEST_ASSERT(testSuite, outFailureCallback == &failureCallback);

        outSuccessCallback->mCall(outSuccessCallback->mContext, kTestArgument);
        outFailureCallback->mCall(outFailureCallback->mContext, kTestError);
    }

    {
        Callback::Cancelable * successCancelable = nullptr;
        Callback::Cancelable * failureCancelable = nullptr;
        CHIP_ERROR error = callbacks.GetResponseCallback(kTestNodeId, kTestSequence, &successCancelable, &failureCancelable);
        NL_TEST_ASSERT(testSuite, error == CHIP_ERROR_KEY_NOT_FOUND);
    }
}

void ShouldGetMultipleResponseCallbacks(nlTestSuite * testSuite, void * apContext)
{
    auto & callbacks = CHIPDeviceCallbacksMgr::GetInstance();

    struct CallbackContext
    {
        nlTestSuite * const testSuite;
    } callbackContext = { testSuite };

    const auto onSuccess = [](void * opaqueContext, uint64_t value) {
        CallbackContext & context = *static_cast<CallbackContext *>(opaqueContext);

        // Not called.
        NL_TEST_ASSERT(context.testSuite, false);
    };

    const auto onFailure = [](void * opaqueContext, CHIP_ERROR error) {
        CallbackContext & context = *static_cast<CallbackContext *>(opaqueContext);

        // Not called.
        NL_TEST_ASSERT(context.testSuite, false);
    };

    Callback::Callback<SuccessCallback> successCallback{ onSuccess, &callbackContext };
    Callback::Callback<FailureCallback> failureCallback{ onFailure, &callbackContext };

    Callback::Callback<SuccessCallback> successCallback2{ onSuccess, &callbackContext };
    Callback::Callback<FailureCallback> failureCallback2{ onFailure, &callbackContext };

    Callback::Callback<SuccessCallback> successCallback3{ onSuccess, &callbackContext };
    Callback::Callback<FailureCallback> failureCallback3{ onFailure, &callbackContext };

    static constexpr NodeId kTestNodeId1    = 0x2f699f085d799846;
    static constexpr NodeId kTestNodeId2    = 0xc354d359ee0b47f3;
    static constexpr uint8_t kTestSequence1 = 0x8b;
    static constexpr uint8_t kTestSequence2 = kTestSequence1 + 1;

    {
        CHIP_ERROR error =
            callbacks.AddResponseCallback(kTestNodeId1, kTestSequence1, successCallback.Cancel(), failureCallback.Cancel());
        NL_TEST_ASSERT(testSuite, error == CHIP_NO_ERROR);
    }

    {
        Callback::Cancelable * successCancelable = nullptr;
        Callback::Cancelable * failureCancelable = nullptr;
        CHIP_ERROR error = callbacks.GetResponseCallback(kTestNodeId2, kTestSequence1, &successCancelable, &failureCancelable);
        NL_TEST_ASSERT(testSuite, error == CHIP_ERROR_KEY_NOT_FOUND);
    }

    {
        Callback::Cancelable * successCancelable = nullptr;
        Callback::Cancelable * failureCancelable = nullptr;
        CHIP_ERROR error = callbacks.GetResponseCallback(kTestNodeId1, kTestSequence2, &successCancelable, &failureCancelable);
        NL_TEST_ASSERT(testSuite, error == CHIP_ERROR_KEY_NOT_FOUND);
    }

    {
        CHIP_ERROR error =
            callbacks.AddResponseCallback(kTestNodeId1, kTestSequence1, successCallback.Cancel(), failureCallback.Cancel());
        NL_TEST_ASSERT(testSuite, error == CHIP_NO_ERROR);
    }

    {
        CHIP_ERROR error =
            callbacks.AddResponseCallback(kTestNodeId2, kTestSequence1, successCallback2.Cancel(), failureCallback2.Cancel());
        NL_TEST_ASSERT(testSuite, error == CHIP_NO_ERROR);
    }

    {
        CHIP_ERROR error =
            callbacks.AddResponseCallback(kTestNodeId1, kTestSequence2, successCallback3.Cancel(), failureCallback3.Cancel());
        NL_TEST_ASSERT(testSuite, error == CHIP_NO_ERROR);
    }

    {
        Callback::Cancelable * successCancelable = nullptr;
        Callback::Cancelable * failureCancelable = nullptr;
        CHIP_ERROR error = callbacks.GetResponseCallback(kTestNodeId2, kTestSequence1, &successCancelable, &failureCancelable);
        NL_TEST_ASSERT(testSuite, error == CHIP_NO_ERROR);

        auto outSuccessCallback = decltype(successCallback2)::FromCancelable(successCancelable);
        auto outFailureCallback = decltype(failureCallback2)::FromCancelable(failureCancelable);

        NL_TEST_ASSERT(testSuite, outSuccessCallback == &successCallback2);
        NL_TEST_ASSERT(testSuite, outFailureCallback == &failureCallback2);
    }

    {
        Callback::Cancelable * successCancelable = nullptr;
        Callback::Cancelable * failureCancelable = nullptr;
        CHIP_ERROR error = callbacks.GetResponseCallback(kTestNodeId1, kTestSequence2, &successCancelable, &failureCancelable);
        NL_TEST_ASSERT(testSuite, error == CHIP_NO_ERROR);

        auto outSuccessCallback = decltype(successCallback2)::FromCancelable(successCancelable);
        auto outFailureCallback = decltype(failureCallback2)::FromCancelable(failureCancelable);

        NL_TEST_ASSERT(testSuite, outSuccessCallback == &successCallback3);
        NL_TEST_ASSERT(testSuite, outFailureCallback == &failureCallback3);
    }

    {
        Callback::Cancelable * successCancelable = nullptr;
        Callback::Cancelable * failureCancelable = nullptr;
        CHIP_ERROR error = callbacks.GetResponseCallback(kTestNodeId1, kTestSequence1, &successCancelable, &failureCancelable);
        NL_TEST_ASSERT(testSuite, error == CHIP_NO_ERROR);

        auto outSuccessCallback = decltype(successCallback)::FromCancelable(successCancelable);
        auto outFailureCallback = decltype(failureCallback)::FromCancelable(failureCancelable);

        NL_TEST_ASSERT(testSuite, outSuccessCallback == &successCallback);
        NL_TEST_ASSERT(testSuite, outFailureCallback == &failureCallback);
    }

    {
        Callback::Cancelable * successCancelable = nullptr;
        Callback::Cancelable * failureCancelable = nullptr;
        CHIP_ERROR error = callbacks.GetResponseCallback(kTestNodeId1, kTestSequence1, &successCancelable, &failureCancelable);
        NL_TEST_ASSERT(testSuite, error == CHIP_ERROR_KEY_NOT_FOUND);
    }

    {
        Callback::Cancelable * successCancelable = nullptr;
        Callback::Cancelable * failureCancelable = nullptr;
        CHIP_ERROR error = callbacks.GetResponseCallback(kTestNodeId1, kTestSequence2, &successCancelable, &failureCancelable);
        NL_TEST_ASSERT(testSuite, error == CHIP_ERROR_KEY_NOT_FOUND);
    }

    {
        Callback::Cancelable * successCancelable = nullptr;
        Callback::Cancelable * failureCancelable = nullptr;
        CHIP_ERROR error = callbacks.GetResponseCallback(kTestNodeId2, kTestSequence1, &successCancelable, &failureCancelable);
        NL_TEST_ASSERT(testSuite, error == CHIP_ERROR_KEY_NOT_FOUND);
    }
}

void ShouldFailGetCanceledResponseCallback(nlTestSuite * testSuite, void * apContext)
{
    auto & callbacks = CHIPDeviceCallbacksMgr::GetInstance();

    static constexpr NodeId kTestNodeId    = 0x9b3780f93739918d;
    static constexpr uint8_t kTestSequence = 0x8b;

    struct CallbackContext
    {
        nlTestSuite * const testSuite;
    } callbackContext = { testSuite };

    const auto onSuccess = [](void * opaqueContext, uint64_t value) {
        CallbackContext & context = *static_cast<CallbackContext *>(opaqueContext);

        // Not called.
        NL_TEST_ASSERT(context.testSuite, false);
    };

    const auto onFailure = [](void * opaqueContext, CHIP_ERROR error) {
        CallbackContext & context = *static_cast<CallbackContext *>(opaqueContext);

        // Not called.
        NL_TEST_ASSERT(context.testSuite, false);
    };

    Callback::Callback<SuccessCallback> successCallback{ onSuccess, &callbackContext };
    Callback::Callback<FailureCallback> failureCallback{ onFailure, &callbackContext };

    {
        Callback::Cancelable * successCancelable = nullptr;
        Callback::Cancelable * failureCancelable = nullptr;
        CHIP_ERROR error = callbacks.GetResponseCallback(kTestNodeId, kTestSequence, &successCancelable, &failureCancelable);
        NL_TEST_ASSERT(testSuite, error == CHIP_ERROR_KEY_NOT_FOUND);
    }

    {
        CHIP_ERROR error =
            callbacks.AddResponseCallback(kTestNodeId, kTestSequence, successCallback.Cancel(), failureCallback.Cancel());
        NL_TEST_ASSERT(testSuite, error == CHIP_NO_ERROR);
    }

    {
        Callback::Cancelable * successCancelable = nullptr;
        Callback::Cancelable * failureCancelable = nullptr;
        CHIP_ERROR error = callbacks.GetResponseCallback(kTestNodeId, kTestSequence, &successCancelable, &failureCancelable);
        NL_TEST_ASSERT(testSuite, error == CHIP_NO_ERROR);

        auto outSuccessCallback = decltype(successCallback)::FromCancelable(successCancelable);
        auto outFailureCallback = decltype(failureCallback)::FromCancelable(failureCancelable);

        NL_TEST_ASSERT(testSuite, outSuccessCallback == &successCallback);
        NL_TEST_ASSERT(testSuite, outFailureCallback == &failureCallback);
    }

    {
        Callback::Cancelable * successCancelable = nullptr;
        Callback::Cancelable * failureCancelable = nullptr;
        CHIP_ERROR error = callbacks.GetResponseCallback(kTestNodeId, kTestSequence, &successCancelable, &failureCancelable);
        NL_TEST_ASSERT(testSuite, error == CHIP_ERROR_KEY_NOT_FOUND);
    }
}

void ShouldGetSingleReportCallback(nlTestSuite * testSuite, void * apContext)
{
    auto & callbacks = CHIPDeviceCallbacksMgr::GetInstance();

    static constexpr NodeId kTestNodeId           = 0x9b3780f93739918d;
    static constexpr EndpointId kTestEndpointId   = 0x20;
    static constexpr ClusterId kTestClusterId     = 0x9103;
    static constexpr AttributeId kTestAttributeId = 0x2232;

    struct CallbackContext
    {
        nlTestSuite * const testSuite;
    } callbackContext = { testSuite };

    const auto onReport = [](void * opaqueContext, uint64_t value) {
        CallbackContext & context = *static_cast<CallbackContext *>(opaqueContext);

        // Not called.
        NL_TEST_ASSERT(context.testSuite, false);
    };

    Callback::Callback<SuccessCallback> reportCallback{ onReport, &callbackContext };

    {
        Callback::Cancelable * reportCancelable = nullptr;
        CHIP_ERROR error =
            callbacks.GetReportCallback(kTestNodeId, kTestEndpointId, kTestClusterId, kTestAttributeId, &reportCancelable);
        NL_TEST_ASSERT(testSuite, error == CHIP_ERROR_KEY_NOT_FOUND);
        NL_TEST_ASSERT(testSuite, reportCancelable == nullptr);
    }

    {
        CHIP_ERROR error =
            callbacks.AddReportCallback(kTestNodeId, kTestEndpointId, kTestClusterId, kTestAttributeId, reportCallback.Cancel());
        NL_TEST_ASSERT(testSuite, error == CHIP_NO_ERROR);
    }

    {
        Callback::Cancelable * reportCancelable = nullptr;
        CHIP_ERROR error =
            callbacks.GetReportCallback(kTestNodeId, kTestEndpointId, kTestClusterId, kTestAttributeId, &reportCancelable);
        NL_TEST_ASSERT(testSuite, error == CHIP_NO_ERROR);

        auto outReportCallback = decltype(reportCallback)::FromCancelable(reportCancelable);

        NL_TEST_ASSERT(testSuite, outReportCallback == &reportCallback);
    }

    {
        Callback::Cancelable * reportCancelable = nullptr;
        CHIP_ERROR error =
            callbacks.GetReportCallback(kTestNodeId, kTestEndpointId, kTestClusterId, kTestAttributeId, &reportCancelable);
        NL_TEST_ASSERT(testSuite, error == CHIP_NO_ERROR);

        auto outReportCallback = decltype(reportCallback)::FromCancelable(reportCancelable);

        NL_TEST_ASSERT(testSuite, outReportCallback == &reportCallback);
    }
}

void ShouldFailGetCanceledReportCallback(nlTestSuite * testSuite, void * apContext)
{
    auto & callbacks = CHIPDeviceCallbacksMgr::GetInstance();

    static constexpr NodeId kTestNodeId           = 0x9b3780f93739918d;
    static constexpr EndpointId kTestEndpointId   = 0x20;
    static constexpr ClusterId kTestClusterId     = 0x9103;
    static constexpr AttributeId kTestAttributeId = 0x2232;

    struct CallbackContext
    {
        nlTestSuite * const testSuite;
    } callbackContext = { testSuite };

    const auto onReport = [](void * opaqueContext, uint64_t value) {
        CallbackContext & context = *static_cast<CallbackContext *>(opaqueContext);

        // Not called.
        NL_TEST_ASSERT(context.testSuite, false);
    };

    Callback::Callback<SuccessCallback> reportCallback{ onReport, &callbackContext };

    {
        Callback::Cancelable * reportCancelable = nullptr;
        CHIP_ERROR error =
            callbacks.GetReportCallback(kTestNodeId, kTestEndpointId, kTestClusterId, kTestAttributeId, &reportCancelable);
        NL_TEST_ASSERT(testSuite, error == CHIP_ERROR_KEY_NOT_FOUND);
        NL_TEST_ASSERT(testSuite, reportCancelable == nullptr);
    }

    {
        CHIP_ERROR error =
            callbacks.AddReportCallback(kTestNodeId, kTestEndpointId, kTestClusterId, kTestAttributeId, reportCallback.Cancel());
        NL_TEST_ASSERT(testSuite, error == CHIP_NO_ERROR);
    }

    {
        Callback::Cancelable * reportCancelable = nullptr;
        CHIP_ERROR error =
            callbacks.GetReportCallback(kTestNodeId, kTestEndpointId, kTestClusterId, kTestAttributeId, &reportCancelable);
        NL_TEST_ASSERT(testSuite, error == CHIP_NO_ERROR);

        auto outReportCallback = decltype(reportCallback)::FromCancelable(reportCancelable);

        NL_TEST_ASSERT(testSuite, outReportCallback == &reportCallback);
    }

    reportCallback.Cancel();

    {
        Callback::Cancelable * reportCancelable = nullptr;
        CHIP_ERROR error =
            callbacks.GetReportCallback(kTestNodeId, kTestEndpointId, kTestClusterId, kTestAttributeId, &reportCancelable);
        NL_TEST_ASSERT(testSuite, error == CHIP_ERROR_KEY_NOT_FOUND);
        NL_TEST_ASSERT(testSuite, reportCancelable == nullptr);
    }
}

} // namespace
} // namespace app
} // namespace chip

const nlTest sTests[] = {
    NL_TEST_DEF("ShouldGetSingleResponseCallback", chip::app::ShouldGetSingleResponseCallback),             //
    NL_TEST_DEF("ShouldGetMultipleResponseCallbacks", chip::app::ShouldGetMultipleResponseCallbacks),       //
    NL_TEST_DEF("ShouldFailGetCanceledResponseCallback", chip::app::ShouldFailGetCanceledResponseCallback), //
    NL_TEST_DEF("ShouldGetSingleReportCallback", chip::app::ShouldGetSingleReportCallback),                 //
    NL_TEST_DEF("ShouldFailGetCanceledReportCallback", chip::app::ShouldFailGetCanceledReportCallback),     //
    NL_TEST_SENTINEL(),                                                                                     //
};

int TestCHIPDeviceCallbacksMgr()
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "TestCHIPDeviceCallbacksMgr",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestCHIPDeviceCallbacksMgr)
