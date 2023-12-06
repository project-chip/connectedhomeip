/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

#include <cstring>
#include <jni.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

namespace chip {

void JniReferences::SetJavaVm(JavaVM * jvm, const char * clsType)
{
    VerifyOrReturn(mJvm == nullptr, ChipLogError(Support, "JavaVM is already set"));
    mJvm = jvm;

    // Cache the classloader for CHIP Java classes. env->FindClass() can start in the system class loader if called from a different
    // thread, meaning it won't find our Chip classes.
    // https://developer.android.com/training/articles/perf-jni#faq_FindClass
    JNIEnv * env = GetEnvForCurrentThread();
    // Any chip.devicecontroller.* class will work here - just need something to call getClassLoader() on.
    jclass chipClass = env->FindClass(clsType);
    VerifyOrReturn(chipClass != nullptr, ChipLogError(Support, "clsType can not found"));

    jclass classClass              = env->FindClass("java/lang/Class");
    jclass classLoaderClass        = env->FindClass("java/lang/ClassLoader");
    jmethodID getClassLoaderMethod = env->GetMethodID(classClass, "getClassLoader", "()Ljava/lang/ClassLoader;");

    mClassLoader     = env->NewGlobalRef(env->CallObjectMethod(chipClass, getClassLoaderMethod));
    mFindClassMethod = env->GetMethodID(classLoaderClass, "findClass", "(Ljava/lang/String;)Ljava/lang/Class;");

    chip::JniReferences::GetInstance().GetClassRef(env, "java/util/List", mListClass);
    chip::JniReferences::GetInstance().GetClassRef(env, "java/util/ArrayList", mArrayListClass);
    chip::JniReferences::GetInstance().GetClassRef(env, "java/util/HashMap", mHashMapClass);
}

JNIEnv * JniReferences::GetEnvForCurrentThread()
{
    JNIEnv * env;
    if (mJvm == nullptr)
    {
        ChipLogError(Support, "Missing Java VM");
        return nullptr;
    }
    mJvm->GetEnv((void **) &env, JNI_VERSION_1_6);
    if (env == nullptr)
    {
        jint error;
#ifdef __ANDROID__
        error = mJvm->AttachCurrentThreadAsDaemon(&env, nullptr);
#else
        error = mJvm->AttachCurrentThreadAsDaemon((void **) &env, nullptr);
#endif
        if (error != JNI_OK)
        {
            ChipLogError(Support, "Failed to get JNIEnv for the current thread");
            return nullptr;
        }
    }
    return env;
}

CHIP_ERROR JniReferences::GetClassRef(JNIEnv * env, const char * clsType, jclass & outCls)
{
    jclass cls     = nullptr;
    CHIP_ERROR err = GetLocalClassRef(env, clsType, cls);
    ReturnErrorOnFailure(err);
    outCls = (jclass) env->NewGlobalRef((jobject) cls);
    VerifyOrReturnError(outCls != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);

    return err;
}

CHIP_ERROR JniReferences::GetLocalClassRef(JNIEnv * env, const char * clsType, jclass & outCls)
{
    jclass cls = nullptr;

    // Try `j$/util/Optional` when enabling Java8.
    if (strcmp(clsType, "java/util/Optional") == 0)
    {
        cls = env->FindClass("j$/util/Optional");
        env->ExceptionClear();
    }

    if (cls == nullptr)
    {
        cls = env->FindClass(clsType);
        env->ExceptionClear();
    }

    if (cls == nullptr)
    {
        // Try the cached classloader if FindClass() didn't work.
        cls = static_cast<jclass>(env->CallObjectMethod(mClassLoader, mFindClassMethod, env->NewStringUTF(clsType)));
        VerifyOrReturnError(cls != nullptr && env->ExceptionCheck() != JNI_TRUE, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
    }

    outCls = cls;
    return CHIP_NO_ERROR;
}

CHIP_ERROR JniReferences::N2J_ByteArray(JNIEnv * env, const uint8_t * inArray, jsize inArrayLen, jbyteArray & outArray)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    outArray = env->NewByteArray(inArrayLen);
    VerifyOrReturnError(outArray != NULL, CHIP_ERROR_NO_MEMORY);

    env->ExceptionClear();
    env->SetByteArrayRegion(outArray, 0, inArrayLen, (jbyte *) inArray);
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

exit:
    return err;
}

CHIP_ERROR JniReferences::FindMethod(JNIEnv * env, jobject object, const char * methodName, const char * methodSignature,
                                     jmethodID * methodId)
{
    jclass javaClass = nullptr;
    VerifyOrReturnError(env != nullptr && object != nullptr, CHIP_JNI_ERROR_NULL_OBJECT);

    javaClass = env->GetObjectClass(object);
    VerifyOrReturnError(javaClass != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);

    *methodId = env->GetMethodID(javaClass, methodName, methodSignature);
    env->ExceptionClear();

    if (*methodId != nullptr)
    {
        return CHIP_NO_ERROR;
    }

    // Try `j$` when enabling Java8.
    std::string methodSignature_java8_str(methodSignature);
    size_t pos = methodSignature_java8_str.find("java/util/Optional");
    if (pos != std::string::npos)
    {
        // Replace all "java/util/Optional" with "j$/util/Optional".
        while (pos != std::string::npos)
        {
            methodSignature_java8_str.replace(pos, strlen("java/util/Optional"), "j$/util/Optional");
            pos = methodSignature_java8_str.find("java/util/Optional");
        }
        *methodId = env->GetMethodID(javaClass, methodName, methodSignature_java8_str.c_str());
        env->ExceptionClear();
    }

    VerifyOrReturnError(*methodId != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    return CHIP_NO_ERROR;
}

void JniReferences::CallVoidInt(JNIEnv * env, jobject object, const char * methodName, jint argument)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    jmethodID method;

    err = JniReferences::FindMethod(env, object, methodName, "(I)V", &method);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Support, "Error finding Java method: %" CHIP_ERROR_FORMAT, err.Format());
    }

    env->ExceptionClear();
    env->CallVoidMethod(object, method, argument);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

