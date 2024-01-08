#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

#define JNI_METHOD(RETURN, CLASS_NAME, METHOD_NAME)                                                                                \
    extern "C" JNIEXPORT RETURN JNICALL Java_chip_devicecontroller_ChipClusters_00024##CLASS_NAME##_##METHOD_NAME

#define CHIP_TLV_WRITER_BUFFER_SIZE 1024

jobject decodeValueFromTLV(JNIEnv * env, chip::TLV::TLVReader * data);
static CHIP_ERROR encodeTLVFromValue(JNIEnv * env, jobject jObject, chip::TLV::TLVWriter & writer, chip::TLV::Tag tag);

jobject decodeValueFromTLV(JNIEnv * env, chip::TLV::TLVReader * data)
{
    chip::TLV::TLVType dataTLVType = data->GetType();
    switch (dataTLVType)
    {
    case chip::TLV::kTLVType_SignedInteger: {
        int64_t val;
        CHIP_ERROR err = data->Get(val);
        VerifyOrReturnValue(
            err == CHIP_NO_ERROR, nullptr,
            ChipLogError(Controller, "Error: TLV signed integer decoding failed : %" CHIP_ERROR_FORMAT, err.Format()));
        jclass intTypeCls     = env->FindClass("chip/devicecontroller/ChipTLVType$IntType");
        jmethodID constructor = env->GetMethodID(intTypeCls, "<init>", "(J)V");
        return env->NewObject(intTypeCls, constructor, static_cast<jlong>(val));
    }
    case chip::TLV::kTLVType_UnsignedInteger: {
        uint64_t val;
        CHIP_ERROR err = data->Get(val);
        VerifyOrReturnValue(
            err == CHIP_NO_ERROR, nullptr,
            ChipLogError(Controller, "Error: TLV unsigned integer decoding failed : %" CHIP_ERROR_FORMAT, err.Format()));
        jclass uintTypeCls    = env->FindClass("chip/devicecontroller/ChipTLVType$UIntType");
        jmethodID constructor = env->GetMethodID(uintTypeCls, "<init>", "(J)V");
        return env->NewObject(uintTypeCls, constructor, static_cast<jlong>(val));
    }
    case chip::TLV::kTLVType_Boolean: {
        bool val;
        CHIP_ERROR err = data->Get(val);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                            ChipLogError(Controller, "Error: TLV boolean decoding failed : %" CHIP_ERROR_FORMAT, err.Format()));
        jclass booleanTypeCls = env->FindClass("chip/devicecontroller/ChipTLVType$BooleanType");
        jmethodID constructor = env->GetMethodID(booleanTypeCls, "<init>", "(Z)V");
        return env->NewObject(booleanTypeCls, constructor, static_cast<jboolean>(val));
    }
    case chip::TLV::kTLVType_FloatingPointNumber: {
        // Try float first
        float floatValue;
        CHIP_ERROR err = data->Get(floatValue);
        if (err == CHIP_NO_ERROR)
        {
            jclass floatTypeCls   = env->FindClass("chip/devicecontroller/ChipTLVType$FloatType");
            jmethodID constructor = env->GetMethodID(floatTypeCls, "<init>", "(F)V");
            return env->NewObject(floatTypeCls, constructor, static_cast<jfloat>(floatValue));
        }
        double val;
        err = data->Get(val);
        VerifyOrReturnValue(
            err == CHIP_NO_ERROR, nullptr,
            ChipLogError(Controller, "Error: TLV floating point decoding failed : %" CHIP_ERROR_FORMAT, err.Format()));
        jclass doubleTypeCls        = env->FindClass("chip/devicecontroller/ChipTLVType$DoubleType");
        jmethodID doubleConstructor = env->GetMethodID(doubleTypeCls, "<init>", "(D)V");
        return env->NewObject(doubleTypeCls, doubleConstructor, static_cast<jdouble>(val));
    }
    case chip::TLV::kTLVType_UTF8String: {
        chip::CharSpan stringValue;
        CHIP_ERROR err = data->Get(stringValue);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                            ChipLogError(Controller, "Error: TLV UTF8String decoding failed : %" CHIP_ERROR_FORMAT, err.Format()));
        chip::UtfString stringObj(env, stringValue);
        jclass stringTypeCls  = env->FindClass("chip/devicecontroller/ChipTLVType$StringType");
        jmethodID constructor = env->GetMethodID(stringTypeCls, "<init>", "(Ljava/lang/String;)V");
        return env->NewObject(stringTypeCls, constructor, stringObj.jniValue());
    }
    case chip::TLV::kTLVType_ByteString: {
        chip::ByteSpan bytesValue;
        CHIP_ERROR err = data->Get(bytesValue);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                            ChipLogError(Controller, "Error: TLV ByteString decoding failed : %" CHIP_ERROR_FORMAT, err.Format()));
        chip::ByteArray byteArrayObj(env, bytesValue);
        jclass stringTypeCls  = env->FindClass("chip/devicecontroller/ChipTLVType$ByteArrayType");
        jmethodID constructor = env->GetMethodID(stringTypeCls, "<init>", "([B)V");
        return env->NewObject(stringTypeCls, constructor, byteArrayObj.jniValue());
    }
    case chip::TLV::kTLVType_Null: {
        jclass nullTypeCls    = env->FindClass("chip/devicecontroller/ChipTLVType$NullType");
        jmethodID constructor = env->GetMethodID(nullTypeCls, "<init>", "()V");
        return env->NewObject(nullTypeCls, constructor);
    }
    case chip::TLV::kTLVType_Structure:
    case chip::TLV::kTLVType_Array: {
        chip::TLV::TLVType tlvType;
        CHIP_ERROR err = data->EnterContainer(tlvType);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                            ChipLogError(Controller, "Error: TLV container entering failed : %" CHIP_ERROR_FORMAT, err.Format()));

        jobject arrayLists;
        err = chip::JniReferences::GetInstance().CreateArrayList(arrayLists);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                            ChipLogError(Controller, "Error: Create ArrayList object failed : %" CHIP_ERROR_FORMAT, err.Format()));

        while ((err = data->Next()) == CHIP_NO_ERROR)
        {
            chip::TLV::Tag tag = data->GetTag();
            jobject value      = decodeValueFromTLV(env, data);
            VerifyOrReturnValue(
                value != nullptr, nullptr,
                ChipLogError(Controller, "Error when decoding TLV container of type : %" CHIP_ERROR_FORMAT, err.Format()));

            jobject jValue = nullptr;
            if (dataTLVType == chip::TLV::kTLVType_Structure)
            {
                uint64_t tagNum = TagNumFromTag(tag);

                jclass structElementCls = env->FindClass("chip/devicecontroller/ChipTLVType$StructElement");
                jmethodID constructor =
                    env->GetMethodID(structElementCls, "<init>", "(JLchip/devicecontroller/ChipTLVType$BaseTLVType;)V");

                jValue = env->NewObject(structElementCls, constructor, static_cast<jlong>(tagNum), value);
            }
            else
            {
                jValue = value;
            }
            err = chip::JniReferences::GetInstance().AddToList(arrayLists, jValue);
            VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                                ChipLogError(Controller, "Error: Add ArrayList object failed : %" CHIP_ERROR_FORMAT, err.Format()));
        }
        if (err != CHIP_END_OF_TLV)
        {
            ChipLogError(Controller, "Error: TLV container decoding failed: %" CHIP_ERROR_FORMAT, err.Format());
            return nullptr;
        }

        err = data->ExitContainer(tlvType);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                            ChipLogError(Controller, "Error: TLV container exiting failed: %" CHIP_ERROR_FORMAT, err.Format()));

        jclass typeCls = nullptr;
        if (dataTLVType == chip::TLV::kTLVType_Structure)
        {
            typeCls = env->FindClass("chip/devicecontroller/ChipTLVType$StructType");
        }
        else
        {
            typeCls = env->FindClass("chip/devicecontroller/ChipTLVType$ArrayType");
        }
        jmethodID constructor = env->GetMethodID(typeCls, "<init>", "(Ljava/util/ArrayList;)V");
        return env->NewObject(typeCls, constructor, arrayLists);
    }
    default:
        ChipLogError(Controller, "Error: Unsupported TLV type for conversion : %u", data->GetType());
        return nullptr;
    }
}

