#include <setup_payload/ManualSetupPayloadParser.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>

#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include <vector>

#include <jni.h>

using namespace chip;

#define SETUP_PAYLOAD_PARSER_JNI_ERROR_MIN 10000
#define SETUP_PAYLOAD_PARSER_JNI_ERROR_MAX 10999

#define _SETUP_PAYLOAD_PARSER_JNI_ERROR(e) (SETUP_PAYLOAD_PARSER_JNI_ERROR_MIN + (e))

#define SETUP_PAYLOAD_PARSER_JNI_ERROR_EXCEPTION_THROWN _SETUP_PAYLOAD_PARSER_JNI_ERROR(0)
#define SETUP_PAYLOAD_PARSER_JNI_ERROR_TYPE_NOT_FOUND _SETUP_PAYLOAD_PARSER_JNI_ERROR(1)
#define SETUP_PAYLOAD_PARSER_JNI_ERROR_METHOD_NOT_FOUND _SETUP_PAYLOAD_PARSER_JNI_ERROR(2)
#define SETUP_PAYLOAD_PARSER_JNI_ERROR_FIELD_NOT_FOUND _SETUP_PAYLOAD_PARSER_JNI_ERROR(3)

#define JNI_METHOD(RETURN, METHOD_NAME) extern "C" JNIEXPORT RETURN JNICALL Java_chip_setuppayload_SetupPayloadParser_##METHOD_NAME

static jobject TransformSetupPayload(JNIEnv * env, SetupPayload & payload);
static jobject CreateCapabilitiesHashSet(JNIEnv * env, RendezvousInformationFlags flags);
static CHIP_ERROR ThrowUnrecognizedQRCodeException(JNIEnv * env, jstring qrCodeObj);
static CHIP_ERROR ThrowInvalidEntryCodeFormatException(JNIEnv * env, jstring entryCodeObj);

jint JNI_OnLoad(JavaVM * jvm, void * reserved)
{
    ChipLogProgress(SetupPayload, "JNI_OnLoad() called");
    chip::Platform::MemoryInit();
    return JNI_VERSION_1_6;
}

JNI_METHOD(jobject, fetchPayloadFromQrCode)(JNIEnv * env, jobject self, jstring qrCodeObj)
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    const char * qrString = NULL;
    SetupPayload payload;

    qrString = env->GetStringUTFChars(qrCodeObj, 0);

    err = QRCodeSetupPayloadParser(qrString).populatePayload(payload);
    env->ReleaseStringUTFChars(qrCodeObj, qrString);

    if (err != CHIP_NO_ERROR)
    {
        err = ThrowUnrecognizedQRCodeException(env, qrCodeObj);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SetupPayload, "Error throwing UnrecognizedQRCodeException: %d", err);
        }
        return nullptr;
    }

    return TransformSetupPayload(env, payload);
}

JNI_METHOD(jobject, fetchPayloadFromManualEntryCode)(JNIEnv * env, jobject self, jstring entryCode)
{
    CHIP_ERROR err               = CHIP_NO_ERROR;
    const char * entryCodeString = NULL;
    SetupPayload payload;

    entryCodeString = env->GetStringUTFChars(entryCode, 0);
    env->ReleaseStringUTFChars(entryCode, entryCodeString);

    err = ManualSetupPayloadParser(entryCodeString).populatePayload(payload);

    if (err != CHIP_NO_ERROR)
    {
        err = ThrowInvalidEntryCodeFormatException(env, entryCode);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SetupPayload, "Error throwing InvalidEntryCodeFormatException: %d", err);
        }
        return nullptr;
    }

    return TransformSetupPayload(env, payload);
}

