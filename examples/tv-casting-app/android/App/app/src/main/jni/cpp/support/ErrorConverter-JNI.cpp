/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "ErrorConverter-JNI.h"
#include <lib/support/JniReferences.h>

namespace matter {
namespace casting {
namespace support {

using namespace chip;

jobject createJMatterError(CHIP_ERROR inErr)
{
    JNIEnv * env = chip::JniReferences::GetInstance().GetEnvForCurrentThread();
    jclass jMatterErrorClass;
    CHIP_ERROR err =
        chip::JniReferences::GetInstance().GetClassRef(env, "com/matter/casting/support/MatterError", jMatterErrorClass);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr);

    jmethodID jMatterErrorConstructor = env->GetMethodID(jMatterErrorClass, "<init>", "(JLjava/lang/String;)V");

    return env->NewObject(jMatterErrorClass, jMatterErrorConstructor, err.AsInteger(), nullptr);
}

}; // namespace support
}; // namespace casting
}; // namespace matter