void JniReferences::ReportError(JNIEnv * env, CHIP_ERROR cbErr, const char * functName)
{
    if (cbErr == CHIP_JNI_ERROR_EXCEPTION_THROWN)
    {
        ChipLogError(Support, "Java exception thrown in %s", StringOrNullMarker(functName));
        env->ExceptionDescribe();
    }
    else
    {
        const char * errStr;
        switch (cbErr.AsInteger())
        {
        case CHIP_JNI_ERROR_TYPE_NOT_FOUND.AsInteger():
            errStr = "JNI type not found";
            break;
        case CHIP_JNI_ERROR_METHOD_NOT_FOUND.AsInteger():
            errStr = "JNI method not found";
            break;
        case CHIP_JNI_ERROR_FIELD_NOT_FOUND.AsInteger():
            errStr = "JNI field not found";
            break;
        default:
            errStr = ErrorStr(cbErr);
            break;
        }
        ChipLogError(Support, "Error in %s : %s", StringOrNullMarker(functName), errStr);
    }
}

void JniReferences::ThrowError(JNIEnv * env, jclass exceptionCls, CHIP_ERROR errToThrow)
{
    env->ExceptionClear();
    jmethodID constructor = env->GetMethodID(exceptionCls, "<init>", "(JLjava/lang/String;)V");
    VerifyOrReturn(constructor != NULL);

    jstring jerrStr = env->NewStringUTF(ErrorStr(errToThrow));

    jthrowable outEx = (jthrowable) env->NewObject(exceptionCls, constructor, static_cast<jlong>(errToThrow.AsInteger()), jerrStr);
    VerifyOrReturn(!env->ExceptionCheck());
    env->Throw(outEx);
}

