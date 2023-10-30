#include <controller/CHIPCluster.h>
#include <jni.h>
#include <lib/support/JniReferences.h>
#include <platform/PlatformManager.h>

#include "AndroidCallbacks.h"

#define JNI_METHOD(RETURN, CLASS_NAME, METHOD_NAME)                                                                                \
    extern "C" JNIEXPORT RETURN JNICALL Java_chip_devicecontroller_ChipClusters_00024##CLASS_NAME##_##METHOD_NAME

#define CHIP_TLV_WRITER_BUFFER_SIZE 1024

JNI_METHOD(void, BaseChipCluster, readAttribute)
(JNIEnv * env, jobject self, jlong callbackHandle, jlong devicePtr, jint jEndpointId, jlong jClusterId, jlong jAttributeId, jboolean isFabricFiltered, jlong imTimeoutMs)
{
    using namespace chip;
    using namespace chip::Controller;

    ChipLogError(Controller, "BaseChipCluster:readAttribute");
    DeviceLayer::StackLock lock;
    CHIP_ERROR err = CHIP_NO_ERROR;

    auto callback = reinterpret_cast<ReportCallback *>(callbackHandle);
    std::vector<app::AttributePathParams> attributePathParamsList;
    app::ReadClient * readClient = nullptr;
    DeviceProxy * device         = reinterpret_cast<DeviceProxy *>(devicePtr);
    if (device == nullptr)
    {
        ChipLogProgress(Controller, "Could not cast device pointer to Device object");
        return;
    }
    chip::app::ReadPrepareParams params(device->GetSecureSession().Value());

    EndpointId endpointId = static_cast<EndpointId>(jEndpointId);
    ClusterId clusterId = static_cast<ClusterId>(jClusterId);
    AttributeId attributeId = static_cast<AttributeId>(jAttributeId);
    attributePathParamsList.push_back(app::AttributePathParams(endpointId, clusterId, attributeId));

    params.mpAttributePathParamsList    = attributePathParamsList.data();
    params.mAttributePathParamsListSize = 1;

    params.mIsFabricFiltered = (isFabricFiltered != JNI_FALSE);
    params.mTimeout          = imTimeoutMs != 0 ? System::Clock::Milliseconds32(imTimeoutMs) : System::Clock::kZero;

    readClient = Platform::New<app::ReadClient>(app::InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                                callback->mClusterCacheAdapter.GetBufferedCallback(),
                                                app::ReadClient::InteractionType::Read);

    SuccessOrExit(err = readClient->SendRequest(params));
    callback->mReadClient = readClient;

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JNI IM Read Error: %s", err.AsString());
        if (err == CHIP_JNI_ERROR_EXCEPTION_THROWN)
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        callback->OnError(err);
        if (readClient != nullptr)
        {
            Platform::Delete(readClient);
        }
        if (callback != nullptr)
        {
            Platform::Delete(callback);
        }
    }
}

