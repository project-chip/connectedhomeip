#include <jni.h>
#include <jni/CHIPReadCallbacks.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <platform/PlatformManager.h>
#include <controller/java/zap-generated/CHIPClientCallbacks.h>

CHIPBooleanAttributeCallback::CHIPBooleanAttributeCallback(jobject javaCallback, bool keepAlive) :
    chip::Callback::Callback<BooleanAttributeCallback>(CallbackFn, this), keepAlive(keepAlive)
{
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
        return;
    }
    javaCallbackRef = env->NewGlobalRef(javaCallback);
    if (javaCallbackRef == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
    }
}

CHIPBooleanAttributeCallback::~CHIPBooleanAttributeCallback() {
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr)
    {
        ChipLogError(Zcl, "Could not delete global reference for Java callback");
        return;
    }
    env->DeleteGlobalRef(javaCallbackRef);
}

void CHIPBooleanAttributeCallback::CallbackFn(void * context, bool value)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;

    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Could not get JNI env"));

    std::unique_ptr<CHIPBooleanAttributeCallback, decltype(&maybeDestroy)> cppCallback(reinterpret_cast<CHIPBooleanAttributeCallback *>(context), maybeDestroy);

    // It's valid for javaCallbackRef to be nullptr if the Java code passed in a
    // null callback.
    jobject javaCallbackRef = cppCallback.get()->javaCallbackRef;
    VerifyOrReturn(javaCallbackRef != nullptr, ChipLogDetail(Zcl, "Early return from attribute callback since Java callback is null"));

    jmethodID javaMethod;
    err = chip::JniReferences::GetInstance().FindMethod(env, javaCallbackRef, "onSuccess", "(Z)V", &javaMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Could not find onSuccess method"));
    env->CallVoidMethod(javaCallbackRef, javaMethod, static_cast<jboolean>(value));
}

CHIPCharStringAttributeCallback::CHIPCharStringAttributeCallback(jobject javaCallback, bool keepAlive) :
    chip::Callback::Callback<CharStringAttributeCallback>(CallbackFn, this), keepAlive(keepAlive)
{
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
        return;
    }
    javaCallbackRef = env->NewGlobalRef(javaCallback);
    if (javaCallbackRef == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
    }
}

CHIPCharStringAttributeCallback::~CHIPCharStringAttributeCallback() {
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr)
    {
        ChipLogError(Zcl, "Could not delete global reference for Java callback");
        return;
    }
    env->DeleteGlobalRef(javaCallbackRef);
}

void CHIPCharStringAttributeCallback::CallbackFn(void * context, const chip::CharSpan value)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;

    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Could not get JNI env"));

    std::unique_ptr<CHIPCharStringAttributeCallback, decltype(&maybeDestroy)> cppCallback(reinterpret_cast<CHIPCharStringAttributeCallback *>(context), maybeDestroy);

    // It's valid for javaCallbackRef to be nullptr if the Java code passed in a
    // null callback.
    jobject javaCallbackRef = cppCallback.get()->javaCallbackRef;
    VerifyOrReturn(javaCallbackRef != nullptr, ChipLogDetail(Zcl, "Early return from attribute callback since Java callback is null"));

    jmethodID javaMethod;
    err = chip::JniReferences::GetInstance().FindMethod(env, javaCallbackRef, "onSuccess", "(Ljava/lang/String;)V", &javaMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Could not find onSuccess method"));

    chip::UtfString valueStr(env, value);
    env->CallVoidMethod(javaCallbackRef, javaMethod, valueStr.jniValue());
}

CHIPDoubleAttributeCallback::CHIPDoubleAttributeCallback(jobject javaCallback, bool keepAlive) :
    chip::Callback::Callback<DoubleAttributeCallback>(CallbackFn, this), keepAlive(keepAlive)
{
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
        return;
    }
    javaCallbackRef = env->NewGlobalRef(javaCallback);
    if (javaCallbackRef == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
    }
}

CHIPDoubleAttributeCallback::~CHIPDoubleAttributeCallback() {
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr)
    {
        ChipLogError(Zcl, "Could not delete global reference for Java callback");
        return;
    }
    env->DeleteGlobalRef(javaCallbackRef);
}

