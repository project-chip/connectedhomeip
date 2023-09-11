/*
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

// This is just a sample function to demonstrate end-to-end calling in/out of CHIP stack
// is working.
std::string base38Encode(void)
{
    const uint8_t buf[] = { 0, 1, 2, 3, 4, 5 };
    size_t size         = ArraySize(buf);
    return chip::base38Encode(&buf[0], size);
}

extern "C" JNIEXPORT jstring JNICALL Java_com_google_chip_chiptool_CHIPNativeBridge_base38Encode(JNIEnv * env, jobject thiz)
{
    std::string s = base38Encode();
    return env->NewStringUTF(s.c_str());
}
