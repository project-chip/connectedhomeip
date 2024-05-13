#include "lib/core/CHIPError.h"
#include "lib/support/JniTypeWrappers.h"

// #include <lib/support/CHIPMem.h>
// #include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>
#include <lib/support/logging/CHIPLogging.h>

#include "CHIPAttributeTLVValueDecoder.h"
#include "CHIPEventTLVValueDecoder.h"

#include <jni.h>

using namespace chip;

#define JNI_METHOD(RETURN, METHOD_NAME)                                                                                            \
    extern "C" JNIEXPORT RETURN JNICALL Java_chip_devicecontroller_ChipTLVValueDecoder_##METHOD_NAME

extern CHIP_ERROR ParseAttributePath(jobject attributePath, EndpointId & outEndpointId, ClusterId & outClusterId,
                                     AttributeId & outAttributeId);
extern CHIP_ERROR ParseEventPath(jobject eventPath, EndpointId & outEndpointId, ClusterId & outClusterId, EventId & outEventId,
                                 bool & outIsUrgent);

JNI_METHOD(jobject, decodeAttributeValue)(JNIEnv * env, jclass clazz, jobject attributePath, jbyteArray jTlv)
{
    EndpointId endpointId;
    ClusterId clusterId;
    AttributeId attributeId;
    CHIP_ERROR err = ParseAttributePath(attributePath, endpointId, clusterId, attributeId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Controller, "decode error attributePath");
        return nullptr;
    }

    JniByteArray tlv(env, jTlv);

    chip::app::ConcreteAttributePath path(endpointId, clusterId, attributeId);

    chip::TLV::TLVReader reader;
    reader.Init(tlv.byteSpan());
    reader.Next();

    jobject ret = DecodeAttributeValue(path, reader, &err);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Controller, "decode error attributeValue");
        return nullptr;
    }

    return ret;
}

JNI_METHOD(jobject, decodeEventValue)(JNIEnv * env, jclass clazz, jobject eventPath, jbyteArray jTlv)
{
    EndpointId endpointId;
    ClusterId clusterId;
    EventId eventId;
    bool isUrgent;
    CHIP_ERROR err = ParseEventPath(eventPath, endpointId, clusterId, eventId, isUrgent);
    if (err != CHIP_NO_ERROR)
    {
        return nullptr;
    }

    JniByteArray tlv(env, jTlv);

    chip::app::ConcreteEventPath path(endpointId, clusterId, eventId);
    chip::TLV::TLVReader reader;

    reader.Init(tlv.byteSpan());
    reader.Next();

    jobject ret = DecodeEventValue(path, reader, &err);

    if (err != CHIP_NO_ERROR)
    {
        return nullptr;
    }

    return ret;
}