void CHIPDoubleAttributeCallback::CallbackFn(void * context, double value)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;

    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Could not get JNI env"));

    std::unique_ptr<CHIPDoubleAttributeCallback, decltype(&maybeDestroy)> cppCallback(reinterpret_cast<CHIPDoubleAttributeCallback *>(context), maybeDestroy);

    // It's valid for javaCallbackRef to be nullptr if the Java code passed in a
    // null callback.
    jobject javaCallbackRef = cppCallback.get()->javaCallbackRef;
    VerifyOrReturn(javaCallbackRef != nullptr, ChipLogDetail(Zcl, "Early return from attribute callback since Java callback is null"));

    jmethodID javaMethod;
    err = chip::JniReferences::GetInstance().FindMethod(env, javaCallbackRef, "onSuccess", "(D)V", &javaMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Could not find onSuccess method"));
    env->CallVoidMethod(javaCallbackRef, javaMethod, static_cast<jdouble>(value));
}

CHIPFloatAttributeCallback::CHIPFloatAttributeCallback(jobject javaCallback, bool keepAlive) :
    chip::Callback::Callback<FloatAttributeCallback>(CallbackFn, this), keepAlive(keepAlive)
{
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
        return;
    }
    javaCallbackRef = env->NewGlobalRef(javaCallback);
    if (javaCallbackRef == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
    }
}

CHIPFloatAttributeCallback::~CHIPFloatAttributeCallback() {
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr)
    {
        ChipLogError(Zcl, "Could not delete global reference for Java callback");
        return;
    }
    env->DeleteGlobalRef(javaCallbackRef);
}

void CHIPFloatAttributeCallback::CallbackFn(void * context, float value)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;

    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Could not get JNI env"));

    std::unique_ptr<CHIPFloatAttributeCallback, decltype(&maybeDestroy)> cppCallback(reinterpret_cast<CHIPFloatAttributeCallback *>(context), maybeDestroy);

    // It's valid for javaCallbackRef to be nullptr if the Java code passed in a
    // null callback.
    jobject javaCallbackRef = cppCallback.get()->javaCallbackRef;
    VerifyOrReturn(javaCallbackRef != nullptr, ChipLogDetail(Zcl, "Early return from attribute callback since Java callback is null"));

    jmethodID javaMethod;
    err = chip::JniReferences::GetInstance().FindMethod(env, javaCallbackRef, "onSuccess", "(F)V", &javaMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Could not find onSuccess method"));
    env->CallVoidMethod(javaCallbackRef, javaMethod, static_cast<jfloat>(value));
}

CHIPInt8sAttributeCallback::CHIPInt8sAttributeCallback(jobject javaCallback, bool keepAlive) :
    chip::Callback::Callback<Int8sAttributeCallback>(CallbackFn, this), keepAlive(keepAlive)
{
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
        return;
    }
    javaCallbackRef = env->NewGlobalRef(javaCallback);
    if (javaCallbackRef == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
    }
}

CHIPInt8sAttributeCallback::~CHIPInt8sAttributeCallback() {
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr)
    {
        ChipLogError(Zcl, "Could not delete global reference for Java callback");
        return;
    }
    env->DeleteGlobalRef(javaCallbackRef);
}

void CHIPInt8sAttributeCallback::CallbackFn(void * context, int8_t value)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;

    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Could not get JNI env"));

    std::unique_ptr<CHIPInt8sAttributeCallback, decltype(&maybeDestroy)> cppCallback(reinterpret_cast<CHIPInt8sAttributeCallback *>(context), maybeDestroy);

    // It's valid for javaCallbackRef to be nullptr if the Java code passed in a
    // null callback.
    jobject javaCallbackRef = cppCallback.get()->javaCallbackRef;
    VerifyOrReturn(javaCallbackRef != nullptr, ChipLogDetail(Zcl, "Early return from attribute callback since Java callback is null"));

    jmethodID javaMethod;
    err = chip::JniReferences::GetInstance().FindMethod(env, javaCallbackRef, "onSuccess", "(I)V", &javaMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Could not find onSuccess method"));
    env->CallVoidMethod(javaCallbackRef, javaMethod, static_cast<jint>(value));
}

CHIPInt8uAttributeCallback::CHIPInt8uAttributeCallback(jobject javaCallback, bool keepAlive) :
    chip::Callback::Callback<Int8uAttributeCallback>(CallbackFn, this), keepAlive(keepAlive)
{
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
        return;
    }
    javaCallbackRef = env->NewGlobalRef(javaCallback);
    if (javaCallbackRef == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
    }
}

