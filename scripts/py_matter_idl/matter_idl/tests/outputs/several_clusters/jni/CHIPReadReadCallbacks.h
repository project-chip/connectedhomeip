/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <jni/CHIPCallbackTypes.h>

#include <controller/java/AndroidCallbacks.h>
#include <jni.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/JniReferences.h>
#include <zap-generated/CHIPClientCallbacks.h>

class CHIPBooleanAttributeCallback : public chip::Callback::Callback<BooleanAttributeCallback>
{
public:
    CHIPBooleanAttributeCallback(jobject javaCallback, bool keepAlive = false);

    ~CHIPBooleanAttributeCallback();

    static void maybeDestroy(CHIPBooleanAttributeCallback * callback) {
        if (!callback->keepAlive) {
            callback->Cancel();
            chip::Platform::Delete<CHIPBooleanAttributeCallback>(callback);
        }
    }

    static void CallbackFn(void * context, bool value);
    static void OnSubscriptionEstablished(void * context, chip::SubscriptionId subscriptionId) {
        CHIP_ERROR err = chip::JniReferences::GetInstance().CallSubscriptionEstablished(reinterpret_cast<CHIPBooleanAttributeCallback *>(context)->javaCallbackRef, subscriptionId);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Error calling onSubscriptionEstablished: %s", ErrorStr(err)));
    };

private:
    jobject javaCallbackRef;
    bool keepAlive;
};
class CHIPCharStringAttributeCallback : public chip::Callback::Callback<CharStringAttributeCallback>
{
public:
    CHIPCharStringAttributeCallback(jobject javaCallback, bool keepAlive = false);

    ~CHIPCharStringAttributeCallback();

    static void maybeDestroy(CHIPCharStringAttributeCallback * callback) {
        if (!callback->keepAlive) {
            callback->Cancel();
            chip::Platform::Delete<CHIPCharStringAttributeCallback>(callback);
        }
    }

    static void CallbackFn(void * context, const chip::CharSpan value);
    static void OnSubscriptionEstablished(void * context, chip::SubscriptionId subscriptionId) {
        CHIP_ERROR err = chip::JniReferences::GetInstance().CallSubscriptionEstablished(reinterpret_cast<CHIPCharStringAttributeCallback *>(context)->javaCallbackRef, subscriptionId);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Error calling onSubscriptionEstablished: %s", ErrorStr(err)));
    };

private:
    jobject javaCallbackRef;
    bool keepAlive;
};
class CHIPDoubleAttributeCallback : public chip::Callback::Callback<DoubleAttributeCallback>
{
public:
    CHIPDoubleAttributeCallback(jobject javaCallback, bool keepAlive = false);

    ~CHIPDoubleAttributeCallback();

    static void maybeDestroy(CHIPDoubleAttributeCallback * callback) {
        if (!callback->keepAlive) {
            callback->Cancel();
            chip::Platform::Delete<CHIPDoubleAttributeCallback>(callback);
        }
    }

    static void CallbackFn(void * context, double value);
    static void OnSubscriptionEstablished(void * context, chip::SubscriptionId subscriptionId) {
        CHIP_ERROR err = chip::JniReferences::GetInstance().CallSubscriptionEstablished(reinterpret_cast<CHIPDoubleAttributeCallback *>(context)->javaCallbackRef, subscriptionId);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Error calling onSubscriptionEstablished: %s", ErrorStr(err)));
    };

private:
    jobject javaCallbackRef;
    bool keepAlive;
};
class CHIPFloatAttributeCallback : public chip::Callback::Callback<FloatAttributeCallback>
{
public:
    CHIPFloatAttributeCallback(jobject javaCallback, bool keepAlive = false);

    ~CHIPFloatAttributeCallback();

    static void maybeDestroy(CHIPFloatAttributeCallback * callback) {
        if (!callback->keepAlive) {
            callback->Cancel();
            chip::Platform::Delete<CHIPFloatAttributeCallback>(callback);
        }
    }

