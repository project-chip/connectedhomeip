/*
 *   Copyright (c) 2023 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "OTAProviderDelegateBridge.h"

#include <app/clusters/ota-provider/ota-provider.h>
#include <app/data-model/List.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider::Commands;

constexpr EndpointId kOtaProviderEndpoint = 0;

// Time in seconds after which the requestor should retry calling query image if
// busy status is receieved.  The spec minimum is 2 minutes, but in practice OTA
// generally takes a lot longer than that and devices only retry a few times
// before giving up.  Default to 10 minutes for now, until we have a better
// system of computing an expected completion time for the currently-running
// OTA.
constexpr uint32_t kDelayedActionTimeSeconds = 600;

OTAProviderDelegateBridge::~OTAProviderDelegateBridge()
{
    mBdxOTASender->ResetState();
    Clusters::OTAProvider::SetDelegate(kOtaProviderEndpoint, nullptr);
    mBdxOTASender = nullptr;
}

CHIP_ERROR OTAProviderDelegateBridge::Init(chip::System::Layer * systemLayer, chip::Messaging::ExchangeManager * exchangeManager,
                                           jobject otaProviderDelegate)
{
    ReturnLogErrorOnFailure(mOtaProviderDelegate.Init(otaProviderDelegate));

    Clusters::OTAProvider::SetDelegate(kOtaProviderEndpoint, this);
    mBdxOTASender = std::make_unique<BdxOTASender>(mOtaProviderDelegate.ObjectRef());
    return mBdxOTASender->Init(systemLayer, exchangeManager);
}

void OTAProviderDelegateBridge::Shutdown()
{
    VerifyOrReturn(mBdxOTASender != nullptr, ChipLogError(Controller, "BdxOTASender is null"));
    CHIP_ERROR err = mBdxOTASender->Shutdown();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Controller, "OTAProviderDelegateBridge-Shutdown : %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void GenerateUpdateToken(uint8_t * buf, size_t bufSize)
{
    for (size_t i = 0; i < bufSize; ++i)
    {
        buf[i] = chip::Crypto::GetRandU8();
    }
}

void OTAProviderDelegateBridge::sendOTAQueryFailure(uint8_t status)
{
    VerifyOrReturn(mOtaProviderDelegate.HasValidObjectRef(), ChipLogError(Controller, "mOtaProviderDelegate is null"));

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    jobject otaProviderDelegate = mOtaProviderDelegate.ObjectRef();
    jmethodID handleOTAQueryFailureMethod;
    CHIP_ERROR err = JniReferences::GetInstance().FindMethod(env, otaProviderDelegate, "handleOTAQueryFailure", "(I)V",
                                                             &handleOTAQueryFailureMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find handleOTAQueryFailure method"));

    env->CallVoidMethod(otaProviderDelegate, handleOTAQueryFailureMethod, static_cast<jint>(status));
    if (env->ExceptionCheck())
    {
        ChipLogError(Support, "Exception in call java method");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
    return;
}
void OTAProviderDelegateBridge::HandleQueryImage(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                 const QueryImage::DecodableType & commandData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t errorStatus = static_cast<uint8_t>(OTAQueryStatus::kNotAvailable);

    NodeId nodeId = kUndefinedNodeId;

    FabricIndex fabricIndex = kUndefinedFabricIndex;
    ScopedNodeId ourNodeId;

    VendorId vendorId = VendorId::Unspecified;
    uint16_t productId = 0;
    uint32_t softwareVersion = 0;
    DataModel::DecodableList<OTADownloadProtocol> protocolsSupported;
    Optional<uint16_t> hardwareVersion;
    Optional<chip::CharSpan> location;
    Optional<bool> requestorCanConsent;
    Optional<chip::ByteSpan> metadataForProvider;

    bool isBDXProtocolSupported = false;

    ConcreteCommandPath cachedCommandPath(commandPath.mEndpointId, commandPath.mClusterId, commandPath.mCommandId);

    assertChipStackLockedByCurrentThread();

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    jobject otaProviderDelegate = nullptr;
    chip::JniLocalReferenceManager manager(env);
    jmethodID handleQueryImageMethod = nullptr;

    jobject boxedHardwareVersion = nullptr;
    jobject boxedLocation = nullptr;
    jobject boxedRequestorCanConsent = nullptr;
    jobject boxedMetadataForProvider = nullptr;
    jobject jResponse = nullptr;

    jmethodID getSoftwareVersionMethod= nullptr;
    jmethodID getSoftwareVersionStringMethod = nullptr;
    jmethodID getFilePathMethod = nullptr;

    jobject jSoftwareVersion = nullptr;
    jstring jSoftwareVersionString = nullptr;
    jstring jFilePath = nullptr;

    bool hasUpdate = false;
    Commands::QueryImageResponse::Type response;

    char uriBuffer[kMaxBDXURILen];
    MutableCharSpan uri(uriBuffer);

    VerifyOrExit(mOtaProviderDelegate.HasValidObjectRef(),
                   commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Failure));

    VerifyOrExit(mBdxOTASender != nullptr, commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Failure));

    nodeId = commandObj->GetSubjectDescriptor().subject;

    fabricIndex = commandObj->GetAccessingFabricIndex();
    ourNodeId  = commandObj->GetExchangeContext()->GetSessionHandle()->AsSecureSession()->GetLocalScopedNodeId();
    vendorId                                                = commandData.vendorID;
    productId                                               = commandData.productID;
    softwareVersion                                         = commandData.softwareVersion;
    protocolsSupported = commandData.protocolsSupported;
    hardwareVersion                               = commandData.hardwareVersion;
    location                                = commandData.location;
    requestorCanConsent                               = commandData.requestorCanConsent;
    metadataForProvider                     = commandData.metadataForProvider;

    {
        auto iterator = commandData.protocolsSupported.begin();
        while (iterator.Next())
        {
            OTADownloadProtocol protocol = iterator.GetValue();
            if (protocol == OTADownloadProtocol::kBDXSynchronous)
            {
                isBDXProtocolSupported = true;
                break;
            }
        }
    }

    // The logic we are following here is if none of the protocols supported by the requestor are supported by us, we
    // can't transfer the image even if we had an image available and we would return a Protocol Not Supported status.
    // Assumption here is the requestor would send us a list of all the protocols it supports. If one/more of the
    // protocols supported by the requestor are supported by us, we check if an image is not available due to various
    // reasons - image not available, delegate reporting busy, we will respond with the status in the delegate response.
    // If update is available, we try to prepare for transfer and build the uri in the response with a status of Image
    // Available

    // If the protocol requested is not supported, return status - Protocol Not Supported
    if (!isBDXProtocolSupported)
    {
        response.status = OTAQueryStatus::kDownloadProtocolNotSupported;
        commandObj->AddResponse(cachedCommandPath, response);
        ExitNow(errorStatus = static_cast<uint8_t>(response.status));
    }

    otaProviderDelegate = mOtaProviderDelegate.ObjectRef();
    err = JniReferences::GetInstance().FindMethod(env, otaProviderDelegate, "handleQueryImage",
                                                  "(IIJLjava/lang/Integer;Ljava/lang/String;Ljava/lang/Boolean;[B)Lchip/"
                                                  "devicecontroller/OTAProviderDelegate$QueryImageResponse;",
                                                  &handleQueryImageMethod);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find handleQueryImage method"));

    if (hardwareVersion.HasValue())
    {
        chip::JniReferences::GetInstance().CreateBoxedObject<jint>(
            "java/lang/Integer", "(I)V", static_cast<jint>(hardwareVersion.Value()), boxedHardwareVersion);
    }

    if (location.HasValue())
    {
        boxedLocation = env->NewStringUTF(std::string(location.Value().data(), location.Value().size()).c_str());
    }

    if (requestorCanConsent.HasValue())
    {
        chip::JniReferences::GetInstance().CreateBoxedObject<jboolean>(
            "java/lang/Boolean", "(Z)V", requestorCanConsent.Value() ? JNI_TRUE : JNI_FALSE, boxedRequestorCanConsent);
    }

    if (metadataForProvider.HasValue())
    {
        jbyteArray boxedMetadataForProviderByteArray = env->NewByteArray(static_cast<jsize>(metadataForProvider.Value().size()));
        env->SetByteArrayRegion(boxedMetadataForProviderByteArray, 0, static_cast<jsize>(metadataForProvider.Value().size()),
                                reinterpret_cast<const jbyte *>(metadataForProvider.Value().data()));
        boxedMetadataForProvider = boxedMetadataForProviderByteArray;
    }

    jResponse =
        env->CallObjectMethod(otaProviderDelegate, handleQueryImageMethod, static_cast<jint>(vendorId),
                              static_cast<jint>(productId), static_cast<jlong>(softwareVersion), boxedHardwareVersion,
                              boxedLocation, boxedRequestorCanConsent, boxedMetadataForProvider);
    if (env->ExceptionCheck())
    {
        ChipLogError(Support, "Exception in call java method");
        env->ExceptionDescribe();
        env->ExceptionClear();
        ExitNow();
    }

    hasUpdate = (jResponse != nullptr);
    // If update is not available, return the delegate response
    if (!hasUpdate)
    {
        response.status = OTAQueryStatus::kNotAvailable;
        commandObj->AddResponse(cachedCommandPath, response);
        sendOTAQueryFailure(static_cast<uint8_t>(response.status));
        return;
    }

    err = JniReferences::GetInstance().FindMethod(env, jResponse, "getSoftwareVersion", "()Ljava/lang/Long;",
                                                  &getSoftwareVersionMethod);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find getSoftwareVersion method"));


    err = JniReferences::GetInstance().FindMethod(env, jResponse, "getSoftwareVersionString", "()Ljava/lang/String;",
                                                  &getSoftwareVersionStringMethod);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find getSoftwareVersionString method"));


    err = JniReferences::GetInstance().FindMethod(env, jResponse, "getFilePath", "()Ljava/lang/String;", &getFilePathMethod);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find getFilePath method"));

    jSoftwareVersion       = env->CallObjectMethod(jResponse, getSoftwareVersionMethod);
    jSoftwareVersionString = (jstring) env->CallObjectMethod(jResponse, getSoftwareVersionStringMethod);
    jFilePath              = (jstring) env->CallObjectMethod(jResponse, getFilePathMethod);

    {
        response.status = OTAQueryStatus::kUpdateAvailable;
        if (jSoftwareVersion != nullptr)
        {
            response.softwareVersion.SetValue(static_cast<uint32_t>(JniReferences::GetInstance().LongToPrimitive(jSoftwareVersion)));
        }

        JniUtfString jniSoftwareVersionString(env, jSoftwareVersionString);

        if (jniSoftwareVersionString.c_str() != nullptr)
        {
            response.softwareVersionString.SetValue(jniSoftwareVersionString.charSpan());
        }

        JniUtfString jniFilePath(env, jFilePath);

        GenerateUpdateToken(mToken, kUpdateTokenLen);

        response.updateToken.SetValue(chip::ByteSpan(mToken, kUpdateTokenLen));
        response.userConsentNeeded.SetValue(0);

        err = mBdxOTASender->PrepareForTransfer(fabricIndex, nodeId);
        if (CHIP_NO_ERROR != err)
        {
            // Handle busy error separately as we have a query image response status that maps to busy
            if (err == CHIP_ERROR_BUSY)
            {
                ChipLogError(Controller, "Responding with Busy due to being in the middle of handling another BDX transfer");
                Commands::QueryImageResponse::Type errorResponse;
                errorResponse.status = OTAQueryStatus::kBusy;
                errorResponse.delayedActionTime.SetValue(kDelayedActionTimeSeconds);
                commandObj->AddResponse(cachedCommandPath, errorResponse);
                // We do not reset state when we get the busy error because that means we are locked in a BDX transfer
                // session with another requestor when we get this query image request. We do not want to interrupt the
                // ongoing transfer instead just respond to the second requestor with a busy status and a delayedActionTime
                // in which the requestor can retry.
                ExitNow(errorStatus = static_cast<uint8_t>(errorResponse.status));
            }
            LogErrorOnFailure(err);
            commandObj->AddStatus(cachedCommandPath, StatusIB(err).mStatus);
            // We need to reset state here to clean up any initialization we might have done including starting the BDX
            // timeout timer while preparing for transfer if any failure occurs afterwards.
            mBdxOTASender->ResetState();
            ExitNow(errorStatus = static_cast<uint8_t>(OTAQueryStatus::kNotAvailable));
        }
        err = bdx::MakeURI(ourNodeId.GetNodeId(), jniFilePath.c_str() != nullptr ? jniFilePath.charSpan() : CharSpan(), uri);
        if (CHIP_NO_ERROR != err)
        {
            LogErrorOnFailure(err);
            commandObj->AddStatus(cachedCommandPath, StatusIB(err).mStatus);
            mBdxOTASender->ResetState();
            ExitNow(errorStatus = static_cast<uint8_t>(OTAQueryStatus::kNotAvailable));
        }
        response.imageURI.SetValue(uri);
        commandObj->AddResponse(cachedCommandPath, response);
    }
    return;

exit:
    ChipLogError(Controller, "OTA Query Failure : %u, %" CHIP_ERROR_FORMAT, errorStatus, err.Format());
    sendOTAQueryFailure(errorStatus);
}

void OTAProviderDelegateBridge::HandleApplyUpdateRequest(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                         const ApplyUpdateRequest::DecodableType & commandData)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturn(mOtaProviderDelegate.HasValidObjectRef(),
                   commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Failure));

    NodeId nodeId = commandObj->GetSubjectDescriptor().subject;

    ConcreteCommandPath cachedCommandPath(commandPath.mEndpointId, commandPath.mClusterId, commandPath.mCommandId);

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    jobject otaProviderDelegate = mOtaProviderDelegate.ObjectRef();
    chip::JniLocalReferenceManager manager(env);
    jmethodID handleApplyUpdateRequestMethod;
    CHIP_ERROR err = JniReferences::GetInstance().FindMethod(env, otaProviderDelegate, "handleApplyUpdateRequest",
                                                             "(JJ)Lchip/devicecontroller/OTAProviderDelegate$ApplyUpdateResponse;",
                                                             &handleApplyUpdateRequestMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find handleApplyUpdateRequest method"));

    jobject jResponse = env->CallObjectMethod(otaProviderDelegate, handleApplyUpdateRequestMethod, static_cast<jlong>(nodeId),
                                              static_cast<jlong>(commandData.newVersion));
    if (env->ExceptionCheck())
    {
        ChipLogError(Support, "Exception in call java method");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }

    jmethodID getActionMethod;
    err = JniReferences::GetInstance().FindMethod(env, jResponse, "getAction", "()I", &getActionMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find getAction method"));

    jmethodID getDelayedActionTimeMethod;
    err = JniReferences::GetInstance().FindMethod(env, jResponse, "getDelayedActionTime", "()J", &getDelayedActionTimeMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find getDelayedActionTime method"));

    jint jAction             = env->CallIntMethod(jResponse, getActionMethod);
    jlong jDelayedActionTime = env->CallLongMethod(jResponse, getDelayedActionTimeMethod);

    Commands::ApplyUpdateResponse::Type response;
    response.action            = static_cast<OTAApplyUpdateAction>(jAction);
    response.delayedActionTime = static_cast<uint32_t>(jDelayedActionTime);
    commandObj->AddResponse(cachedCommandPath, response);
}

void OTAProviderDelegateBridge::HandleNotifyUpdateApplied(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                          const NotifyUpdateApplied::DecodableType & commandData)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturn(mOtaProviderDelegate.HasValidObjectRef(),
                   commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Failure));

    NodeId nodeId = commandObj->GetSubjectDescriptor().subject;

    ConcreteCommandPath cachedCommandPath(commandPath.mEndpointId, commandPath.mClusterId, commandPath.mCommandId);

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    jobject otaProviderDelegate = mOtaProviderDelegate.ObjectRef();
    jmethodID handleNotifyUpdateAppliedMethod;
    CHIP_ERROR err = JniReferences::GetInstance().FindMethod(env, otaProviderDelegate, "handleNotifyUpdateApplied", "(J)V",
                                                             &handleNotifyUpdateAppliedMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find handleNotifyUpdateApplied method"));

    env->CallVoidMethod(otaProviderDelegate, handleNotifyUpdateAppliedMethod, static_cast<jlong>(nodeId));
    if (env->ExceptionCheck())
    {
        ChipLogError(Support, "Exception in call java method");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }

    commandObj->AddStatus(cachedCommandPath, Protocols::InteractionModel::Status::Success);
}
