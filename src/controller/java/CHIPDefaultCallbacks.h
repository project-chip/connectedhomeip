#include <jni.h>

#include <lib/core/CHIPError.h>
#include <zap-generated/CHIPClientCallbacks.h>

#include <jni/CHIPCallbackTypes.h>

namespace chip {

/** A success callback that delegates to the Java DefaultClusterCallback.onSuccess(). */
class CHIPDefaultSuccessCallback : public Callback::Callback<DefaultSuccessCallback>
{
public:
    CHIPDefaultSuccessCallback(jobject javaCallback);

    ~CHIPDefaultSuccessCallback();

    static void CallbackFn(void * context);

private:
    jobject javaCallbackRef;
};

/** A failure callback that delegates to the Java DefaultClusterCallback.onError(). */
class CHIPDefaultFailureCallback : public Callback::Callback<CHIPDefaultFailureCallbackType>
{
public:
    CHIPDefaultFailureCallback(jobject javaCallback);

    ~CHIPDefaultFailureCallback();

    static void CallbackFn(void * context, CHIP_ERROR error);

private:
    jobject javaCallbackRef;
};

} // namespace chip
