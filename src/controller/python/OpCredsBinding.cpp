/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2019-2020 Google LLC.
 *    Copyright (c) 2013-2018 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <type_traits>

#include "ChipDeviceController-ScriptDeviceAddressUpdateDelegate.h"
#include "ChipDeviceController-ScriptDevicePairingDelegate.h"
#include "ChipDeviceController-StorageDelegate.h"

#include "controller/python/chip/interaction_model/Delegate.h"

#include <controller/CHIPDeviceController.h>
#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/logging/CHIPLogging.h>

#include <credentials/DeviceAttestationVerifier.h>
#include <credentials/examples/DefaultDeviceAttestationVerifier.h>

using namespace chip;

static_assert(std::is_same<uint32_t, ChipError::StorageType>::value, "python assumes CHIP_ERROR maps to c_uint32");

using Py_GenerateNOCChainFunc         = void (*)(void * pyContext, const char * csrElements, const char * attestationSignature,
                                         const char * dac, const char * pai, const char * paa,
                                         Controller::OnNOCChainGeneration onNocChainGenerationFunc);
using Py_SetNodeIdForNextNOCRequest   = void (*)(void * pyContext, NodeId nodeId);
using Py_SetFabricIdForNextNOCRequest = void (*)(void * pyContext, FabricId fabricId);

namespace chip {
namespace Controller {
namespace Python {

class OperationalCredentialsAdapter : public OperationalCredentialsDelegate
{
public:
    OperationalCredentialsAdapter(uint32_t fabricCredentialsIndex) : mExampleOpCredsIssuer(fabricCredentialsIndex) {}

    CHIP_ERROR Initialize(PersistentStorageDelegate & storageDelegate) { return mExampleOpCredsIssuer.Initialize(storageDelegate); }

    CHIP_ERROR GenerateNOCChain(NodeId nodeId, FabricId fabricId, const Crypto::P256PublicKey & pubKey, MutableByteSpan & rcac,
                                MutableByteSpan & icac, MutableByteSpan & noc)
    {
        return mExampleOpCredsIssuer.GenerateNOCChainAfterValidation(nodeId, fabricId, pubKey, rcac, icac, noc);
    }

private:
    CHIP_ERROR GenerateNOCChain(const ByteSpan & csrElements, const ByteSpan & attestationSignature, const ByteSpan & DAC,
                                const ByteSpan & PAI, const ByteSpan & PAA,
                                Callback::Callback<OnNOCChainGeneration> * onCompletion) override
    {
        return mExampleOpCredsIssuer.GenerateNOCChain(csrElements, attestationSignature, DAC, PAI, PAA, onCompletion);
    }

    void SetNodeIdForNextNOCRequest(NodeId nodeId) override { mExampleOpCredsIssuer.SetNodeIdForNextNOCRequest(nodeId); }

    void SetFabricIdForNextNOCRequest(FabricId fabricId) override { mExampleOpCredsIssuer.SetFabricIdForNextNOCRequest(fabricId); }

    ExampleOperationalCredentialsIssuer mExampleOpCredsIssuer;
};

} // namespace Python
} // namespace Controller
} // namespace chip

extern chip::Controller::Python::StorageAdapter * pychip_Storage_GetStorageAdapter();
extern chip::Controller::Python::StorageAdapter * sStorageAdapter;
extern chip::Controller::ScriptDeviceAddressUpdateDelegate sDeviceAddressUpdateDelegate;
extern chip::Controller::ScriptDevicePairingDelegate sPairingDelegate;
bool sTestCommissionerUsed = false;
class TestCommissioner : public chip::Controller::AutoCommissioner
{
public:
    TestCommissioner() : AutoCommissioner() {}
    ~TestCommissioner() {}
    CHIP_ERROR CommissioningStepFinished(CHIP_ERROR err,
                                         chip::Controller::CommissioningDelegate::CommissioningReport report) override
    {
        sTestCommissionerUsed = true;
        return chip::Controller::AutoCommissioner::CommissioningStepFinished(err, report);
    }
};
TestCommissioner sTestCommissioner;