    static void CallbackFn(void * context, float value);
    static void OnSubscriptionEstablished(void * context, chip::SubscriptionId subscriptionId) {
        CHIP_ERROR err = chip::JniReferences::GetInstance().CallSubscriptionEstablished(reinterpret_cast<CHIPFloatAttributeCallback *>(context)->javaCallbackRef, subscriptionId);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Error calling onSubscriptionEstablished: %s", ErrorStr(err)));
    };

private:
    jobject javaCallbackRef;
    bool keepAlive;
};
class CHIPInt8sAttributeCallback : public chip::Callback::Callback<Int8sAttributeCallback>
{
public:
    CHIPInt8sAttributeCallback(jobject javaCallback, bool keepAlive = false);

    ~CHIPInt8sAttributeCallback();

    static void maybeDestroy(CHIPInt8sAttributeCallback * callback) {
        if (!callback->keepAlive) {
            callback->Cancel();
            chip::Platform::Delete<CHIPInt8sAttributeCallback>(callback);
        }
    }

    static void CallbackFn(void * context, int8_t value);
    static void OnSubscriptionEstablished(void * context, chip::SubscriptionId subscriptionId) {
        CHIP_ERROR err = chip::JniReferences::GetInstance().CallSubscriptionEstablished(reinterpret_cast<CHIPInt8sAttributeCallback *>(context)->javaCallbackRef, subscriptionId);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Error calling onSubscriptionEstablished: %s", ErrorStr(err)));
    };

private:
    jobject javaCallbackRef;
    bool keepAlive;
};
class CHIPInt8uAttributeCallback : public chip::Callback::Callback<Int8uAttributeCallback>
{
public:
    CHIPInt8uAttributeCallback(jobject javaCallback, bool keepAlive = false);

    ~CHIPInt8uAttributeCallback();

    static void maybeDestroy(CHIPInt8uAttributeCallback * callback) {
        if (!callback->keepAlive) {
            callback->Cancel();
            chip::Platform::Delete<CHIPInt8uAttributeCallback>(callback);
        }
    }

    static void CallbackFn(void * context, uint8_t value);
    static void OnSubscriptionEstablished(void * context, chip::SubscriptionId subscriptionId) {
        CHIP_ERROR err = chip::JniReferences::GetInstance().CallSubscriptionEstablished(reinterpret_cast<CHIPInt8uAttributeCallback *>(context)->javaCallbackRef, subscriptionId);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Error calling onSubscriptionEstablished: %s", ErrorStr(err)));
    };

private:
    jobject javaCallbackRef;
    bool keepAlive;
};
class CHIPInt16sAttributeCallback : public chip::Callback::Callback<Int16sAttributeCallback>
{
public:
    CHIPInt16sAttributeCallback(jobject javaCallback, bool keepAlive = false);

    ~CHIPInt16sAttributeCallback();

    static void maybeDestroy(CHIPInt16sAttributeCallback * callback) {
        if (!callback->keepAlive) {
            callback->Cancel();
            chip::Platform::Delete<CHIPInt16sAttributeCallback>(callback);
        }
    }

    static void CallbackFn(void * context, int16_t value);
    static void OnSubscriptionEstablished(void * context, chip::SubscriptionId subscriptionId) {
        CHIP_ERROR err = chip::JniReferences::GetInstance().CallSubscriptionEstablished(reinterpret_cast<CHIPInt16sAttributeCallback *>(context)->javaCallbackRef, subscriptionId);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Error calling onSubscriptionEstablished: %s", ErrorStr(err)));
    };

private:
    jobject javaCallbackRef;
    bool keepAlive;
};
class CHIPInt16uAttributeCallback : public chip::Callback::Callback<Int16uAttributeCallback>
{
public:
    CHIPInt16uAttributeCallback(jobject javaCallback, bool keepAlive = false);

    ~CHIPInt16uAttributeCallback();

    static void maybeDestroy(CHIPInt16uAttributeCallback * callback) {
        if (!callback->keepAlive) {
            callback->Cancel();
            chip::Platform::Delete<CHIPInt16uAttributeCallback>(callback);
        }
    }