CHIP_ERROR JniReferences::CreateOptional(jobject objectToWrap, jobject & outOptional)
{
    JNIEnv * env = GetEnvForCurrentThread();
    jclass optionalCls;
    chip::JniReferences::GetInstance().GetClassRef(env, "java/util/Optional", optionalCls);
    VerifyOrReturnError(optionalCls != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
    chip::JniClass jniClass(optionalCls);

    jmethodID ofMethod = env->GetStaticMethodID(optionalCls, "ofNullable", "(Ljava/lang/Object;)Ljava/util/Optional;");
    env->ExceptionClear();

    // Try `Lj$/util/Optional;` when enabling Java8.
    if (ofMethod == nullptr)
    {
        ofMethod = env->GetStaticMethodID(optionalCls, "ofNullable", "(Ljava/lang/Object;)Lj$/util/Optional;");
        env->ExceptionClear();
    }

    VerifyOrReturnError(ofMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);
    outOptional = env->CallStaticObjectMethod(optionalCls, ofMethod, objectToWrap);

    VerifyOrReturnError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

    return CHIP_NO_ERROR;
}

CHIP_ERROR JniReferences::GetOptionalValue(jobject optionalObj, jobject & optionalValue)
{
    JNIEnv * env = GetEnvForCurrentThread();
    jclass optionalCls;
    chip::JniReferences::GetInstance().GetClassRef(env, "java/util/Optional", optionalCls);
    VerifyOrReturnError(optionalCls != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
    chip::JniClass jniClass(optionalCls);

    jmethodID isPresentMethod = env->GetMethodID(optionalCls, "isPresent", "()Z");
    VerifyOrReturnError(isPresentMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);
    jboolean isPresent = optionalObj && env->CallBooleanMethod(optionalObj, isPresentMethod);

    if (!isPresent)
    {
        optionalValue = nullptr;
        return CHIP_NO_ERROR;
    }

    jmethodID getMethod = env->GetMethodID(optionalCls, "get", "()Ljava/lang/Object;");
    VerifyOrReturnError(getMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);
    optionalValue = env->CallObjectMethod(optionalObj, getMethod);
    return CHIP_NO_ERROR;
}

jint JniReferences::IntegerToPrimitive(jobject boxedInteger)
{
    JNIEnv * env = GetEnvForCurrentThread();
    jclass boxedTypeCls;
    chip::JniReferences::GetInstance().GetClassRef(env, "java/lang/Integer", boxedTypeCls);
    chip::JniClass jniClass(boxedTypeCls);

    jmethodID valueMethod = env->GetMethodID(boxedTypeCls, "intValue", "()I");
    return env->CallIntMethod(boxedInteger, valueMethod);
}

jlong JniReferences::LongToPrimitive(jobject boxedLong)
{
    JNIEnv * env = GetEnvForCurrentThread();
    jclass boxedTypeCls;
    chip::JniReferences::GetInstance().GetClassRef(env, "java/lang/Long", boxedTypeCls);
    chip::JniClass jniClass(boxedTypeCls);

    jmethodID valueMethod = env->GetMethodID(boxedTypeCls, "longValue", "()J");
    return env->CallLongMethod(boxedLong, valueMethod);
}

jboolean JniReferences::BooleanToPrimitive(jobject boxedBoolean)
{
    JNIEnv * env = GetEnvForCurrentThread();
    jclass boxedTypeCls;
    chip::JniReferences::GetInstance().GetClassRef(env, "java/lang/Boolean", boxedTypeCls);
    chip::JniClass jniClass(boxedTypeCls);

    jmethodID valueMethod = env->GetMethodID(boxedTypeCls, "booleanValue", "()Z");
    return env->CallBooleanMethod(boxedBoolean, valueMethod);
}

jfloat JniReferences::FloatToPrimitive(jobject boxedFloat)
{
    JNIEnv * env = GetEnvForCurrentThread();
    jclass boxedTypeCls;
    chip::JniReferences::GetInstance().GetClassRef(env, "java/lang/Float", boxedTypeCls);
    chip::JniClass jniClass(boxedTypeCls);

    jmethodID valueMethod = env->GetMethodID(boxedTypeCls, "floatValue", "()F");
    return env->CallFloatMethod(boxedFloat, valueMethod);
}

jdouble JniReferences::DoubleToPrimitive(jobject boxedDouble)
{
    JNIEnv * env = GetEnvForCurrentThread();
    jclass boxedTypeCls;
    chip::JniReferences::GetInstance().GetClassRef(env, "java/lang/Double", boxedTypeCls);
    chip::JniClass jniClass(boxedTypeCls);

    jmethodID valueMethod = env->GetMethodID(boxedTypeCls, "doubleValue", "()D");
    return env->CallDoubleMethod(boxedDouble, valueMethod);
}

CHIP_ERROR JniReferences::CallSubscriptionEstablished(jobject javaCallback, long subscriptionId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = chip::JniReferences::GetInstance().GetEnvForCurrentThread();

    jmethodID subscriptionEstablishedMethod;
    err = chip::JniReferences::GetInstance().FindMethod(env, javaCallback, "onSubscriptionEstablished", "(J)V",
                                                        &subscriptionEstablishedMethod);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    env->CallVoidMethod(javaCallback, subscriptionEstablishedMethod, static_cast<jlong>(subscriptionId));
    VerifyOrReturnError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

    return err;
}

CHIP_ERROR JniReferences::CreateArrayList(jobject & outList)
{
    JNIEnv * env   = GetEnvForCurrentThread();
    CHIP_ERROR err = CHIP_NO_ERROR;

    jmethodID arrayListCtor = env->GetMethodID(mArrayListClass, "<init>", "()V");
    VerifyOrReturnError(arrayListCtor != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);
    outList = env->NewObject(mArrayListClass, arrayListCtor);
    VerifyOrReturnError(outList != nullptr, CHIP_JNI_ERROR_NULL_OBJECT);

    return err;
}

CHIP_ERROR JniReferences::AddToList(jobject list, jobject objectToAdd)
{
    JNIEnv * env   = GetEnvForCurrentThread();
    CHIP_ERROR err = CHIP_NO_ERROR;

    jmethodID addMethod = env->GetMethodID(mListClass, "add", "(Ljava/lang/Object;)Z");
    VerifyOrReturnError(addMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    env->CallBooleanMethod(list, addMethod, objectToAdd);
    VerifyOrReturnError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);
    return err;
}

CHIP_ERROR JniReferences::CreateHashMap(jobject & outMap)
{
    JNIEnv * env   = GetEnvForCurrentThread();
    CHIP_ERROR err = CHIP_NO_ERROR;

    jmethodID hashMapCtor = env->GetMethodID(mHashMapClass, "<init>", "()V");
    VerifyOrReturnError(hashMapCtor != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);
    outMap = env->NewObject(mHashMapClass, hashMapCtor);
    VerifyOrReturnError(outMap != nullptr, CHIP_JNI_ERROR_NULL_OBJECT);

    return err;
}

CHIP_ERROR JniReferences::PutInMap(jobject map, jobject key, jobject value)
{
    JNIEnv * env   = GetEnvForCurrentThread();
    CHIP_ERROR err = CHIP_NO_ERROR;

    jmethodID putMethod = env->GetMethodID(mHashMapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    VerifyOrReturnError(putMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    env->CallObjectMethod(map, putMethod, key, value);
    VerifyOrReturnError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);
    return err;
}

CHIP_ERROR JniReferences::GetListSize(jobject list, jint & size)
{
    JNIEnv * env   = GetEnvForCurrentThread();
    CHIP_ERROR err = CHIP_NO_ERROR;

    jmethodID sizeMethod = env->GetMethodID(mListClass, "size", "()I");
    VerifyOrReturnError(sizeMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    size = env->CallIntMethod(list, sizeMethod);
    VerifyOrReturnError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);
    return err;
}

CHIP_ERROR JniReferences::GetListItem(jobject list, jint index, jobject & outItem)
{
    JNIEnv * env   = GetEnvForCurrentThread();
    CHIP_ERROR err = CHIP_NO_ERROR;

    jmethodID getMethod = env->GetMethodID(mListClass, "get", "(I)Ljava/lang/Object;");
    VerifyOrReturnError(getMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    outItem = env->CallObjectMethod(list, getMethod, index);
    VerifyOrReturnError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);
    return err;
}

CHIP_ERROR JniReferences::GetObjectField(jobject objectToRead, const char * name, const char * signature, jobject & outObject)
{
    JNIEnv * env   = GetEnvForCurrentThread();
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(objectToRead != nullptr, CHIP_JNI_ERROR_NULL_OBJECT);

    jclass objClass = env->GetObjectClass(objectToRead);
    jfieldID field  = env->GetFieldID(objClass, name, signature);

    outObject = env->GetObjectField(objectToRead, field);
    return err;
}

CHIP_ERROR JniReferences::CharToStringUTF(const chip::CharSpan & charSpan, jobject & outStr)
{
    JNIEnv * env        = GetEnvForCurrentThread();
    jobject jbyteBuffer = env->NewDirectByteBuffer((void *) charSpan.data(), static_cast<jlong>(charSpan.size()));

    jclass charSetClass = env->FindClass("java/nio/charset/Charset");
    jmethodID charsetForNameMethod =
        env->GetStaticMethodID(charSetClass, "forName", "(Ljava/lang/String;)Ljava/nio/charset/Charset;");
    jobject charsetObject = env->CallStaticObjectMethod(charSetClass, charsetForNameMethod, env->NewStringUTF("UTF-8"));

    jclass charSetDocoderClass = env->FindClass("java/nio/charset/CharsetDecoder");
    jmethodID newDocoderMethod = env->GetMethodID(charSetClass, "newDecoder", "()Ljava/nio/charset/CharsetDecoder;");
    jobject decoderObject      = env->CallObjectMethod(charsetObject, newDocoderMethod);

    // Even though spec requires UTF-8 strings, we have seen instances in the field of certified devices sending
    // invalid strings like "startup?" (0x73 0x74 0x61 0x72 0x74 0x75 0x70 <0x91>) and we want to actually
    // be lenient on those rather than failing an entire decode (which may fail an entire report for one invalid string,
    // like in a very common 'subscribe *')
    //
    // As a result call:
    //   onMalformedInput(CodingErrorAction.REPLACE)
    //   onUnmappableCharacter(CodingErrorAction.REPLACE)
    jclass codingErrorActionClass = env->FindClass("java/nio/charset/CodingErrorAction");
    jobject replaceAction         = env->GetStaticObjectField(
        codingErrorActionClass, env->GetStaticFieldID(codingErrorActionClass, "REPLACE", "Ljava/nio/charset/CodingErrorAction;"));
    {
        jmethodID onMalformedInput = env->GetMethodID(charSetDocoderClass, "onMalformedInput",
                                                      "(Ljava/nio/charset/CodingErrorAction;)Ljava/nio/charset/CharsetDecoder;");
        decoderObject              = env->CallObjectMethod(decoderObject, onMalformedInput, replaceAction);
    }
    {
        jmethodID onUnmappableCharacter =
            env->GetMethodID(charSetDocoderClass, "onUnmappableCharacter",
                             "(Ljava/nio/charset/CodingErrorAction;)Ljava/nio/charset/CharsetDecoder;");
        decoderObject = env->CallObjectMethod(decoderObject, onUnmappableCharacter, replaceAction);
    }

    jmethodID charSetDecodeMethod = env->GetMethodID(charSetDocoderClass, "decode", "(Ljava/nio/ByteBuffer;)Ljava/nio/CharBuffer;");
    jobject decodeObject          = env->CallObjectMethod(decoderObject, charSetDecodeMethod, jbyteBuffer);
    env->DeleteLocalRef(jbyteBuffer);

    // If decode exception occur, outStr will be set null.
    outStr = nullptr;

    if (env->ExceptionCheck())
    {
        // If there is an exception, decode will not fail. Instead just
        // an error will be reported.
        ChipLogError(Support, "Exception encountered trying to decode a UTF string.");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_JNI_ERROR_EXCEPTION_THROWN;
    }

    jclass charBufferClass       = env->FindClass("java/nio/CharBuffer");
    jmethodID charBufferToString = env->GetMethodID(charBufferClass, "toString", "()Ljava/lang/String;");
    outStr                       = static_cast<jstring>(env->CallObjectMethod(decodeObject, charBufferToString));

    return CHIP_NO_ERROR;
}

} // namespace chip