jobject TransformSetupPayload(JNIEnv * env, SetupPayload & payload)
{
    jclass setupPayloadClass = env->FindClass("chip/setuppayload/SetupPayload");
    jmethodID setupConstr    = env->GetMethodID(setupPayloadClass, "<init>", "()V");
    jobject setupPayload     = env->NewObject(setupPayloadClass, setupConstr);

    jfieldID version               = env->GetFieldID(setupPayloadClass, "version", "I");
    jfieldID vendorId              = env->GetFieldID(setupPayloadClass, "vendorId", "I");
    jfieldID productId             = env->GetFieldID(setupPayloadClass, "productId", "I");
    jfieldID commissioningFlow     = env->GetFieldID(setupPayloadClass, "commissioningFlow", "I");
    jfieldID discriminator         = env->GetFieldID(setupPayloadClass, "discriminator", "I");
    jfieldID setUpPinCode          = env->GetFieldID(setupPayloadClass, "setupPinCode", "J");
    jfieldID discoveryCapabilities = env->GetFieldID(setupPayloadClass, "discoveryCapabilities", "Ljava/util/Set;");

    env->SetIntField(setupPayload, version, payload.version);
    env->SetIntField(setupPayload, vendorId, payload.vendorID);
    env->SetIntField(setupPayload, productId, payload.productID);
    env->SetIntField(setupPayload, commissioningFlow, static_cast<int>(payload.commissioningFlow));
    env->SetIntField(setupPayload, discriminator, payload.discriminator);
    env->SetLongField(setupPayload, setUpPinCode, payload.setUpPINCode);

    env->SetObjectField(setupPayload, discoveryCapabilities, CreateCapabilitiesHashSet(env, payload.rendezvousInformation));

    jmethodID addOptionalInfoMid =
        env->GetMethodID(setupPayloadClass, "addOptionalQRCodeInfo", "(Lchip/setuppayload/OptionalQRCodeInfo;)V");

    std::vector<OptionalQRCodeInfo> optional_info = payload.getAllOptionalVendorData();
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

jobject CreateCapabilitiesHashSet(JNIEnv * env, RendezvousInformationFlags flags)
{
    jclass hashSetClass          = env->FindClass("java/util/HashSet");
    jmethodID hashSetConstructor = env->GetMethodID(hashSetClass, "<init>", "()V");
    jobject capabilitiesHashSet  = env->NewObject(hashSetClass, hashSetConstructor);

    jmethodID hashSetAddMethod = env->GetMethodID(hashSetClass, "add", "(Ljava/lang/Object;)Z");
    jclass capabilityEnum      = env->FindClass("chip/setuppayload/DiscoveryCapability");

    if (flags.Has(chip::RendezvousInformationFlag::kBLE))
    {
        jfieldID bleCapability = env->GetStaticFieldID(capabilityEnum, "BLE", "Lchip/setuppayload/DiscoveryCapability;");
        jobject enumObj        = env->GetStaticObjectField(capabilityEnum, bleCapability);
        env->CallBooleanMethod(capabilitiesHashSet, hashSetAddMethod, enumObj);
    }
    if (flags.Has(chip::RendezvousInformationFlag::kSoftAP))
    {
        jfieldID softApCapability = env->GetStaticFieldID(capabilityEnum, "SOFT_AP", "Lchip/setuppayload/DiscoveryCapability;");
        jobject enumObj           = env->GetStaticObjectField(capabilityEnum, softApCapability);
        env->CallBooleanMethod(capabilitiesHashSet, hashSetAddMethod, enumObj);
    }
    if (flags.Has(chip::RendezvousInformationFlag::kOnNetwork))
    {
        jfieldID onNetworkCapability =
            env->GetStaticFieldID(capabilityEnum, "ON_NETWORK", "Lchip/setuppayload/DiscoveryCapability;");
        jobject enumObj = env->GetStaticObjectField(capabilityEnum, onNetworkCapability);
        env->CallBooleanMethod(capabilitiesHashSet, hashSetAddMethod, enumObj);
    }
    return capabilitiesHashSet;
}

CHIP_ERROR ThrowUnrecognizedQRCodeException(JNIEnv * env, jstring qrCodeObj)
{
    jclass exceptionCls            = nullptr;
    jmethodID exceptionConstructor = nullptr;
    jthrowable exception           = nullptr;

    env->ExceptionClear();

    exceptionCls = env->FindClass("chip/setuppayload/SetupPayloadParser$UnrecognizedQrCodeException");
    VerifyOrReturnError(exceptionCls != NULL, SETUP_PAYLOAD_PARSER_JNI_ERROR_TYPE_NOT_FOUND);
    exceptionConstructor = env->GetMethodID(exceptionCls, "<init>", "(Ljava/lang/String;)V");
    VerifyOrReturnError(exceptionConstructor != NULL, SETUP_PAYLOAD_PARSER_JNI_ERROR_METHOD_NOT_FOUND);
    exception = (jthrowable) env->NewObject(exceptionCls, exceptionConstructor, qrCodeObj);
    VerifyOrReturnError(exception != NULL, SETUP_PAYLOAD_PARSER_JNI_ERROR_EXCEPTION_THROWN);

    env->Throw(exception);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThrowInvalidEntryCodeFormatException(JNIEnv * env, jstring entryCodeObj)
{
    jclass exceptionCls            = nullptr;
    jmethodID exceptionConstructor = nullptr;
    jthrowable exception           = nullptr;

    env->ExceptionClear();

    exceptionCls = env->FindClass("chip/setuppayload/SetupPayloadParser$InvalidEntryCodeFormatException");
    VerifyOrReturnError(exceptionCls != NULL, SETUP_PAYLOAD_PARSER_JNI_ERROR_TYPE_NOT_FOUND);
    exceptionConstructor = env->GetMethodID(exceptionCls, "<init>", "(Ljava/lang/String;)V");
    VerifyOrReturnError(exceptionConstructor != NULL, SETUP_PAYLOAD_PARSER_JNI_ERROR_METHOD_NOT_FOUND);
    exception = (jthrowable) env->NewObject(exceptionCls, exceptionConstructor, entryCodeObj);
    VerifyOrReturnError(exception != NULL, SETUP_PAYLOAD_PARSER_JNI_ERROR_EXCEPTION_THROWN);

    env->Throw(exception);
    return CHIP_NO_ERROR;
}