CHIPInt8uAttributeCallback::~CHIPInt8uAttributeCallback() {
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr)
    {
        ChipLogError(Zcl, "Could not delete global reference for Java callback");
        return;
    }
    env->DeleteGlobalRef(javaCallbackRef);
}

void CHIPInt8uAttributeCallback::CallbackFn(void * context, uint8_t value)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;

    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Could not get JNI env"));

    std::unique_ptr<CHIPInt8uAttributeCallback, decltype(&maybeDestroy)> cppCallback(reinterpret_cast<CHIPInt8uAttributeCallback *>(context), maybeDestroy);

    // It's valid for javaCallbackRef to be nullptr if the Java code passed in a
    // null callback.
    jobject javaCallbackRef = cppCallback.get()->javaCallbackRef;
    VerifyOrReturn(javaCallbackRef != nullptr, ChipLogDetail(Zcl, "Early return from attribute callback since Java callback is null"));

    jmethodID javaMethod;
    err = chip::JniReferences::GetInstance().FindMethod(env, javaCallbackRef, "onSuccess", "(I)V", &javaMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Could not find onSuccess method"));
    env->CallVoidMethod(javaCallbackRef, javaMethod, static_cast<jint>(value));
}

CHIPInt16sAttributeCallback::CHIPInt16sAttributeCallback(jobject javaCallback, bool keepAlive) :
    chip::Callback::Callback<Int16sAttributeCallback>(CallbackFn, this), keepAlive(keepAlive)
{
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
        return;
    }
    javaCallbackRef = env->NewGlobalRef(javaCallback);
    if (javaCallbackRef == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
    }
}

CHIPInt16sAttributeCallback::~CHIPInt16sAttributeCallback() {
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr)
    {
        ChipLogError(Zcl, "Could not delete global reference for Java callback");
        return;
    }
    env->DeleteGlobalRef(javaCallbackRef);
}

void CHIPInt16sAttributeCallback::CallbackFn(void * context, int16_t value)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;

    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Could not get JNI env"));

    std::unique_ptr<CHIPInt16sAttributeCallback, decltype(&maybeDestroy)> cppCallback(reinterpret_cast<CHIPInt16sAttributeCallback *>(context), maybeDestroy);

    // It's valid for javaCallbackRef to be nullptr if the Java code passed in a
    // null callback.
    jobject javaCallbackRef = cppCallback.get()->javaCallbackRef;
    VerifyOrReturn(javaCallbackRef != nullptr, ChipLogDetail(Zcl, "Early return from attribute callback since Java callback is null"));

    jmethodID javaMethod;
    err = chip::JniReferences::GetInstance().FindMethod(env, javaCallbackRef, "onSuccess", "(I)V", &javaMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Could not find onSuccess method"));
    env->CallVoidMethod(javaCallbackRef, javaMethod, static_cast<jint>(value));
}

CHIPInt16uAttributeCallback::CHIPInt16uAttributeCallback(jobject javaCallback, bool keepAlive) :
    chip::Callback::Callback<Int16uAttributeCallback>(CallbackFn, this), keepAlive(keepAlive)
{
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
        return;
    }
    javaCallbackRef = env->NewGlobalRef(javaCallback);
    if (javaCallbackRef == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
    }
}

CHIPInt16uAttributeCallback::~CHIPInt16uAttributeCallback() {
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr)
    {
        ChipLogError(Zcl, "Could not delete global reference for Java callback");
        return;
    }
    env->DeleteGlobalRef(javaCallbackRef);
}

void CHIPInt16uAttributeCallback::CallbackFn(void * context, uint16_t value)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;

    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Could not get JNI env"));

    std::unique_ptr<CHIPInt16uAttributeCallback, decltype(&maybeDestroy)> cppCallback(reinterpret_cast<CHIPInt16uAttributeCallback *>(context), maybeDestroy);

    // It's valid for javaCallbackRef to be nullptr if the Java code passed in a
    // null callback.
    jobject javaCallbackRef = cppCallback.get()->javaCallbackRef;
    VerifyOrReturn(javaCallbackRef != nullptr, ChipLogDetail(Zcl, "Early return from attribute callback since Java callback is null"));

    jmethodID javaMethod;
    err = chip::JniReferences::GetInstance().FindMethod(env, javaCallbackRef, "onSuccess", "(I)V", &javaMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Could not find onSuccess method"));
    env->CallVoidMethod(javaCallbackRef, javaMethod, static_cast<jint>(value));
}

