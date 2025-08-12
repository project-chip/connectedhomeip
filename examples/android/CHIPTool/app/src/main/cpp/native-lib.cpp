/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

// TODO: Build this out with native code to perform tasks with CHIP stack
// QRCode parsing
// Echo request/response processing
// On/Off request/response processing

#include <Base38.h>
#include <SetupPayload.h>
#include <jni.h>
#include <lib/support/CodeUtils.h>

#include <string>

// This is just a sample function to demonstrate end-to-end calling in/out of CHIP stack
// is working.
std::string base38Encode(void)
{
    const uint8_t buf[] = { 0, 1, 2, 3, 4, 5 };
    size_t size         = MATTER_ARRAY_SIZE(buf);
    return chip::base38Encode(&buf[0], size);
}

extern "C" JNIEXPORT jstring JNICALL Java_com_google_chip_chiptool_CHIPNativeBridge_base38Encode(JNIEnv * env, jobject thiz)
{
    std::string s = base38Encode();
    return env->NewStringUTF(s.c_str());
}
