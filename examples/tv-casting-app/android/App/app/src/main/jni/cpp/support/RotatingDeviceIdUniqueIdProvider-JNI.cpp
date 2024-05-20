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

#include "RotatingDeviceIdUniqueIdProvider-JNI.h"
#include "lib/support/logging/CHIPLogging.h"
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/Span.h>

using namespace chip;

namespace matter {
namespace casting {
namespace support {

CHIP_ERROR RotatingDeviceIdUniqueIdProviderJNI::Initialize(jobject provider)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnValue(env != nullptr, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(AppServer, "Failed to GetEnvForCurrentThread for RotatingDeviceIdUniqueIdProviderJNI"));

    ReturnLogErrorOnFailure(mJNIProviderObject.Init(provider));

    jclass JNIProviderClass = env->GetObjectClass(provider);
    VerifyOrReturnValue(JNIProviderClass != nullptr, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(AppServer, "Failed to get JNIProvider Java class"));

    mGetMethod = env->GetMethodID(JNIProviderClass, "_get", "()Ljava/lang/Object;");
    if (mGetMethod == nullptr)
    {
        ChipLogError(AppServer, "Failed to access JNIProvider '_get' method");
        env->ExceptionClear();
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR RotatingDeviceIdUniqueIdProviderJNI::GetJavaByteByMethod(jmethodID method, MutableByteSpan & out_buffer)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnLogError(mJNIProviderObject.HasValidObjectRef(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnLogError(method != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnLogError(env != nullptr, CHIP_JNI_ERROR_NO_ENV);

    jbyteArray outArray = (jbyteArray) env->CallObjectMethod(mJNIProviderObject.ObjectRef(), method);
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in get Method");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if (outArray == nullptr || env->GetArrayLength(outArray) <= 0)
    {
        out_buffer.reduce_size(0);
        return CHIP_NO_ERROR;
    }

    JniByteArray JniOutArray(env, outArray);
    return CopySpanToMutableSpan(JniOutArray.byteSpan(), out_buffer);
}

MutableByteSpan * RotatingDeviceIdUniqueIdProviderJNI::Get()
{
    ChipLogProgress(AppServer, "RotatingDeviceIdUniqueIdProviderJNI.Get() called");
    mRotatingDeviceIdUniqueIdSpan = MutableByteSpan(mRotatingDeviceIdUniqueId);
    CHIP_ERROR err                = GetJavaByteByMethod(mGetMethod, mRotatingDeviceIdUniqueIdSpan);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                        ChipLogError(AppServer, "Error calling GetJavaByteByMethod %" CHIP_ERROR_FORMAT, err.Format()));
    return &mRotatingDeviceIdUniqueIdSpan;
}

}; // namespace support
}; // namespace casting
}; // namespace matter
