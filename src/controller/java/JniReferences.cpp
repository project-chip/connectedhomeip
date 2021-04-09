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

#include "JniReferences.h"

#include "CHIPJNIError.h"

#include <support/CodeUtils.h>

CHIP_ERROR GetClassRef(JNIEnv * env, const char * clsType, jclass & outCls)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    jclass cls     = NULL;

    cls = env->FindClass(clsType);
    VerifyOrExit(cls != NULL, err = CHIP_JNI_ERROR_TYPE_NOT_FOUND);

    outCls = (jclass) env->NewGlobalRef((jobject) cls);
    VerifyOrExit(outCls != NULL, err = CHIP_JNI_ERROR_TYPE_NOT_FOUND);

exit:
    env->DeleteLocalRef(cls);
    return err;
}