JNI_METHOD(void, BaseChipCluster, writeAttribute)
(JNIEnv * env, jobject self, jlong callbackHandle, jlong devicePtr, jint jEndpointId, jlong jClusterId, jlong jAttributeId, jbyteArray tlvBytes, jint timedRequestTimeoutMs, jlong imTimeoutMs)
{
    using namespace chip;
    using namespace chip::Controller;

    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    auto callback                           = reinterpret_cast<WriteAttributesCallback *>(callbackHandle);
    app::WriteClient * writeClient          = nullptr;
    uint16_t convertedTimedRequestTimeoutMs = static_cast<uint16_t>(timedRequestTimeoutMs);

    ChipLogDetail(Controller, "IM write() called");

    if (tlvBytes == nullptr) {
        err = CHIP_ERROR_INCORRECT_STATE;
        ChipLogError(Controller, "JNI IM Write Error: %s", err.AsString());
        return;
    }

    EndpointId endpointId = 0;
    ClusterId clusterId = static_cast<ClusterId>(jClusterId);
    AttributeId attributeId = static_cast<AttributeId>(jAttributeId);
    bool isGroupSession = false;
    DeviceProxy * device = reinterpret_cast<DeviceProxy *>(devicePtr);
    TLV::TLVReader reader;

    isGroupSession = device->GetSecureSession().Value()->IsGroupSession();

    if (isGroupSession)
    {
        endpointId = kInvalidEndpointId;
    }
    else
    {
        endpointId = static_cast<EndpointId>(jEndpointId);
    }

    app::ConcreteDataAttributePath path(static_cast<EndpointId>(endpointId), static_cast<ClusterId>(clusterId),
                                        static_cast<AttributeId>(attributeId), Optional<DataVersion>());

    writeClient = Platform::New<app::WriteClient>(
        device->GetExchangeManager(), callback->GetChunkedWriteCallback(),
        convertedTimedRequestTimeoutMs != 0 ? Optional<uint16_t>(convertedTimedRequestTimeoutMs) : Optional<uint16_t>::Missing());

    JniByteArray tlvByteArray(env, tlvBytes);

    VerifyOrExit(device != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(device->GetSecureSession().HasValue(), err = CHIP_ERROR_MISSING_SECURE_SESSION);
    VerifyOrExit(writeClient != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    reader.Init(tlvByteArray.byteSpan());
    SuccessOrExit(err = reader.Next());
    SuccessOrExit(err = writeClient->PutPreencodedAttribute(path, reader));

    err = writeClient->SendWriteRequest(device->GetSecureSession().Value(),
                                        imTimeoutMs != 0 ? System::Clock::Milliseconds32(imTimeoutMs) : System::Clock::kZero);
    SuccessOrExit(err);
    callback->mWriteClient = writeClient;
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JNI IM Write Error: %s", err.AsString());
        if (err == CHIP_JNI_ERROR_EXCEPTION_THROWN)
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        callback->OnError(writeClient, err);
        if (writeClient != nullptr)
        {
            Platform::Delete(writeClient);
        }
        if (callback != nullptr)
        {
            Platform::Delete(callback);
        }
    }
}

JNI_METHOD(void, BaseChipCluster, subscribeAttribute)
(JNIEnv * env, jobject self, jlong callbackHandle, jlong devicePtr, jint jEndpointId, jlong jClusterId, jlong jAttributeId, jint minInterval, jint maxInterval, jboolean keepSubscriptions, jboolean isFabricFiltered, jlong imTimeoutMs)
{
    using namespace chip;
    using namespace chip::Controller;

    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err               = CHIP_NO_ERROR;
    app::ReadClient * readClient = nullptr;
    auto callback                = reinterpret_cast<ReportCallback *>(callbackHandle);
    DeviceProxy * device         = reinterpret_cast<DeviceProxy *>(devicePtr);
    if (device == nullptr)
    {
        ChipLogProgress(Controller, "Could not cast device pointer to Device object");
        return;
    }
    app::ReadPrepareParams params(device->GetSecureSession().Value());

    uint16_t aImTimeoutMs             = static_cast<uint16_t>(imTimeoutMs);
    params.mMinIntervalFloorSeconds   = static_cast<uint16_t>(minInterval);
    params.mMaxIntervalCeilingSeconds = static_cast<uint16_t>(maxInterval);
    params.mKeepSubscriptions         = (keepSubscriptions != JNI_FALSE);
    params.mIsFabricFiltered          = (isFabricFiltered != JNI_FALSE);
    params.mTimeout                   = aImTimeoutMs != 0 ? System::Clock::Milliseconds32(aImTimeoutMs) : System::Clock::kZero;

    EndpointId endpointId = static_cast<EndpointId>(jEndpointId);
    ClusterId clusterId = static_cast<ClusterId>(jClusterId);
    AttributeId attributeId = static_cast<AttributeId>(jAttributeId);

    std::unique_ptr<chip::app::AttributePathParams[]> attributePaths(new chip::app::AttributePathParams[1]);
    attributePaths[0] = chip::app::AttributePathParams(endpointId, clusterId, attributeId);

    params.mpAttributePathParamsList    = attributePaths.get();
    params.mAttributePathParamsListSize = 1;

    readClient = Platform::New<app::ReadClient>(app::InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                                callback->mClusterCacheAdapter.GetBufferedCallback(),
                                                app::ReadClient::InteractionType::Subscribe);

    attributePaths.release();

    SuccessOrExit(err = readClient->SendAutoResubscribeRequest(std::move(params)));
    callback->mReadClient = readClient;

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JNI IM Subscribe Error: %s", err.AsString());
        if (err == CHIP_JNI_ERROR_EXCEPTION_THROWN)
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        callback->OnError(err);
        if (readClient != nullptr)
        {
            Platform::Delete(readClient);
        }
        if (callback != nullptr)
        {
            Platform::Delete(callback);
        }
    }
}

