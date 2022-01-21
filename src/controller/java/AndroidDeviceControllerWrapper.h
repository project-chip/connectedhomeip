/*
 *   Copyright (c) 2020-2021 Project CHIP Authors
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
#pragma once

#include <lib/support/JniReferences.h>

#include <memory>

#include <jni.h>

#include <controller/CHIPDeviceController.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <lib/support/TimeUtils.h>
#include <platform/internal/DeviceNetworkInfo.h>

/**
 * This class contains all relevant information for the JNI view of CHIPDeviceController
 * to handle all controller-related processing.
 *
 * Generally it contains the DeviceController class itself, plus any related delegates/callbacks.
 */
class AndroidDeviceControllerWrapper : public chip::Controller::DevicePairingDelegate,
                                       public chip::Controller::OperationalCredentialsDelegate,
                                       public chip::PersistentStorageDelegate,
                                       public chip::FabricStorage
{
public:
    ~AndroidDeviceControllerWrapper();

    chip::Controller::DeviceCommissioner * Controller() { return mController.get(); }
    void SetJavaObjectRef(JavaVM * vm, jobject obj);
    jobject JavaObjectRef() { return mJavaObjectRef; }
    jlong ToJNIHandle();

    void CallJavaMethod(const char * methodName, jint argument);
    CHIP_ERROR InitializeOperationalCredentialsIssuer();

    /**
     * Convert network credentials from Java, and apply them to the commissioning parameters object.
     */
    CHIP_ERROR ApplyNetworkCredentials(chip::Controller::CommissioningParameters & params, jobject networkCredentials);

    // DevicePairingDelegate implementation
    void OnStatusUpdate(chip::Controller::DevicePairingDelegate::Status status) override;
    void OnPairingComplete(CHIP_ERROR error) override;
    void OnPairingDeleted(CHIP_ERROR error) override;
    void OnCommissioningComplete(chip::NodeId deviceId, CHIP_ERROR error) override;

    // OperationalCredentialsDelegate implementation
    CHIP_ERROR GenerateNOCChain(const chip::ByteSpan & csrElements, const chip::ByteSpan & attestationSignature,
                                const chip::ByteSpan & DAC, const chip::ByteSpan & PAI, const chip::ByteSpan & PAA,
                                chip::Callback::Callback<chip::Controller::OnNOCChainGeneration> * onCompletion) override;

    void SetNodeIdForNextNOCRequest(chip::NodeId nodeId) override
    {
        mNextRequestedNodeId = nodeId;
        mNodeIdRequested     = true;
    }

    void SetFabricIdForNextNOCRequest(chip::FabricId fabricId) override { mNextFabricId = fabricId; }

    // PersistentStorageDelegate implementation
    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override;
    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override;
    CHIP_ERROR SyncDeleteKeyValue(const char * key) override;

    // FabricStorage implementation
    CHIP_ERROR SyncStore(chip::FabricIndex fabricIndex, const char * key, const void * buffer, uint16_t size) override;
    CHIP_ERROR SyncLoad(chip::FabricIndex fabricIndex, const char * key, void * buffer, uint16_t & size) override;
    CHIP_ERROR SyncDelete(chip::FabricIndex fabricIndex, const char * key) override;

    static AndroidDeviceControllerWrapper * FromJNIHandle(jlong handle)
    {
        return reinterpret_cast<AndroidDeviceControllerWrapper *>(handle);
    }

    static AndroidDeviceControllerWrapper * AllocateNew(JavaVM * vm, jobject deviceControllerObj, chip::NodeId nodeId,
                                                        chip::System::Layer * systemLayer,
                                                        chip::Inet::EndPointManager<chip::Inet::TCPEndPoint> * tcpEndPointManager,
                                                        chip::Inet::EndPointManager<chip::Inet::UDPEndPoint> * udpEndPointManager,
                                                        CHIP_ERROR * errInfoOnFailure);

    CHIP_ERROR GenerateNOCChainAfterValidation(chip::NodeId nodeId, chip::FabricId fabricId,
                                               const chip::Crypto::P256PublicKey & ephemeralKey, chip::MutableByteSpan & rcac,
                                               chip::MutableByteSpan & icac, chip::MutableByteSpan & noc);

private:
    using ChipDeviceControllerPtr = std::unique_ptr<chip::Controller::DeviceCommissioner>;
    chip::Crypto::P256Keypair mIssuer;
    bool mInitialized  = false;
    uint32_t mIssuerId = 0;
    uint32_t mNow      = 0;
    uint32_t mValidity = 10 * chip::kSecondsPerStandardYear;

    ChipDeviceControllerPtr mController;
    chip::Controller::ExampleOperationalCredentialsIssuer mOpCredsIssuer;

    JavaVM * mJavaVM       = nullptr;
    jobject mJavaObjectRef = nullptr;

    chip::NodeId mNextAvailableNodeId = 1;

    chip::NodeId mNextRequestedNodeId = 1;
    chip::FabricId mNextFabricId      = 0;
    bool mNodeIdRequested             = false;

    // These fields allow us to release the string/byte array memory later.
    jstring ssidStr                    = nullptr;
    jstring passwordStr                = nullptr;
    const char * ssid                  = nullptr;
    const char * password              = nullptr;
    jbyteArray operationalDatasetBytes = nullptr;
    jbyte * operationalDataset         = nullptr;

    AndroidDeviceControllerWrapper(ChipDeviceControllerPtr controller) : mController(std::move(controller))
    {
        chip::CalendarToChipEpochTime(2021, 06, 10, 0, 0, 0, mNow);
    }
};

inline jlong AndroidDeviceControllerWrapper::ToJNIHandle()
{
    static_assert(sizeof(jlong) >= sizeof(void *), "Need to store a pointer in a java handle");
    return reinterpret_cast<jlong>(this);
}