    static void CallbackFn(void * context, uint16_t value);
    static void OnSubscriptionEstablished(void * context, chip::SubscriptionId subscriptionId) {
        CHIP_ERROR err = chip::JniReferences::GetInstance().CallSubscriptionEstablished(reinterpret_cast<CHIPInt16uAttributeCallback *>(context)->javaCallbackRef, subscriptionId);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Error calling onSubscriptionEstablished: %s", ErrorStr(err)));
    };

private:
    jobject javaCallbackRef;
    bool keepAlive;
};
class CHIPInt32sAttributeCallback : public chip::Callback::Callback<Int32sAttributeCallback>
{
public:
    CHIPInt32sAttributeCallback(jobject javaCallback, bool keepAlive = false);

    ~CHIPInt32sAttributeCallback();

    static void maybeDestroy(CHIPInt32sAttributeCallback * callback) {
        if (!callback->keepAlive) {
            callback->Cancel();
            chip::Platform::Delete<CHIPInt32sAttributeCallback>(callback);
        }
    }

    static void CallbackFn(void * context, int32_t value);
    static void OnSubscriptionEstablished(void * context, chip::SubscriptionId subscriptionId) {
        CHIP_ERROR err = chip::JniReferences::GetInstance().CallSubscriptionEstablished(reinterpret_cast<CHIPInt32sAttributeCallback *>(context)->javaCallbackRef, subscriptionId);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Error calling onSubscriptionEstablished: %s", ErrorStr(err)));
    };

private:
    jobject javaCallbackRef;
    bool keepAlive;
};
class CHIPInt32uAttributeCallback : public chip::Callback::Callback<Int32uAttributeCallback>
{
public:
    CHIPInt32uAttributeCallback(jobject javaCallback, bool keepAlive = false);

    ~CHIPInt32uAttributeCallback();

    static void maybeDestroy(CHIPInt32uAttributeCallback * callback) {
        if (!callback->keepAlive) {
            callback->Cancel();
            chip::Platform::Delete<CHIPInt32uAttributeCallback>(callback);
        }
    }

    static void CallbackFn(void * context, uint32_t value);
    static void OnSubscriptionEstablished(void * context, chip::SubscriptionId subscriptionId) {
        CHIP_ERROR err = chip::JniReferences::GetInstance().CallSubscriptionEstablished(reinterpret_cast<CHIPInt32uAttributeCallback *>(context)->javaCallbackRef, subscriptionId);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Error calling onSubscriptionEstablished: %s", ErrorStr(err)));
    };

private:
    jobject javaCallbackRef;
    bool keepAlive;
};
class CHIPInt64sAttributeCallback : public chip::Callback::Callback<Int64sAttributeCallback>
{
public:
    CHIPInt64sAttributeCallback(jobject javaCallback, bool keepAlive = false);

    ~CHIPInt64sAttributeCallback();

    static void maybeDestroy(CHIPInt64sAttributeCallback * callback) {
        if (!callback->keepAlive) {
            callback->Cancel();
            chip::Platform::Delete<CHIPInt64sAttributeCallback>(callback);
        }
    }

    static void CallbackFn(void * context, int64_t value);
    static void OnSubscriptionEstablished(void * context, chip::SubscriptionId subscriptionId) {
        CHIP_ERROR err = chip::JniReferences::GetInstance().CallSubscriptionEstablished(reinterpret_cast<CHIPInt64sAttributeCallback *>(context)->javaCallbackRef, subscriptionId);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Error calling onSubscriptionEstablished: %s", ErrorStr(err)));
    };

private:
    jobject javaCallbackRef;
    bool keepAlive;
};
class CHIPInt64uAttributeCallback : public chip::Callback::Callback<Int64uAttributeCallback>
{
public:
    CHIPInt64uAttributeCallback(jobject javaCallback, bool keepAlive = false);

    ~CHIPInt64uAttributeCallback();

    static void maybeDestroy(CHIPInt64uAttributeCallback * callback) {
        if (!callback->keepAlive) {
            callback->Cancel();
            chip::Platform::Delete<CHIPInt64uAttributeCallback>(callback);
        }
    }