CHIPInt32sAttributeCallback::CHIPInt32sAttributeCallback(jobject javaCallback, bool keepAlive) :
    chip::Callback::Callback<Int32sAttributeCallback>(CallbackFn, this), keepAlive(keepAlive)
{
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
        return;
    }
    javaCallbackRef = env->NewGlobalRef(javaCallback);
    if (javaCallbackRef == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
    }
}

CHIPInt32sAttributeCallback::~CHIPInt32sAttributeCallback() {
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr)
    {
        ChipLogError(Zcl, "Could not delete global reference for Java callback");
        return;
    }
    env->DeleteGlobalRef(javaCallbackRef);
}

void CHIPInt32sAttributeCallback::CallbackFn(void * context, int32_t value)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;

    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Could not get JNI env"));

    std::unique_ptr<CHIPInt32sAttributeCallback, decltype(&maybeDestroy)> cppCallback(reinterpret_cast<CHIPInt32sAttributeCallback *>(context), maybeDestroy);

    // It's valid for javaCallbackRef to be nullptr if the Java code passed in a
    // null callback.
    jobject javaCallbackRef = cppCallback.get()->javaCallbackRef;
    VerifyOrReturn(javaCallbackRef != nullptr, ChipLogDetail(Zcl, "Early return from attribute callback since Java callback is null"));

    jmethodID javaMethod;
    err = chip::JniReferences::GetInstance().FindMethod(env, javaCallbackRef, "onSuccess", "(J)V", &javaMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Could not find onSuccess method"));
    env->CallVoidMethod(javaCallbackRef, javaMethod, static_cast<jlong>(value));
}

CHIPInt32uAttributeCallback::CHIPInt32uAttributeCallback(jobject javaCallback, bool keepAlive) :
    chip::Callback::Callback<Int32uAttributeCallback>(CallbackFn, this), keepAlive(keepAlive)
{
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
        return;
    }
    javaCallbackRef = env->NewGlobalRef(javaCallback);
    if (javaCallbackRef == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
    }
}

CHIPInt32uAttributeCallback::~CHIPInt32uAttributeCallback() {
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr)
    {
        ChipLogError(Zcl, "Could not delete global reference for Java callback");
        return;
    }
    env->DeleteGlobalRef(javaCallbackRef);
}

void CHIPInt32uAttributeCallback::CallbackFn(void * context, uint32_t value)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;

    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Could not get JNI env"));

    std::unique_ptr<CHIPInt32uAttributeCallback, decltype(&maybeDestroy)> cppCallback(reinterpret_cast<CHIPInt32uAttributeCallback *>(context), maybeDestroy);

    // It's valid for javaCallbackRef to be nullptr if the Java code passed in a
    // null callback.
    jobject javaCallbackRef = cppCallback.get()->javaCallbackRef;
    VerifyOrReturn(javaCallbackRef != nullptr, ChipLogDetail(Zcl, "Early return from attribute callback since Java callback is null"));

    jmethodID javaMethod;
    err = chip::JniReferences::GetInstance().FindMethod(env, javaCallbackRef, "onSuccess", "(J)V", &javaMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Could not find onSuccess method"));
    env->CallVoidMethod(javaCallbackRef, javaMethod, static_cast<jlong>(value));
}

CHIPInt64sAttributeCallback::CHIPInt64sAttributeCallback(jobject javaCallback, bool keepAlive) :
    chip::Callback::Callback<Int64sAttributeCallback>(CallbackFn, this), keepAlive(keepAlive)
{
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
        return;
    }
    javaCallbackRef = env->NewGlobalRef(javaCallback);
    if (javaCallbackRef == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
    }
}

CHIPInt64sAttributeCallback::~CHIPInt64sAttributeCallback() {
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr)
    {
        ChipLogError(Zcl, "Could not delete global reference for Java callback");
        return;
    }
    env->DeleteGlobalRef(javaCallbackRef);
}