JNI_METHOD(void, BaseChipCluster, invoke)
(JNIEnv * env, jobject self, jlong callbackHandle, jlong devicePtr, jint jEndpointId, jlong jClusterId, jlong jCommandId, jbyteArray tlvBytes, jint timedRequestTimeoutMs, jlong imTimeoutMs)
{
    using namespace chip;
    using namespace chip::Controller;

    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    auto callback                           = reinterpret_cast<InvokeCallback *>(callbackHandle);
    app::CommandSender * commandSender      = nullptr;
    EndpointId endpointId = static_cast<EndpointId>(jEndpointId);
    ClusterId clusterId = static_cast<ClusterId>(jClusterId);
    CommandId commandId = static_cast<CommandId>(jCommandId);
    uint16_t convertedTimedRequestTimeoutMs = static_cast<uint16_t>(timedRequestTimeoutMs);
    ChipLogDetail(Controller, "IM invoke() called");

    DeviceProxy * device = reinterpret_cast<DeviceProxy *>(devicePtr);
    VerifyOrExit(device != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(device->GetSecureSession().HasValue(), err = CHIP_ERROR_MISSING_SECURE_SESSION);

    commandSender = Platform::New<app::CommandSender>(callback, device->GetExchangeManager(), convertedTimedRequestTimeoutMs != 0);
    {
        app::CommandPathParams path(endpointId, clusterId, commandId, app::CommandPathFlags::kEndpointIdValid);
        JniByteArray tlvBytesObjBytes(env, tlvBytes);
        TLV::TLVWriter * writer = nullptr;
        TLV::TLVReader reader;

        SuccessOrExit(err = commandSender->PrepareCommand(path, false /* aStartDataStruct */));

        writer = commandSender->GetCommandDataIBTLVWriter();
        VerifyOrExit(writer != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        reader.Init(tlvBytesObjBytes.byteSpan());
        SuccessOrExit(err = reader.Next());
        SuccessOrExit(err = writer->CopyContainer(TLV::ContextTag(app::CommandDataIB::Tag::kFields), reader));
    }
    SuccessOrExit(err = commandSender->FinishCommand(convertedTimedRequestTimeoutMs != 0
                                                         ? Optional<uint16_t>(convertedTimedRequestTimeoutMs)
                                                         : Optional<uint16_t>::Missing()));

    SuccessOrExit(err = commandSender->SendCommandRequest(device->GetSecureSession().Value(), imTimeoutMs != 0 ? MakeOptional(System::Clock::Milliseconds32(imTimeoutMs)) : Optional<System::Clock::Timeout>::Missing()));

    callback->mCommandSender = commandSender;

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JNI IM Invoke Error: %s", err.AsString());
        if (err == CHIP_JNI_ERROR_EXCEPTION_THROWN)
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        callback->OnError(nullptr, err);
        if (commandSender != nullptr)
        {
            Platform::Delete(commandSender);
        }
        if (callback != nullptr)
        {
            Platform::Delete(callback);
        }
    }
}

jobject decodeValueFromTLV(JNIEnv *env, chip::TLV::TLVReader * data);
static CHIP_ERROR encodeTLVFromValue(JNIEnv *env, jobject jObject, chip::TLV::TLVWriter & writer, chip::TLV::Tag tag);

jobject decodeValueFromTLV(JNIEnv *env, chip::TLV::TLVReader * data)
{    
    chip::TLV::TLVType dataTLVType = data->GetType();
    switch (dataTLVType) {
    case chip::TLV::kTLVType_SignedInteger: {
        int64_t val;
        CHIP_ERROR err = data->Get(val);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr, ChipLogError(Controller, "Error: TLV signed integer decoding failed : %" CHIP_ERROR_FORMAT , err.Format()));
        jclass intTypeCls = env->FindClass("chip/devicecontroller/ChipTLVType$IntType");
        jmethodID constructor      = env->GetMethodID(intTypeCls, "<init>", "(J)V");
        return env->NewObject(intTypeCls, constructor, static_cast<jlong>(val));
    }
    case chip::TLV::kTLVType_UnsignedInteger: {
        uint64_t val;
        CHIP_ERROR err = data->Get(val);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr, ChipLogError(Controller, "Error: TLV unsigned integer decoding failed : %" CHIP_ERROR_FORMAT , err.Format()));
        jclass uintTypeCls = env->FindClass("chip/devicecontroller/ChipTLVType$UIntType");
        jmethodID constructor      = env->GetMethodID(uintTypeCls, "<init>", "(J)V");
        return env->NewObject(uintTypeCls, constructor, static_cast<jlong>(val));
    }
    case chip::TLV::kTLVType_Boolean: {
        bool val;
        CHIP_ERROR err = data->Get(val);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr, ChipLogError(Controller, "Error: TLV boolean decoding failed : %" CHIP_ERROR_FORMAT , err.Format()));
        jclass booleanTypeCls = env->FindClass("chip/devicecontroller/ChipTLVType$BooleanType");
        jmethodID constructor      = env->GetMethodID(booleanTypeCls, "<init>", "(Z)V");
        return env->NewObject(booleanTypeCls, constructor, static_cast<jboolean>(val));
    }
    case chip::TLV::kTLVType_FloatingPointNumber: {
        // Try float first
        float floatValue;
        CHIP_ERROR err = data->Get(floatValue);
        if (err == CHIP_NO_ERROR) {
            jclass floatTypeCls = env->FindClass("chip/devicecontroller/ChipTLVType$FloatType");
            jmethodID constructor      = env->GetMethodID(floatTypeCls, "<init>", "(F)V");
            return env->NewObject(floatTypeCls, constructor, static_cast<jfloat>(floatValue));
        }
        double val;
        err = data->Get(val);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr, ChipLogError(Controller, "Error: TLV floating point decoding failed : %" CHIP_ERROR_FORMAT , err.Format()));
        jclass doubleTypeCls = env->FindClass("chip/devicecontroller/ChipTLVType$DoubleType");
        jmethodID doubleConstructor      = env->GetMethodID(doubleTypeCls, "<init>", "(D)V");
        return env->NewObject(doubleTypeCls, doubleConstructor, static_cast<jdouble>(val));
    }
    case chip::TLV::kTLVType_UTF8String: {
        chip::CharSpan stringValue;
        CHIP_ERROR err = data->Get(stringValue);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr, ChipLogError(Controller, "Error: TLV UTF8String decoding failed : %" CHIP_ERROR_FORMAT , err.Format()));
        chip::UtfString stringObj(env, stringValue);
        jclass stringTypeCls = env->FindClass("chip/devicecontroller/ChipTLVType$StringType");
        jmethodID constructor      = env->GetMethodID(stringTypeCls, "<init>", "(Ljava/lang/String;)V");
        return env->NewObject(stringTypeCls, constructor, stringObj.jniValue());
    }
    case chip::TLV::kTLVType_ByteString: {
        chip::ByteSpan bytesValue;
        CHIP_ERROR err = data->Get(bytesValue);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr, ChipLogError(Controller, "Error: TLV ByteString decoding failed : %" CHIP_ERROR_FORMAT , err.Format()));
        chip::ByteArray byteArrayObj(env, bytesValue);
        jclass stringTypeCls = env->FindClass("chip/devicecontroller/ChipTLVType$ByteArrayType");
        jmethodID constructor      = env->GetMethodID(stringTypeCls, "<init>", "([B)V");
        return env->NewObject(stringTypeCls, constructor, byteArrayObj.jniValue());
    }
    case chip::TLV::kTLVType_Null: {
        jclass nullTypeCls = env->FindClass("chip/devicecontroller/ChipTLVType$NullType");
        jmethodID constructor      = env->GetMethodID(nullTypeCls, "<init>", "()V");
        return env->NewObject(nullTypeCls, constructor);
    }
    case chip::TLV::kTLVType_Structure:
    case chip::TLV::kTLVType_Array: {
        chip::TLV::TLVType tlvType;
        CHIP_ERROR err = data->EnterContainer(tlvType);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr, ChipLogError(Controller, "Error: TLV container entering failed : %" CHIP_ERROR_FORMAT , err.Format()));

        jobject arrayLists;
        err = chip::JniReferences::GetInstance().CreateArrayList(arrayLists);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr, ChipLogError(Controller, "Error: Create ArrayList object failed : %" CHIP_ERROR_FORMAT , err.Format()));

        while ((err = data->Next()) == CHIP_NO_ERROR) {
            chip::TLV::Tag tag = data->GetTag();
            jobject value = decodeValueFromTLV(env, data);
            VerifyOrReturnValue(value != nullptr, nullptr, ChipLogError(Controller, "Error when decoding TLV container of type : %" CHIP_ERROR_FORMAT , err.Format()));

            jobject jValue = nullptr;
            if (dataTLVType == chip::TLV::kTLVType_Structure) {
                uint64_t tagNum = TagNumFromTag(tag);

                jclass structElementCls = env->FindClass("chip/devicecontroller/ChipTLVType$StructElement");
                jmethodID constructor      = env->GetMethodID(structElementCls, "<init>", "(JLchip/devicecontroller/ChipTLVType$BaseTLVType;)V");

                jValue = env->NewObject(structElementCls, constructor, static_cast<jlong>(tagNum), value);
            } else {
                jValue = value;
            }
            err = chip::JniReferences::GetInstance().AddToList(arrayLists, jValue);
            VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr, ChipLogError(Controller, "Error: Add ArrayList object failed : %" CHIP_ERROR_FORMAT , err.Format()));
        }
        if (err != CHIP_END_OF_TLV) {
            ChipLogError(Controller, "Error: TLV container decoding failed: %" CHIP_ERROR_FORMAT, err.Format());
            return nullptr;
        }

        err = data->ExitContainer(tlvType);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr, ChipLogError(Controller, "Error: TLV container exiting failed: %" CHIP_ERROR_FORMAT, err.Format()));
        return arrayLists;
    }
    default:
        ChipLogError(Controller, "Error: Unsupported TLV type for conversion : %u", data->GetType());
        return nullptr;
    }
}