    static void CallbackFn(void * context, uint64_t value);
    static void OnSubscriptionEstablished(void * context, chip::SubscriptionId subscriptionId) {
        CHIP_ERROR err = chip::JniReferences::GetInstance().CallSubscriptionEstablished(reinterpret_cast<CHIPInt64uAttributeCallback *>(context)->javaCallbackRef, subscriptionId);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Error calling onSubscriptionEstablished: %s", ErrorStr(err)));
    };

private:
    jobject javaCallbackRef;
    bool keepAlive;
};
class CHIPOctetStringAttributeCallback : public chip::Callback::Callback<OctetStringAttributeCallback>
{
public:
    CHIPOctetStringAttributeCallback(jobject javaCallback, bool keepAlive = false);

    ~CHIPOctetStringAttributeCallback();

    static void maybeDestroy(CHIPOctetStringAttributeCallback * callback) {
        if (!callback->keepAlive) {
            callback->Cancel();
            chip::Platform::Delete<CHIPOctetStringAttributeCallback>(callback);
        }
    }

    static void CallbackFn(void * context, const chip::ByteSpan value);
    static void OnSubscriptionEstablished(void * context, chip::SubscriptionId subscriptionId) {
        CHIP_ERROR err = chip::JniReferences::GetInstance().CallSubscriptionEstablished(reinterpret_cast<CHIPOctetStringAttributeCallback *>(context)->javaCallbackRef, subscriptionId);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Error calling onSubscriptionEstablished: %s", ErrorStr(err)));
    };

private:
    jobject javaCallbackRef;
    bool keepAlive;
};



  

  

  
class CHIPThirdSomeEnumAttributeCallback : public chip::Callback::Callback<CHIPThirdClusterSomeEnumAttributeCallbackType>
{
public:
    CHIPThirdSomeEnumAttributeCallback(jobject javaCallback, bool keepAlive = false);

    ~CHIPThirdSomeEnumAttributeCallback();

    static void maybeDestroy(CHIPThirdSomeEnumAttributeCallback * callback) {
        if (!callback->keepAlive) {
            callback->Cancel();
            chip::Platform::Delete<CHIPThirdSomeEnumAttributeCallback>(callback);
        }
    }

    static void CallbackFn(void * context, chip::app::Clusters::Third::MyEnum value);
    static void OnSubscriptionEstablished(void * context, chip::SubscriptionId subscriptionId) {
        CHIP_ERROR err = chip::JniReferences::GetInstance().CallSubscriptionEstablished(reinterpret_cast<CHIPThirdSomeEnumAttributeCallback *>(context)->javaCallbackRef, subscriptionId);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Error calling onSubscriptionEstablished: %s", ErrorStr(err)));
    };

private:
    jobject javaCallbackRef;
    bool keepAlive;
};

  
class CHIPThirdOptionsAttributeCallback : public chip::Callback::Callback<CHIPThirdClusterOptionsAttributeCallbackType>
{
public:
    CHIPThirdOptionsAttributeCallback(jobject javaCallback, bool keepAlive = false);

    ~CHIPThirdOptionsAttributeCallback();

    static void maybeDestroy(CHIPThirdOptionsAttributeCallback * callback) {
        if (!callback->keepAlive) {
            callback->Cancel();
            chip::Platform::Delete<CHIPThirdOptionsAttributeCallback>(callback);
        }
    }

    static void CallbackFn(void * context, chip::BitMask<chip::app::Clusters::Third::LevelControlOptions> value);
    static void OnSubscriptionEstablished(void * context, chip::SubscriptionId subscriptionId) {
        CHIP_ERROR err = chip::JniReferences::GetInstance().CallSubscriptionEstablished(reinterpret_cast<CHIPThirdOptionsAttributeCallback *>(context)->javaCallbackRef, subscriptionId);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Error calling onSubscriptionEstablished: %s", ErrorStr(err)));
    };

private:
    jobject javaCallbackRef;
    bool keepAlive;
};

  

