/*
 *
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

#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <lib/support/TypeTraits.h>
#include <string>

namespace chip {
class JniReferences
{
public:
    // No copy, move or assignment.
    JniReferences(const JniReferences &)  = delete;
    JniReferences(const JniReferences &&) = delete;
    JniReferences & operator=(const JniReferences &) = delete;

    static JniReferences & GetInstance()
    {
        static JniReferences jniReferences;
        return jniReferences;
    }

    /**
     * Set the JavaVM.
     *
     * we need clsType in context to get ClassLoader
     *
     * This must be called before GetEnvForCurrentThread() or GetClassRef().
     */
    void SetJavaVm(JavaVM * jvm, const char * clsType);

    /**
     * Returns a JNIEnv for the current thread.
     *
     * This must be called after SetJavaVm(). If the current thread is not attached to the JVM, this method will attach the thread
     * first, then retrieve the JNIEnv.
     */
    JNIEnv * GetEnvForCurrentThread();

    /**
     * @brief
     *   Creates a jclass reference to the given class type.
     *
     *   This must be called after SetJavaVm().
     *
     * @param[in] env The JNIEnv for finding a Java class and creating a new Java reference.
     * @param[in] clsType The fully-qualified Java class name to find, e.g. java/lang/IllegalStateException.
     * @param[out] outCls A Java reference to the class matching clsType.
     */
    CHIP_ERROR GetClassRef(JNIEnv * env, const char * clsType, jclass & outCls);
    CHIP_ERROR FindMethod(JNIEnv * env, jobject object, const char * methodName, const char * methodSignature,
                          jmethodID * methodId);
    void CallVoidInt(JNIEnv * env, jobject object, const char * methodName, jint argument);

    CHIP_ERROR N2J_ByteArray(JNIEnv * env, const uint8_t * inArray, jsize inArrayLen, jbyteArray & outArray);

    void ReportError(JNIEnv * env, CHIP_ERROR cbErr, const char * functName);

    void ThrowError(JNIEnv * env, jclass exceptionCls, CHIP_ERROR errToThrow);

    /**
     * Creates a java.util.Optional wrapping the specified jobject. If the wrapped jobject is null, an empty
     * Optional will be returned.
     */
    CHIP_ERROR CreateOptional(jobject objectToWrap, jobject & outOptional);

    /**
     * Retrieve the value of a java.util.Optional, or nullptr if the Optional is empty.
     */
    CHIP_ERROR GetOptionalValue(jobject optionalObj, jobject & optionalValue);

    /**
     * Get a primitive jint from the Java boxed type Integer, using intValue().
     */
    jint IntegerToPrimitive(jobject boxedObject);

    /**
     * Get a primitive jlong from the Java boxed type Long, using longValue().
     */
    jlong LongToPrimitive(jobject boxedObject);

    /**
     * Get a primitive jboolean from the Java boxed type Booelan, using booleanValue().
     */
    jboolean BooleanToPrimitive(jobject boxedObject);

    /**
     * Get a primitive jfloat from the Java boxed type Float, using floatValue().
     */
    jfloat FloatToPrimitive(jobject boxedObject);

    /**
     * Get a primitive jfloat from the Java boxed type Double, using doubleValue().
     */
    jdouble DoubleToPrimitive(jobject boxedObject);

    CHIP_ERROR CreateArrayList(jobject & outList);

    CHIP_ERROR AddToList(jobject list, jobject objectToAdd);

    CHIP_ERROR GetListSize(jobject list, jint & size);

    CHIP_ERROR GetListItem(jobject list, jint index, jobject & outItem);

    CHIP_ERROR CreateHashMap(jobject & outMap);

    CHIP_ERROR PutInMap(jobject map, jobject key, jobject value);

    CHIP_ERROR GetObjectField(jobject objectToRead, const char * name, const char * signature, jobject & outObject);

    /**
     * Call a void method with subscriptionId named "OnSubscriptionEstablished" on the provided jobject.
     */
    CHIP_ERROR CallSubscriptionEstablished(jobject javaCallback, long subscriptionId);

    /**
     * Creates a boxed type (e.g. java.lang.Integer) based on the the class name ("java/lang/Integer"), constructor JNI signature
     * ("(I)V"), and value.
     */
    template <class T, typename std::enable_if_t<!std::is_enum<T>::value, int> = 0>
    CHIP_ERROR CreateBoxedObject(std::string boxedTypeClsName, std::string constructorSignature, T value, jobject & outObj)
    {
        JNIEnv * env   = GetEnvForCurrentThread();
        CHIP_ERROR err = CHIP_NO_ERROR;
        jclass boxedTypeCls;
        err = GetClassRef(env, boxedTypeClsName.c_str(), boxedTypeCls);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err);

        jmethodID boxedTypeConstructor = env->GetMethodID(boxedTypeCls, "<init>", constructorSignature.c_str());
        outObj                         = env->NewObject(boxedTypeCls, boxedTypeConstructor, value);
        env->DeleteGlobalRef(boxedTypeCls);

        return err;
    }

    /**
     * Handling for strongly-typed enums.
     */
    template <class T, typename std::enable_if_t<std::is_enum<T>::value, int> = 0>
    CHIP_ERROR CreateBoxedObject(std::string boxedTypeClsName, std::string constructorSignature, T value, jobject & outObj)
    {
        return CreateBoxedObject(boxedTypeClsName, constructorSignature, chip::to_underlying(value), outObj);
    }

    /**
     * Use instead of 'NewStringUTF' function
     * If the value is not decoded with "UTF-8", the error will be returned.
     * (The NewStringUTF function crashes when the value can not decoded as "UTF-8".)
     *
     * Creates a java string type based on char array.
     */
    CHIP_ERROR CharToStringUTF(const chip::CharSpan & charSpan, jobject & outString);

private:
    JniReferences() {}

    JavaVM * mJvm              = nullptr;
    jobject mClassLoader       = nullptr;
    jmethodID mFindClassMethod = nullptr;

    jclass mHashMapClass   = nullptr;
    jclass mListClass      = nullptr;
    jclass mArrayListClass = nullptr;
};
} // namespace chip
