#include <jni.h>

#include <zap-generated/CHIPClientCallbacks.h>

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
class CHIPDefaultFailureCallback : public Callback::Callback<DefaultFailureCallback>
{
public:
    CHIPDefaultFailureCallback(jobject javaCallback);

    ~CHIPDefaultFailureCallback();

    static void CallbackFn(void * context, uint8_t status);

private:
    jobject javaCallbackRef;
};

} // namespace chip
