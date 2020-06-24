// TODO: Build this out with native code to perform tasks with CHIP stack
// QRCode parsing
// Echo request/response processing
// On/Off request/response processing

#include <Base41.h>
#include <SetupPayload.h>
#include <jni.h>

// This is just a sample function to demonstrate end-to-end calling in/out of CHIP stack
// is working.
std::string base41Encode(void)
{
    const uint8_t buf[6] = { 0, 1, 2, 3, 4, 5 };
    size_t size          = 6;
    return chip::base41Encode(&buf[0], size);
}

extern "C" JNIEXPORT jstring JNICALL Java_com_google_chip_chiptool_CHIPNativeBridge_base41Encode(JNIEnv * env, jobject thiz)
{
    std::string s = base41Encode();
    return env->NewStringUTF(s.c_str());
}
