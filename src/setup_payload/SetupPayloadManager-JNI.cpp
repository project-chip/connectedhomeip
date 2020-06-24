//
// Created by Vidhi Shah on 6/17/20.
//
#include "QRCodeSetupPayloadParser.h"
#include "Base41.h"

#include <jni.h>

#include <string.h>

using namespace chip;

#define JNI_METHOD(RETURN, METHOD_NAME) \
    extern "C" JNIEXPORT RETURN JNICALL Java_com_package_##METHOD_NAME

JNI_METHOD(jlong, newQrCodeSetupPayloadParser)(JNIEnv *env, jobject self, jstring qrCodeObj)
{
    const char *qrString = NULL;
    QRCodeSetupPayloadParser *qrPayloadParser = NULL;

    qrString = env->GetStringUTFChars(qrCodeObj, 0);
    qrPayloadParser = new QRCodeSetupPayloadParser(qrString);
    return (long)qrPayloadParser;
}

JNI_METHOD(jobject, populatePayload)(JNIEnv *env, jobject self, jlong qrPayloadParserPtr)
{
    CHIP_ERROR err;
    QRCodeSetupPayloadParser *qrPayloadParser = (QRCodeSetupPayloadParser *)qrPayloadParserPtr;
    SetupPayload payload;

    err = qrPayloadParser->populatePayload(payload);

    //if (err != CHIP_NO_ERROR)
      //  env->Throw(err)

    jclass setupPayloadClass = env->FindClass("com/chip/setuppayload/SetupPayload");
    jobject setupPayload = env->AllocObject(setupPayloadClass);

    jfieldID version = env->GetFieldID(setupPayloadClass, "version", "I");
    jfieldID vendorId = env->GetFieldID(setupPayloadClass, "vendorId", "I");
    jfieldID productId = env->GetFieldID(setupPayloadClass, "productId", "I");
    jfieldID requiresCustomFlow = env->GetFieldID(setupPayloadClass, "requiresCustomFlow", "Z");
    jfieldID rendezvousInfo = env->GetFieldID(setupPayloadClass, "rendezvousInformation", "I");
    jfieldID discriminator = env->GetFieldID(setupPayloadClass, "discriminator", "I");
    jfieldID setUpPinCode = env->GetFieldID(setupPayloadClass, "setUpPinCode", "J");
    jfieldID serialNumber = env->GetFieldID(setupPayloadClass, "serialNumber", "Ljava/lang/String;");

    env->SetIntField(setupPayload, version, payload.version);
    env->SetIntField(setupPayload, vendorId, payload.vendorID);
    env->SetIntField(setupPayload, productId, payload.productID);
    env->SetBooleanField(setupPayload, requiresCustomFlow, payload.requiresCustomFlow);
    env->SetIntField(setupPayload, rendezvousInfo, payload.rendezvousInformation);
    env->SetIntField(setupPayload, discriminator, payload.discriminator);
    env->SetLongField(setupPayload, setUpPinCode, payload.setUpPINCode);
    env->SetObjectField(setupPayload, serialNumber, env->NewStringUTF(payload.serialNumber.c_str()));

    return setupPayload;
}
