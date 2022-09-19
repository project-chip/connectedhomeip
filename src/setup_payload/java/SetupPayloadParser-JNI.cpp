#include "lib/core/CHIPError.h"
#include "lib/support/JniTypeWrappers.h"
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/ManualSetupPayloadParser.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>
#include <lib/support/logging/CHIPLogging.h>

#include <vector>

#include <jni.h>

using namespace chip;

#define SETUP_PAYLOAD_PARSER_JNI_ERROR_MIN 10 // avoiding collision with CHIPJNIError.h
#define _SETUP_PAYLOAD_PARSER_JNI_ERROR(e) CHIP_APPLICATION_ERROR(SETUP_PAYLOAD_PARSER_JNI_ERROR_MIN + (e))

#define SETUP_PAYLOAD_PARSER_JNI_ERROR_EXCEPTION_THROWN _SETUP_PAYLOAD_PARSER_JNI_ERROR(0)
#define SETUP_PAYLOAD_PARSER_JNI_ERROR_TYPE_NOT_FOUND _SETUP_PAYLOAD_PARSER_JNI_ERROR(1)
#define SETUP_PAYLOAD_PARSER_JNI_ERROR_METHOD_NOT_FOUND _SETUP_PAYLOAD_PARSER_JNI_ERROR(2)
#define SETUP_PAYLOAD_PARSER_JNI_ERROR_FIELD_NOT_FOUND _SETUP_PAYLOAD_PARSER_JNI_ERROR(3)

#define JNI_METHOD(RETURN, METHOD_NAME) extern "C" JNIEXPORT RETURN JNICALL Java_chip_setuppayload_SetupPayloadParser_##METHOD_NAME

static jobject TransformSetupPayload(JNIEnv * env, SetupPayload & payload);
static jobject CreateCapabilitiesHashSet(JNIEnv * env, RendezvousInformationFlags flags);
static void TransformSetupPayloadFromJobject(JNIEnv * env, jobject jPayload, SetupPayload & payload);
static void CreateCapabilitiesFromHashSet(JNIEnv * env, jobject discoveryCapabilitiesObj, RendezvousInformationFlags & flags);
static CHIP_ERROR ThrowUnrecognizedQRCodeException(JNIEnv * env, jstring qrCodeObj);
static CHIP_ERROR ThrowInvalidEntryCodeFormatException(JNIEnv * env, jstring entryCodeObj);

jint JNI_OnLoad(JavaVM * jvm, void * reserved)
{
    ChipLogProgress(SetupPayload, "JNI_OnLoad() called");
    chip::Platform::MemoryInit();
    return JNI_VERSION_1_6;
}

JNI_METHOD(jobject, fetchPayloadFromQrCode)(JNIEnv * env, jobject self, jstring qrCodeObj, jboolean allowInvalidPayload)
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    const char * qrString = NULL;
    SetupPayload payload;

    qrString = env->GetStringUTFChars(qrCodeObj, 0);

    err = QRCodeSetupPayloadParser(qrString).populatePayload(payload);
    env->ReleaseStringUTFChars(qrCodeObj, qrString);

    if (allowInvalidPayload == JNI_FALSE && !payload.isValidQRCodePayload())
    {
        jclass exceptionCls = env->FindClass("chip/setuppayload/SetupPayloadParser$SetupPayloadException");
        JniReferences::GetInstance().ThrowError(env, exceptionCls, CHIP_ERROR_INVALID_ARGUMENT);
        return nullptr;
    }

    if (err != CHIP_NO_ERROR)
    {
        err = ThrowUnrecognizedQRCodeException(env, qrCodeObj);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SetupPayload, "Error throwing UnrecognizedQRCodeException: %" CHIP_ERROR_FORMAT, err.Format());
        }
        return nullptr;
    }

    return TransformSetupPayload(env, payload);
}