extern "C" {
struct OpCredsContext
{
    Platform::UniquePtr<Controller::Python::OperationalCredentialsAdapter> mAdapter;
    void * mPyContext;
};

void * pychip_OpCreds_InitializeDelegate(void * pyContext, uint32_t fabricCredentialsIndex)
{
    auto context      = Platform::MakeUnique<OpCredsContext>();
    context->mAdapter = Platform::MakeUnique<Controller::Python::OperationalCredentialsAdapter>(fabricCredentialsIndex);

    if (pychip_Storage_GetStorageAdapter() == nullptr)
    {
        return nullptr;
    }

    if (context->mAdapter->Initialize(*pychip_Storage_GetStorageAdapter()) != CHIP_NO_ERROR)
    {
        return nullptr;
    }

    return context.release();
}

ChipError::StorageType pychip_OpCreds_AllocateController(OpCredsContext * context,
                                                         chip::Controller::DeviceCommissioner ** outDevCtrl, uint8_t fabricIndex,
                                                         FabricId fabricId, chip::NodeId nodeId, bool useTestCommissioner)
{
    ChipLogDetail(Controller, "Creating New Device Controller");

    VerifyOrReturnError(context != nullptr, CHIP_ERROR_INVALID_ARGUMENT.AsInteger());

    auto devCtrl = std::make_unique<chip::Controller::DeviceCommissioner>();
    VerifyOrReturnError(devCtrl != nullptr, CHIP_ERROR_NO_MEMORY.AsInteger());

    // Initialize device attestation verifier
    // TODO: Replace testingRootStore with a AttestationTrustStore that has the necessary official PAA roots available
    const chip::Credentials::AttestationTrustStore * testingRootStore = chip::Credentials::GetTestAttestationTrustStore();
    SetDeviceAttestationVerifier(GetDefaultDACVerifier(testingRootStore));

    chip::Crypto::P256Keypair ephemeralKey;
    CHIP_ERROR err = ephemeralKey.Initialize();
    VerifyOrReturnError(err == CHIP_NO_ERROR, err.AsInteger());

    chip::Platform::ScopedMemoryBuffer<uint8_t> noc;
    ReturnErrorCodeIf(!noc.Alloc(Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY.AsInteger());
    MutableByteSpan nocSpan(noc.Get(), Controller::kMaxCHIPDERCertLength);

    chip::Platform::ScopedMemoryBuffer<uint8_t> icac;
    ReturnErrorCodeIf(!icac.Alloc(Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY.AsInteger());
    MutableByteSpan icacSpan(icac.Get(), Controller::kMaxCHIPDERCertLength);

    chip::Platform::ScopedMemoryBuffer<uint8_t> rcac;
    ReturnErrorCodeIf(!rcac.Alloc(Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY.AsInteger());
    MutableByteSpan rcacSpan(rcac.Get(), Controller::kMaxCHIPDERCertLength);

    err = context->mAdapter->GenerateNOCChain(nodeId, fabricId, ephemeralKey.Pubkey(), rcacSpan, icacSpan, nocSpan);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err.AsInteger());

    Controller::SetupParams initParams;
    initParams.storageDelegate                = sStorageAdapter;
    initParams.deviceAddressUpdateDelegate    = &sDeviceAddressUpdateDelegate;
    initParams.pairingDelegate                = &sPairingDelegate;
    initParams.operationalCredentialsDelegate = context->mAdapter.get();
    initParams.operationalKeypair             = &ephemeralKey;
    initParams.controllerRCAC                 = rcacSpan;
    initParams.controllerICAC                 = icacSpan;
    initParams.controllerNOC                  = nocSpan;
    if (useTestCommissioner)
    {
        initParams.defaultCommissioner = &sTestCommissioner;
    }

    err = Controller::DeviceControllerFactory::GetInstance().SetupCommissioner(initParams, *devCtrl);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err.AsInteger());

    *outDevCtrl = devCtrl.release();

    return CHIP_NO_ERROR.AsInteger();
}

void pychip_OpCreds_FreeDelegate(OpCredsContext * context)
{
    Platform::Delete(context);
}

ChipError::StorageType pychip_DeviceController_DeleteDeviceController(chip::Controller::DeviceCommissioner * devCtrl)
{
    if (devCtrl != NULL)
    {
        devCtrl->Shutdown();
        delete devCtrl;
    }

    return CHIP_NO_ERROR.AsInteger();
}

bool pychip_TestCommissionerUsed()
{
    return sTestCommissionerUsed;
}

} // extern "C"
