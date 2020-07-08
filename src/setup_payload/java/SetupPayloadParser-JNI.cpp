#include "ManualSetupPayloadParser.h"
#include "QRCodeSetupPayloadParser.h"

#include <jni.h>

using namespace chip;

#define JNI_METHOD(RETURN, METHOD_NAME) extern "C" JNIEXPORT RETURN JNICALL Java_chip_setuppayload_SetupPayloadParser_##METHOD_NAME

static jobject TransformSetupPayload(JNIEnv * env, SetupPayload & payload);

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

jobject TransformSetupPayload(JNIEnv * env, SetupPayload & payload)
{
    jclass setupPayloadClass = env->FindClass("chip/setuppayload/SetupPayload");
    jmethodID setupConstr    = env->GetMethodID(setupPayloadClass, "<init>", "()V");
    jobject setupPayload     = env->NewObject(setupPayloadClass, setupConstr);

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

    jmethodID addOptionalInfoMid =
        env->GetMethodID(setupPayloadClass, "addOptionalQRCodeInfo", "(Lchip/setuppayload/OptionalQRCodeInfo;)V");

    vector<OptionalQRCodeInfo> optional_info = payload.getAllOptionalVendorData();
    for (OptionalQRCodeInfo & info : optional_info)
    {

        jclass optionalInfoClass = env->FindClass("chip/setuppayload/OptionalQRCodeInfo");
        jobject optionalInfo     = env->AllocObject(optionalInfoClass);
        jfieldID tag             = env->GetFieldID(optionalInfoClass, "tag", "I");
        jfieldID type = env->GetFieldID(optionalInfoClass, "type", "Lchip/setuppayload/OptionalQRCodeInfo$OptionalQRCodeInfoType;");
        jfieldID data = env->GetFieldID(optionalInfoClass, "data", "Ljava/lang/String;");
        jfieldID int32 = env->GetFieldID(optionalInfoClass, "int32", "I");

        env->SetIntField(optionalInfo, tag, info.tag);

        jclass enumClass  = env->FindClass("chip/setuppayload/OptionalQRCodeInfo$OptionalQRCodeInfoType");
        jfieldID enumType = nullptr;

        switch (info.tag)
        {
        case optionalQRCodeInfoTypeUnknown:
            enumType =
                env->GetStaticFieldID(enumClass, "TYPE_UNKNOWN", "Lchip/setuppayload/OptionalQRCodeInfo$OptionalQRCodeInfoType;");
            break;
        case optionalQRCodeInfoTypeString:
            enumType =
                env->GetStaticFieldID(enumClass, "TYPE_STRING", "Lchip/setuppayload/OptionalQRCodeInfo$OptionalQRCodeInfoType;");
            break;
        case optionalQRCodeInfoTypeInt32:
            enumType =
                env->GetStaticFieldID(enumClass, "TYPE_INT32", "Lchip/setuppayload/OptionalQRCodeInfo$OptionalQRCodeInfoType;");
            break;
        case optionalQRCodeInfoTypeInt64:
            enumType =
                env->GetStaticFieldID(enumClass, "TYPE_INT64", "Lchip/setuppayload/OptionalQRCodeInfo$OptionalQRCodeInfoType;");
            break;
        case optionalQRCodeInfoTypeUInt32:
            enumType =
                env->GetStaticFieldID(enumClass, "TYPE_UINT32", "Lchip/setuppayload/OptionalQRCodeInfo$OptionalQRCodeInfoType;");
            break;
        case optionalQRCodeInfoTypeUInt64:
            enumType =
                env->GetStaticFieldID(enumClass, "TYPE_UINT64", "Lchip/setuppayload/OptionalQRCodeInfo$OptionalQRCodeInfoType;");
            break;
        default:
            break;
        }

        jobject enumObj = env->GetStaticObjectField(enumClass, enumType);
        env->SetObjectField(optionalInfo, type, enumObj);

        env->SetObjectField(optionalInfo, data, env->NewStringUTF(info.data.c_str()));
        env->SetIntField(optionalInfo, int32, info.int32);

        env->CallVoidMethod(setupPayload, addOptionalInfoMid, optionalInfo);
    }

    return setupPayload;
}