void CHIPInt64sAttributeCallback::CallbackFn(void * context, int64_t value)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;

    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Could not get JNI env"));

    std::unique_ptr<CHIPInt64sAttributeCallback, decltype(&maybeDestroy)> cppCallback(reinterpret_cast<CHIPInt64sAttributeCallback *>(context), maybeDestroy);

    // It's valid for javaCallbackRef to be nullptr if the Java code passed in a
    // null callback.
    jobject javaCallbackRef = cppCallback.get()->javaCallbackRef;
    VerifyOrReturn(javaCallbackRef != nullptr, ChipLogDetail(Zcl, "Early return from attribute callback since Java callback is null"));

    jmethodID javaMethod;
    err = chip::JniReferences::GetInstance().FindMethod(env, javaCallbackRef, "onSuccess", "(J)V", &javaMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Could not find onSuccess method"));
    env->CallVoidMethod(javaCallbackRef, javaMethod, static_cast<jlong>(value));
}

CHIPInt64uAttributeCallback::CHIPInt64uAttributeCallback(jobject javaCallback, bool keepAlive) :
    chip::Callback::Callback<Int64uAttributeCallback>(CallbackFn, this), keepAlive(keepAlive)
{
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
        return;
    }
    javaCallbackRef = env->NewGlobalRef(javaCallback);
    if (javaCallbackRef == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
    }
}

CHIPInt64uAttributeCallback::~CHIPInt64uAttributeCallback() {
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr)
    {
        ChipLogError(Zcl, "Could not delete global reference for Java callback");
        return;
    }
    env->DeleteGlobalRef(javaCallbackRef);
}

void CHIPInt64uAttributeCallback::CallbackFn(void * context, uint64_t value)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;

    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Could not get JNI env"));

    std::unique_ptr<CHIPInt64uAttributeCallback, decltype(&maybeDestroy)> cppCallback(reinterpret_cast<CHIPInt64uAttributeCallback *>(context), maybeDestroy);

    // It's valid for javaCallbackRef to be nullptr if the Java code passed in a
    // null callback.
    jobject javaCallbackRef = cppCallback.get()->javaCallbackRef;
    VerifyOrReturn(javaCallbackRef != nullptr, ChipLogDetail(Zcl, "Early return from attribute callback since Java callback is null"));

    jmethodID javaMethod;
    err = chip::JniReferences::GetInstance().FindMethod(env, javaCallbackRef, "onSuccess", "(J)V", &javaMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Could not find onSuccess method"));
    env->CallVoidMethod(javaCallbackRef, javaMethod, static_cast<jlong>(value));
}

CHIPOctetStringAttributeCallback::CHIPOctetStringAttributeCallback(jobject javaCallback, bool keepAlive) :
    chip::Callback::Callback<OctetStringAttributeCallback>(CallbackFn, this), keepAlive(keepAlive)
{
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
        return;
    }
    javaCallbackRef = env->NewGlobalRef(javaCallback);
    if (javaCallbackRef == nullptr) {
        ChipLogError(Zcl, "Could not create global reference for Java callback");
    }
}

CHIPOctetStringAttributeCallback::~CHIPOctetStringAttributeCallback() {
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    if (env == nullptr)
    {
        ChipLogError(Zcl, "Could not delete global reference for Java callback");
        return;
    }
    env->DeleteGlobalRef(javaCallbackRef);
}

void CHIPOctetStringAttributeCallback::CallbackFn(void * context, const chip::ByteSpan value)
{
    chip::DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;

    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Could not get JNI env"));

    std::unique_ptr<CHIPOctetStringAttributeCallback, decltype(&maybeDestroy)> cppCallback(reinterpret_cast<CHIPOctetStringAttributeCallback *>(context), maybeDestroy);

    // It's valid for javaCallbackRef to be nullptr if the Java code passed in a
    // null callback.
    jobject javaCallbackRef = cppCallback.get()->javaCallbackRef;
    VerifyOrReturn(javaCallbackRef != nullptr, ChipLogDetail(Zcl, "Early return from attribute callback since Java callback is null"));

    jmethodID javaMethod;
    err = chip::JniReferences::GetInstance().FindMethod(env, javaCallbackRef, "onSuccess", "([B)V", &javaMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Zcl, "Could not find onSuccess method"));

    VerifyOrReturn(chip::CanCastTo<uint32_t>(value.size()), ChipLogError(Zcl, "Value too long"));
    jbyteArray valueArr = env->NewByteArray(static_cast<uint32_t>(value.size()));
    env->ExceptionClear();
    env->SetByteArrayRegion(valueArr, 0, static_cast<uint32_t>(value.size()), reinterpret_cast<const jbyte *>(value.data()));

    env->CallVoidMethod(javaCallbackRef, javaMethod, valueArr);
}