JNI_METHOD(jobject, fetchPayloadFromManualEntryCode)(JNIEnv * env, jobject self, jstring entryCode, jboolean allowInvalidPayload)
{
    CHIP_ERROR err               = CHIP_NO_ERROR;
    const char * entryCodeString = NULL;
    SetupPayload payload;

    entryCodeString = env->GetStringUTFChars(entryCode, 0);

    err = ManualSetupPayloadParser(entryCodeString).populatePayload(payload);
    env->ReleaseStringUTFChars(entryCode, entryCodeString);

    if (allowInvalidPayload == JNI_FALSE && !payload.isValidManualCode())
    {
        jclass exceptionCls = env->FindClass("chip/setuppayload/SetupPayloadParser$SetupPayloadException");
        JniReferences::GetInstance().ThrowError(env, exceptionCls, CHIP_ERROR_INVALID_ARGUMENT);
        return nullptr;
    }

    if (err != CHIP_NO_ERROR)
    {
        err = ThrowInvalidEntryCodeFormatException(env, entryCode);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SetupPayload, "Error throwing InvalidEntryCodeFormatException: %" CHIP_ERROR_FORMAT, err.Format());
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
    // TODO: The API we have here does not handle short discriminators in any
    // sane way.  Just do what we used to do, which is pretend that a short
    // discriminator is actually a long discriminator with the low bits all 0.
    uint16_t discriminatorValue;
    if (payload.discriminator.IsShortDiscriminator())
    {
        discriminatorValue = static_cast<uint16_t>(payload.discriminator.GetShortValue())
            << (SetupDiscriminator::kLongBits - SetupDiscriminator::kShortBits);
    }
    else
    {
        discriminatorValue = payload.discriminator.GetLongValue();
    }
    env->SetIntField(setupPayload, discriminator, discriminatorValue);
    env->SetLongField(setupPayload, setUpPinCode, payload.setUpPINCode);

    env->SetObjectField(setupPayload, discoveryCapabilities,
                        CreateCapabilitiesHashSet(env, payload.rendezvousInformation.ValueOr(RendezvousInformationFlag::kNone)));

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

        switch (info.type)
        {
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
        case optionalQRCodeInfoTypeUnknown:
        default: // Optional Type variable has to set any value.
            enumType =
                env->GetStaticFieldID(enumClass, "TYPE_UNKNOWN", "Lchip/setuppayload/OptionalQRCodeInfo$OptionalQRCodeInfoType;");
            break;
        }

        if (enumType != nullptr)
        {
            jobject enumObj = env->GetStaticObjectField(enumClass, enumType);
            env->SetObjectField(optionalInfo, type, enumObj);
        }

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

JNI_METHOD(jstring, getQrCodeFromPayload)(JNIEnv * env, jobject self, jobject setupPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SetupPayload payload;
    std::string qrString;

    TransformSetupPayloadFromJobject(env, setupPayload, payload);

    err = QRCodeSetupPayloadGenerator(payload).payloadBase38Representation(qrString);
    if (err != CHIP_NO_ERROR)
    {
        jclass exceptionCls = env->FindClass("chip/setuppayload/SetupPayloadParser$SetupPayloadException");
        JniReferences::GetInstance().ThrowError(env, exceptionCls, err);
        return nullptr;
    }

    return env->NewStringUTF(qrString.c_str());
}

JNI_METHOD(jstring, getManualEntryCodeFromPayload)(JNIEnv * env, jobject self, jobject setupPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    SetupPayload payload;
    std::string outDecimalString;

    TransformSetupPayloadFromJobject(env, setupPayload, payload);

    err = ManualSetupPayloadGenerator(payload).payloadDecimalStringRepresentation(outDecimalString);
    if (err != CHIP_NO_ERROR)
    {
        jclass exceptionCls = env->FindClass("chip/setuppayload/SetupPayloadParser$SetupPayloadException");
        JniReferences::GetInstance().ThrowError(env, exceptionCls, err);
        return nullptr;
    }

    return env->NewStringUTF(outDecimalString.c_str());
}

void TransformSetupPayloadFromJobject(JNIEnv * env, jobject jPayload, SetupPayload & payload)
{
    jclass setupPayloadClass = env->FindClass("chip/setuppayload/SetupPayload");

    jfieldID version               = env->GetFieldID(setupPayloadClass, "version", "I");
    jfieldID vendorId              = env->GetFieldID(setupPayloadClass, "vendorId", "I");
    jfieldID productId             = env->GetFieldID(setupPayloadClass, "productId", "I");
    jfieldID commissioningFlow     = env->GetFieldID(setupPayloadClass, "commissioningFlow", "I");
    jfieldID discriminator         = env->GetFieldID(setupPayloadClass, "discriminator", "I");
    jfieldID setUpPinCode          = env->GetFieldID(setupPayloadClass, "setupPinCode", "J");
    jfieldID discoveryCapabilities = env->GetFieldID(setupPayloadClass, "discoveryCapabilities", "Ljava/util/Set;");

    payload.version           = env->GetIntField(jPayload, version);
    payload.vendorID          = env->GetIntField(jPayload, vendorId);
    payload.productID         = env->GetIntField(jPayload, productId);
    payload.commissioningFlow = static_cast<CommissioningFlow>(env->GetIntField(jPayload, commissioningFlow));
    payload.discriminator.SetLongValue(env->GetIntField(jPayload, discriminator));
    payload.setUpPINCode = env->GetLongField(jPayload, setUpPinCode);

    jobject discoveryCapabilitiesObj = env->GetObjectField(jPayload, discoveryCapabilities);
    CreateCapabilitiesFromHashSet(env, discoveryCapabilitiesObj,
                                  payload.rendezvousInformation.Emplace(RendezvousInformationFlag::kNone));
}

void CreateCapabilitiesFromHashSet(JNIEnv * env, jobject discoveryCapabilitiesObj, RendezvousInformationFlags & flags)
{
    jclass hashSetClass             = env->FindClass("java/util/HashSet");
    jmethodID hashSetContainsMethod = env->GetMethodID(hashSetClass, "contains", "(Ljava/lang/Object;)Z");

    jboolean contains;
    jclass capabilityEnum = env->FindClass("chip/setuppayload/DiscoveryCapability");

    jfieldID bleCapability = env->GetStaticFieldID(capabilityEnum, "BLE", "Lchip/setuppayload/DiscoveryCapability;");
    jobject bleObj         = env->GetStaticObjectField(capabilityEnum, bleCapability);
    contains               = env->CallBooleanMethod(discoveryCapabilitiesObj, hashSetContainsMethod, bleObj);
    if (contains)
    {
        flags.Set(chip::RendezvousInformationFlag::kBLE);
    }

    jfieldID softApCapability = env->GetStaticFieldID(capabilityEnum, "SOFT_AP", "Lchip/setuppayload/DiscoveryCapability;");
    jobject softApObj         = env->GetStaticObjectField(capabilityEnum, softApCapability);
    contains                  = env->CallBooleanMethod(discoveryCapabilitiesObj, hashSetContainsMethod, softApObj);
    if (contains)
    {
        flags.Set(chip::RendezvousInformationFlag::kSoftAP);
    }

    jfieldID onNetworkCapability = env->GetStaticFieldID(capabilityEnum, "ON_NETWORK", "Lchip/setuppayload/DiscoveryCapability;");
    jobject onNetworkObj         = env->GetStaticObjectField(capabilityEnum, onNetworkCapability);
    contains                     = env->CallBooleanMethod(discoveryCapabilitiesObj, hashSetContainsMethod, onNetworkObj);
    if (contains)
    {
        flags.Set(chip::RendezvousInformationFlag::kOnNetwork);
    }
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
