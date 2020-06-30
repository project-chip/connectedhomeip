#include "ManualSetupPayloadParser.h"
#include "QRCodeSetupPayloadParser.h"

#include <jni.h>

using namespace chip;

#define JNI_METHOD(RETURN, METHOD_NAME) extern "C" JNIEXPORT RETURN JNICALL Java_chip_setuppayload_SetupPayloadParser_##METHOD_NAME

static jobject TransformSetupPayload(JNIEnv * env, SetupPayload payload);

JNI_METHOD(jobject, fetchPayloadFromQrCode)(JNIEnv * env, jobject self, jstring qrCodeObj)
{
    CHIP_ERROR err;
    const char * qrString                      = NULL;
    QRCodeSetupPayloadParser * qrPayloadParser = NULL;
    SetupPayload payload;

    qrString        = env->GetStringUTFChars(qrCodeObj, 0);
    qrPayloadParser = new QRCodeSetupPayloadParser(qrString);

    err = qrPayloadParser->populatePayload(payload);

    return TransformSetupPayload(env, payload);
}

JNI_METHOD(jobject, fetchPayloadFromManualEntryCode)(JNIEnv * env, jobject self, jstring entryCode)
{
    CHIP_ERROR err;
    const char * entryCodeString                        = NULL;
    ManualSetupPayloadParser * manualSetupPayloadParser = NULL;
    SetupPayload payload;

    entryCodeString          = env->GetStringUTFChars(entryCode, 0);
    manualSetupPayloadParser = new ManualSetupPayloadParser(entryCodeString);

    err = manualSetupPayloadParser->populatePayload(payload);

    return TransformSetupPayload(env, payload);
}

jobject TransformSetupPayload(JNIEnv * env, SetupPayload payload)
{
    jclass setupPayloadClass = env->FindClass("chip/setuppayload/SetupPayload");
    jobject setupPayload     = env->AllocObject(setupPayloadClass);

    jfieldID version            = env->GetFieldID(setupPayloadClass, "version", "I");
    jfieldID vendorId           = env->GetFieldID(setupPayloadClass, "vendorId", "I");
    jfieldID productId          = env->GetFieldID(setupPayloadClass, "productId", "I");
    jfieldID requiresCustomFlow = env->GetFieldID(setupPayloadClass, "requiresCustomFlow", "Z");
    jfieldID discriminator      = env->GetFieldID(setupPayloadClass, "discriminator", "I");
    jfieldID setUpPinCode       = env->GetFieldID(setupPayloadClass, "setupPinCode", "J");

    env->SetIntField(setupPayload, version, payload.version);
    env->SetIntField(setupPayload, vendorId, payload.vendorID);
    env->SetIntField(setupPayload, productId, payload.productID);
    env->SetBooleanField(setupPayload, requiresCustomFlow, payload.requiresCustomFlow);
    env->SetIntField(setupPayload, discriminator, payload.discriminator);
    env->SetLongField(setupPayload, setUpPinCode, payload.setUpPINCode);

    return setupPayload;
}