static bool isEqualTLVType(JNIEnv * env, const char *typeName, jobject tlvType)
{
    jclass tlvEnum = env->FindClass("chip/devicecontroller/ChipTLVType$TLVType");

    jfieldID enumFieldID = env->GetStaticFieldID(tlvEnum, typeName, "chip/devicecontroller/ChipTLVType$TLVType");
    jobject enumObj      = env->GetStaticObjectField(tlvEnum, enumFieldID);

    jmethodID equalsMethodID = env->GetMethodID(tlvEnum, "equals", "(Ljava/lang/Object;)Z");

    return (env->CallBooleanMethod(enumObj, equalsMethodID, tlvType) == JNI_TRUE);
}

static CHIP_ERROR encodeTLVFromValue(JNIEnv *env, jobject jValue, chip::TLV::TLVWriter & writer, chip::TLV::Tag tag)
{
    jmethodID getTypeMethod = nullptr;
    jmethodID getValueMethod = nullptr;
    ReturnLogErrorOnFailure(chip::JniReferences::GetInstance().FindMethod(env, jValue, "type", "()Lchip/devicecontroller/ChipTLVType$TLVType;", &getTypeMethod));

    jobject jType = env->CallObjectMethod(jValue, getTypeMethod);
    if (jType == nullptr) {
        ChipLogError(Controller, "Error: Object to encode is corrupt");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (isEqualTLVType(env, "UInt", jType)) {
        jclass typeClass = env->FindClass("chip/devicecontroller/ChipTLVType$UIntType");
        VerifyOrReturnError(typeClass != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
        getValueMethod = env->GetMethodID(typeClass, "value", "()J");
        VerifyOrReturnError(getValueMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

        jlong value = env->CallLongMethod(jValue, getValueMethod);
        VerifyOrReturnLogError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

        return writer.Put(tag, static_cast<uint64_t>(value));
    }
    if (isEqualTLVType(env, "Int", jType)) {
        jclass typeClass = env->FindClass("chip/devicecontroller/ChipTLVType$IntType");
        VerifyOrReturnError(typeClass != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
        getValueMethod = env->GetMethodID(typeClass, "value", "()J");
        VerifyOrReturnError(getValueMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

        jlong value = env->CallLongMethod(jValue, getValueMethod);
        VerifyOrReturnLogError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

        return writer.Put(tag, static_cast<int64_t>(value));
    }
    if (isEqualTLVType(env, "Boolean", jType)) {
        jclass typeClass = env->FindClass("chip/devicecontroller/ChipTLVType$BooleanType");
        VerifyOrReturnError(typeClass != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
        getValueMethod = env->GetMethodID(typeClass, "value", "()Z");
        VerifyOrReturnError(getValueMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

        jboolean value = env->CallBooleanMethod(jValue, getValueMethod);
        VerifyOrReturnLogError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

        return writer.Put(tag, static_cast<bool>(value));
    }
    if (isEqualTLVType(env, "Float", jType)) {
        jclass typeClass = env->FindClass("chip/devicecontroller/ChipTLVType$FloatType");
        VerifyOrReturnError(typeClass != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
        getValueMethod = env->GetMethodID(typeClass, "value", "()F");
        VerifyOrReturnError(getValueMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

        jfloat value = env->CallFloatMethod(jValue, getValueMethod);
        VerifyOrReturnLogError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

        return writer.Put(tag, static_cast<float>(value));
    }
    if (isEqualTLVType(env, "Double", jType)) {
        jclass typeClass = env->FindClass("chip/devicecontroller/ChipTLVType$DoubleType");
        VerifyOrReturnError(typeClass != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
        getValueMethod = env->GetMethodID(typeClass, "value", "()D");
        VerifyOrReturnError(getValueMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

        jdouble value = env->CallDoubleMethod(jValue, getValueMethod);
        VerifyOrReturnLogError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

        return writer.Put(tag, static_cast<double>(value));
    }
    if (isEqualTLVType(env, "Null", jType)) {
        return writer.PutNull(tag);
    }
    if (isEqualTLVType(env, "String", jType)) {
        jclass typeClass = env->FindClass("chip/devicecontroller/ChipTLVType$StringType");
        VerifyOrReturnError(typeClass != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
        getValueMethod = env->GetMethodID(typeClass, "value", "()Ljava/lang/String;");
        VerifyOrReturnError(getValueMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

        jstring value = (jstring)env->CallObjectMethod(jValue, getValueMethod);
        VerifyOrReturnLogError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

        chip::JniUtfString jniString(env, value);

        return writer.PutString(tag, jniString.c_str());
    }
    if (isEqualTLVType(env, "ByteArray", jType)) {
        jclass typeClass = env->FindClass("chip/devicecontroller/ChipTLVType$ByteArrayType");
        VerifyOrReturnError(typeClass != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
        getValueMethod = env->GetMethodID(typeClass, "value", "()[B");
        VerifyOrReturnError(getValueMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

        jbyteArray value = (jbyteArray)env->CallObjectMethod(jValue, getValueMethod);
        VerifyOrReturnLogError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

        chip::JniByteArray jniByteArray(env, value);

        return writer.Put(tag, jniByteArray.byteSpan());
    }
    if (isEqualTLVType(env, "Struct", jType)) {
        jmethodID getSizeMethod = nullptr;

        jclass typeClass = env->FindClass("chip/devicecontroller/ChipTLVType$StructType");
        VerifyOrReturnError(typeClass != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
        getSizeMethod = env->GetMethodID(typeClass, "size", "()I");
        VerifyOrReturnError(getValueMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);
        getValueMethod = env->GetMethodID(typeClass, "value", "(I)Lchip/devicecontroller/ChipTLVType$StructElement;");
        VerifyOrReturnError(getValueMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

        jclass elementClass = env->FindClass("chip/devicecontroller/ChipTLVType$StructElement");
        VerifyOrReturnError(elementClass != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);
        jmethodID getcontextTagNumMethod = env->GetMethodID(elementClass, "contextTagNum", "()J");
        VerifyOrReturnError(getcontextTagNumMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);
        jmethodID getElementValueMethod = env->GetMethodID(elementClass, "value", "()Lchip/devicecontroller/ChipTLVType$BaseTLVType;");
        VerifyOrReturnError(getElementValueMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

        jint size = env->CallIntMethod(jValue, getSizeMethod);
        VerifyOrReturnLogError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

        chip::TLV::TLVType outer;
        ReturnErrorOnFailure(writer.StartContainer(tag, chip::TLV::kTLVType_Structure, outer));
        for (int i = 0 ; i < static_cast<int>(size) ; i++) {
            jobject eachElement = env->CallObjectMethod(jValue, getValueMethod, static_cast<jint>(i));
            VerifyOrReturnLogError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

            jlong jEachTag = env->CallLongMethod(eachElement, getcontextTagNumMethod);
            VerifyOrReturnLogError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

            jobject jEachValue = env->CallObjectMethod(eachElement, getElementValueMethod);
            VerifyOrReturnLogError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

            uint64_t tagValue = static_cast<uint64_t>(jEachTag);
            chip::TLV::Tag innerTag = chip::TLV::ContextTag(static_cast<uint8_t>(tagValue));
            ReturnErrorOnFailure(encodeTLVFromValue(env, jEachValue, writer, innerTag));
        }
        ReturnErrorOnFailure(writer.EndContainer(outer));
        return CHIP_NO_ERROR;
    }
    if (isEqualTLVType(env, "Array", jType)) {
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
        for (int i = 0 ; i < static_cast<int>(size) ; i++) {
            jobject eachValue = env->CallObjectMethod(jValue, getValueMethod, static_cast<jint>(i));
            VerifyOrReturnLogError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

            ReturnErrorOnFailure(encodeTLVFromValue(env, eachValue, writer, chip::TLV::AnonymousTag()));
        }
        ReturnErrorOnFailure(writer.EndContainer(outer));
        return CHIP_NO_ERROR;
    }
    if (isEqualTLVType(env, "Empty", jType)) {
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
    writer.Init(buffer, sizeof(buffer));

    CHIP_ERROR err = encodeTLVFromValue(env, value, writer, chip::TLV::AnonymousTag());
    VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr, ChipLogError(Controller, "Encode Error: %" CHIP_ERROR_FORMAT, err.Format()));

    chip::ByteArray jbyteArray(env, reinterpret_cast<jbyte *>(buffer), writer.GetLengthWritten());
    return jbyteArray.jniValue();
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