static bool isEqualTLVType(JNIEnv * env, const char * typeName, jobject tlvType)
{
    jclass tlvEnum = env->FindClass("chip/devicecontroller/ChipTLVType$TLVType");

    jfieldID enumFieldID = env->GetStaticFieldID(tlvEnum, typeName, "Lchip/devicecontroller/ChipTLVType$TLVType;");
    jobject enumObj      = env->GetStaticObjectField(tlvEnum, enumFieldID);

    jmethodID equalsMethodID = env->GetMethodID(tlvEnum, "equals", "(Ljava/lang/Object;)Z");

    return (env->CallBooleanMethod(enumObj, equalsMethodID, tlvType) == JNI_TRUE);
}

static CHIP_ERROR encodeTLVFromValue(JNIEnv * env, jobject jValue, chip::TLV::TLVWriter & writer, chip::TLV::Tag tag)
{
    jmethodID getTypeMethod  = nullptr;
    jmethodID getValueMethod = nullptr;
    ReturnLogErrorOnFailure(chip::JniReferences::GetInstance().FindMethod(
        env, jValue, "type", "()Lchip/devicecontroller/ChipTLVType$TLVType;", &getTypeMethod));

    jobject jType = env->CallObjectMethod(jValue, getTypeMethod);
    if (jType == nullptr)
    {
        ChipLogError(Controller, "Error: Object to encode is corrupt");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (isEqualTLVType(env, "UInt", jType))
    {
        jclass typeClass = env->FindClass("chip/devicecontroller/ChipTLVType$UIntType");
        VerifyOrReturnError(typeClass != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
        getValueMethod = env->GetMethodID(typeClass, "value", "()J");
        VerifyOrReturnError(getValueMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

        jlong value = env->CallLongMethod(jValue, getValueMethod);
        VerifyOrReturnLogError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

        return writer.Put(tag, static_cast<uint64_t>(value));
    }
    if (isEqualTLVType(env, "Int", jType))
    {
        jclass typeClass = env->FindClass("chip/devicecontroller/ChipTLVType$IntType");
        VerifyOrReturnError(typeClass != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
        getValueMethod = env->GetMethodID(typeClass, "value", "()J");
        VerifyOrReturnError(getValueMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

        jlong value = env->CallLongMethod(jValue, getValueMethod);
        VerifyOrReturnLogError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

        return writer.Put(tag, static_cast<int64_t>(value));
    }
    if (isEqualTLVType(env, "Boolean", jType))
    {
        jclass typeClass = env->FindClass("chip/devicecontroller/ChipTLVType$BooleanType");
        VerifyOrReturnError(typeClass != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
        getValueMethod = env->GetMethodID(typeClass, "value", "()Z");
        VerifyOrReturnError(getValueMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

        jboolean value = env->CallBooleanMethod(jValue, getValueMethod);
        VerifyOrReturnLogError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

        return writer.Put(tag, static_cast<bool>(value));
    }
    if (isEqualTLVType(env, "Float", jType))
    {
        jclass typeClass = env->FindClass("chip/devicecontroller/ChipTLVType$FloatType");
        VerifyOrReturnError(typeClass != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
        getValueMethod = env->GetMethodID(typeClass, "value", "()F");
        VerifyOrReturnError(getValueMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

        jfloat value = env->CallFloatMethod(jValue, getValueMethod);
        VerifyOrReturnLogError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

        return writer.Put(tag, static_cast<float>(value));
    }
    if (isEqualTLVType(env, "Double", jType))
    {
        jclass typeClass = env->FindClass("chip/devicecontroller/ChipTLVType$DoubleType");
        VerifyOrReturnError(typeClass != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
        getValueMethod = env->GetMethodID(typeClass, "value", "()D");
        VerifyOrReturnError(getValueMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

        jdouble value = env->CallDoubleMethod(jValue, getValueMethod);
        VerifyOrReturnLogError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

        return writer.Put(tag, static_cast<double>(value));
    }
    if (isEqualTLVType(env, "Null", jType))
    {
        return writer.PutNull(tag);
    }
    if (isEqualTLVType(env, "String", jType))
    {
        jclass typeClass = env->FindClass("chip/devicecontroller/ChipTLVType$StringType");
        VerifyOrReturnError(typeClass != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
        getValueMethod = env->GetMethodID(typeClass, "value", "()Ljava/lang/String;");
        VerifyOrReturnError(getValueMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

        jstring value = (jstring) env->CallObjectMethod(jValue, getValueMethod);
        VerifyOrReturnLogError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

        chip::JniUtfString jniString(env, value);

        return writer.PutString(tag, jniString.c_str());
    }
    if (isEqualTLVType(env, "ByteArray", jType))
    {
        jclass typeClass = env->FindClass("chip/devicecontroller/ChipTLVType$ByteArrayType");
        VerifyOrReturnError(typeClass != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
        getValueMethod = env->GetMethodID(typeClass, "value", "()[B");
        VerifyOrReturnError(getValueMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

        jbyteArray value = (jbyteArray) env->CallObjectMethod(jValue, getValueMethod);
        VerifyOrReturnLogError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

        chip::JniByteArray jniByteArray(env, value);

        return writer.Put(tag, jniByteArray.byteSpan());
    }
    if (isEqualTLVType(env, "Struct", jType))
    {
        jmethodID getSizeMethod = nullptr;

        jclass typeClass = env->FindClass("chip/devicecontroller/ChipTLVType$StructType");
        VerifyOrReturnError(typeClass != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
        getSizeMethod = env->GetMethodID(typeClass, "size", "()I");
        VerifyOrReturnError(getSizeMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);
        getValueMethod = env->GetMethodID(typeClass, "value", "(I)Lchip/devicecontroller/ChipTLVType$StructElement;");
        VerifyOrReturnError(getValueMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

        jclass elementClass = env->FindClass("chip/devicecontroller/ChipTLVType$StructElement");
        VerifyOrReturnError(elementClass != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
        jmethodID getcontextTagNumMethod = env->GetMethodID(elementClass, "contextTagNum", "()J");
        VerifyOrReturnError(getcontextTagNumMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);
        jmethodID getElementValueMethod =
            env->GetMethodID(elementClass, "value", "()Lchip/devicecontroller/ChipTLVType$BaseTLVType;");
        VerifyOrReturnError(getElementValueMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

        jint size = env->CallIntMethod(jValue, getSizeMethod);
        VerifyOrReturnLogError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

        chip::TLV::TLVType outer;
        ReturnErrorOnFailure(writer.StartContainer(tag, chip::TLV::kTLVType_Structure, outer));
        for (int i = 0; i < static_cast<int>(size); i++)
        {
            jobject eachElement = env->CallObjectMethod(jValue, getValueMethod, static_cast<jint>(i));
            VerifyOrReturnLogError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

            jlong jEachTag = env->CallLongMethod(eachElement, getcontextTagNumMethod);
            VerifyOrReturnLogError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

            jobject jEachValue = env->CallObjectMethod(eachElement, getElementValueMethod);
            VerifyOrReturnLogError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

            uint64_t tagValue       = static_cast<uint64_t>(jEachTag);
            chip::TLV::Tag innerTag = chip::TLV::ContextTag(static_cast<uint8_t>(tagValue));
            ReturnErrorOnFailure(encodeTLVFromValue(env, jEachValue, writer, innerTag));
        }
        ReturnErrorOnFailure(writer.EndContainer(outer));
        return CHIP_NO_ERROR;
    }
    if (isEqualTLVType(env, "Array", jType))
    {
        jmethodID getSizeMethod = nullptr;

        jclass typeClass = env->FindClass("chip/devicecontroller/ChipTLVType$ArrayType");
        VerifyOrReturnError(typeClass != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);

        getSizeMethod = env->GetMethodID(typeClass, "size", "()I");
        VerifyOrReturnError(getSizeMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);
        getValueMethod = env->GetMethodID(typeClass, "value", "(I)Lchip/devicecontroller/ChipTLVType$BaseTLVType;");
        VerifyOrReturnError(getValueMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

        jint size = env->CallIntMethod(jValue, getSizeMethod);
        VerifyOrReturnLogError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

        chip::TLV::TLVType outer;
        ReturnErrorOnFailure(writer.StartContainer(tag, chip::TLV::kTLVType_Array, outer));
        for (int i = 0; i < static_cast<int>(size); i++)
        {
            jobject eachValue = env->CallObjectMethod(jValue, getValueMethod, static_cast<jint>(i));
            VerifyOrReturnLogError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

            ReturnErrorOnFailure(encodeTLVFromValue(env, eachValue, writer, chip::TLV::AnonymousTag()));
        }
        ReturnErrorOnFailure(writer.EndContainer(outer));
        return CHIP_NO_ERROR;
    }
    if (isEqualTLVType(env, "Empty", jType))
    {
        // For optional Value
        return CHIP_NO_ERROR;
    }

    ChipLogError(Controller, "Error: Unsupported type to encode");
    return CHIP_ERROR_INVALID_ARGUMENT;
}

JNI_METHOD(jbyteArray, BaseChipCluster, encodeToTlv)(JNIEnv * env, jclass clazz, jobject value)
{
    VerifyOrReturnValue(value != nullptr, nullptr, ChipLogError(Controller, "invalid parameter: value is null"));
    chip::TLV::TLVWriter writer;
    uint8_t buffer[CHIP_TLV_WRITER_BUFFER_SIZE];
    jbyteArray tlv = nullptr;
    writer.Init(buffer, sizeof(buffer));

    CHIP_ERROR err = encodeTLVFromValue(env, value, writer, chip::TLV::AnonymousTag());
    VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr, ChipLogError(Controller, "Encode Error: %" CHIP_ERROR_FORMAT, err.Format()));

    err = chip::JniReferences::GetInstance().N2J_ByteArray(env, buffer, static_cast<jint>(writer.GetLengthWritten()), tlv);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr, ChipLogError(Controller, "JNI Error: %" CHIP_ERROR_FORMAT, err.Format()));

    return tlv;
}

JNI_METHOD(jobject, BaseChipCluster, decodeFromTlv)(JNIEnv * env, jclass clazz, jbyteArray tlvBytesObj)
{
    VerifyOrReturnValue(tlvBytesObj != nullptr, nullptr, ChipLogError(Controller, "invalid parameter: tlvBytesObj is null"));
    chip::JniByteArray tlvBytesObjBytes(env, tlvBytesObj);

    chip::TLV::TLVReader reader;
    reader.Init(tlvBytesObjBytes.byteSpan());
    VerifyOrReturnValue(reader.Next() == CHIP_NO_ERROR, nullptr, ChipLogError(Controller, "TLV Parsing is wrong"));

    return decodeValueFromTLV(env, &reader);
}
