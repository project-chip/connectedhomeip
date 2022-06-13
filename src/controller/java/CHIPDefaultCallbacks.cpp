#include "CHIPDefaultCallbacks.h"
#include "AndroidClusterExceptions.h"

#include <lib/support/CHIPJNIError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>
#include <platform/PlatformManager.h>

chip::CHIPDefaultSuccessCallback::CHIPDefaultSuccessCallback(jobject javaCallback) :
    Callback::Callback<DefaultSuccessCallback>(CallbackFn, this)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr)
    {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
        return;
    }
    javaCallbackRef = env->NewGlobalRef(javaCallback);
    if (javaCallbackRef == nullptr)
    {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
    }
}

chip::CHIPDefaultSuccessCallback::~CHIPDefaultSuccessCallback()
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr)
    {
        ChipLogError(Zcl, "Could not delete global reference for Java callback");
        return;
    }
    env->DeleteGlobalRef(javaCallbackRef);
}

void chip::CHIPDefaultSuccessCallback::CallbackFn(void * context)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    jmethodID javaMethod;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    jobject javaCallbackRef;
    CHIPDefaultSuccessCallback * cppCallback = nullptr;

    VerifyOrExit(env != nullptr, err = CHIP_JNI_ERROR_NO_ENV);

    cppCallback = reinterpret_cast<CHIPDefaultSuccessCallback *>(context);
    VerifyOrExit(cppCallback != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    // It's valid for javaCallbackRef to be nullptr if the Java code passed in a null callback.
    javaCallbackRef = cppCallback->javaCallbackRef;
    VerifyOrExit(javaCallbackRef != nullptr, err = CHIP_NO_ERROR);

    err = JniReferences::GetInstance().FindMethod(env, javaCallbackRef, "onSuccess", "()V", &javaMethod);
    SuccessOrExit(err);

    env->ExceptionClear();
    env->CallVoidMethod(javaCallbackRef, javaMethod);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Error invoking Java callback: %" CHIP_ERROR_FORMAT, err.Format());
    }
    if (cppCallback != nullptr)
    {
        cppCallback->Cancel();
        delete cppCallback;
    }
}

chip::CHIPDefaultFailureCallback::CHIPDefaultFailureCallback(jobject javaCallback) :
    Callback::Callback<CHIPDefaultFailureCallbackType>(CallbackFn, this)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr)
    {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
        return;
    }
    javaCallbackRef = env->NewGlobalRef(javaCallback);
    if (javaCallbackRef == nullptr)
    {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
    }
}

chip::CHIPDefaultFailureCallback::~CHIPDefaultFailureCallback()
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr)
    {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
        return;
    }
    env->DeleteGlobalRef(javaCallbackRef);
}

void chip::CHIPDefaultFailureCallback::CallbackFn(void * context, CHIP_ERROR error)
{
    chip::app::StatusIB status(error);
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    jmethodID javaMethod;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    jobject javaCallbackRef;
    jthrowable exception;
    CHIPDefaultFailureCallback * cppCallback = nullptr;

    VerifyOrExit(env != nullptr, err = CHIP_JNI_ERROR_NO_ENV);

    cppCallback = reinterpret_cast<CHIPDefaultFailureCallback *>(context);
    VerifyOrExit(cppCallback != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    // It's valid for javaCallbackRef to be nullptr if the Java code passed in a null callback.
    javaCallbackRef = cppCallback->javaCallbackRef;
    VerifyOrExit(javaCallbackRef != nullptr, err = CHIP_NO_ERROR);

    err = JniReferences::GetInstance().FindMethod(env, javaCallbackRef, "onError", "(Ljava/lang/Exception;)V", &javaMethod);
    SuccessOrExit(err);

    // TODO: Figure out what to do with the non-StatusIB cases and the cases
    // when we have a cluster status?
    err = chip::AndroidClusterExceptions::GetInstance().CreateChipClusterException(env, chip::to_underlying(status.mStatus),
                                                                                   exception);
    SuccessOrExit(err);

    env->ExceptionClear();
    env->CallVoidMethod(javaCallbackRef, javaMethod, exception);
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Error invoking Java callback: %" CHIP_ERROR_FORMAT, err.Format());
    }
    if (cppCallback != nullptr)
    {
        cppCallback->Cancel();
        delete cppCallback;
    }
}
