#include "Draft.h"
#include <app/util/basic-types.h>
#include <controller/CHIPDeviceControllerFactory.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include "../config/PersistentStorage.h"

#include <controller/CHIPDeviceControllerFactory.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/DeviceAttestationVerifier.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <credentials/examples/DeviceAttestationVerifierExample.h>
#include "controller/ExampleOperationalCredentialsIssuer.h"


using namespace chip;
using namespace chip::Controller;


struct Context
{
    DeviceCommissioner controller;
    PersistentStorage storage;
    chip::Controller::ExampleOperationalCredentialsIssuer mOpCredsIssuer;
    chip::NodeId mNodeId = 1;
    uint8_t mEndPointId = 1;
};


// struct ExecutionContext
// {
//     ChipDeviceCommissioner * controller;
//     ExampleOperationalCredentialsIssuer * opCredsIssuer;
//     PersistentStorage * storage;
//     chip::NodeId localId;
//     chip::NodeId remoteId;
// };



CHIP_ERROR Draft::Run()
{
    chip::Logging::Log(chip::Logging::kLogModule_Shell, chip::Logging::kLogCategory_Progress, "Draft::Run");

    Context ctx;
    uint32_t pin_code = 73141520;
    uint16_t discriminator = 3840;

    CHIP_ERROR err = Setup(ctx);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "---- Setup Failed"));

    err = Start(*this, ctx);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "---- Run Failed"));

    err = Pair(ctx, 1111, PeerAddress::BLE(), pin_code, discriminator);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "---- Pair Failed"));

exit:
    return err;
}


CHIP_ERROR Draft::Setup(Context &ctx)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = chip::Platform::MemoryInit();
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init Memory failure: %s", chip::ErrorStr(err)));

    err = ctx.storage.Init();
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init Storage failure: %s", chip::ErrorStr(err)));

exit:
    return err;
}


CHIP_ERROR Draft::Start(Context &ctx)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ReturnLogErrorOnFailure(chip::DeviceLayer::Internal::BLEMgrImpl().ConfigureBle(0, true));
    ReturnLogErrorOnFailure(ctx.mOpCredsIssuer.Initialize(ctx.storage));

    chip::Platform::ScopedMemoryBuffer<uint8_t> noc;
    chip::Platform::ScopedMemoryBuffer<uint8_t> icac;
    chip::Platform::ScopedMemoryBuffer<uint8_t> rcac;

    chip::Credentials::SetDeviceAttestationCredentialsProvider(chip::Credentials::Examples::GetExampleDACProvider());
    chip::Credentials::SetDeviceAttestationVerifier(chip::Credentials::Examples::GetExampleDACVerifier());

    VerifyOrReturnError(noc.Alloc(chip::Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    VerifyOrReturnError(icac.Alloc(chip::Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    VerifyOrReturnError(rcac.Alloc(chip::Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);

    chip::MutableByteSpan nocSpan(noc.Get(), chip::Controller::kMaxCHIPDERCertLength);
    chip::MutableByteSpan icacSpan(icac.Get(), chip::Controller::kMaxCHIPDERCertLength);
    chip::MutableByteSpan rcacSpan(rcac.Get(), chip::Controller::kMaxCHIPDERCertLength);

    chip::Crypto::P256Keypair ephemeralKey;
    ReturnLogErrorOnFailure(ephemeralKey.Initialize());

    // TODO - OpCreds should only be generated for pairing command
    //        store the credentials in persistent storage, and
    //        generate when not available in the storage.
    ReturnLogErrorOnFailure(ctx.mOpCredsIssuer.GenerateNOCChainAfterValidation(ctx.storage.GetLocalNodeId(), 0, ephemeralKey.Pubkey(),
                                                                           rcacSpan, icacSpan, nocSpan));

    chip::Controller::FactoryInitParams factoryInitParams;
    factoryInitParams.storageDelegate = &ctx.storage;
    factoryInitParams.listenPort      = ctx.storage.GetListenPort();

    chip::Controller::SetupParams commissionerParams;
    commissionerParams.operationalCredentialsDelegate = &ctx.mOpCredsIssuer;
    commissionerParams.ephemeralKeypair               = &ephemeralKey;
    commissionerParams.controllerRCAC                 = rcacSpan;
    commissionerParams.controllerICAC                 = icacSpan;
    commissionerParams.controllerNOC                  = nocSpan;

    ReturnLogErrorOnFailure(DeviceControllerFactory::GetInstance().Init(factoryInitParams));
    ReturnLogErrorOnFailure(DeviceControllerFactory::GetInstance().SetupCommissioner(commissionerParams, ctx.controller));

    ChipLogProgress(chipTool, "Sending command to node 0x%" PRIx64, ctx.mNodeId);

    err = ctx.controller.GetConnectedDevice(ctx.mNodeId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(chipTool, "Failed in initiating connection to the device: %" PRIu64 ", error %" CHIP_ERROR_FORMAT,
                              ctx.mNodeId, err.Format()));

    // ReturnLogErrorOnFailure(StartWaiting(GetWaitDurationInSeconds()));
    // Shutdown();

    //
    // We can call DeviceController::Shutdown() safely without grabbing the stack lock
    // since the CHIP thread and event queue have been stopped, preventing any thread
    // races.
    //
    // ReturnLogErrorOnFailure(mController.Shutdown());
exit:
    return CHIP_NO_ERROR;
}


CHIP_ERROR Draft::Pair(Context &ctx, NodeId remote_id, Transport::PeerAddress peer_addr, uint32_t pin_code, uint16_t discriminator)
{
    RendezvousParameters params = RendezvousParameters().SetSetupPINCode(pin_code).SetDiscriminator(discriminator).SetPeerAddress(peer_addr);

    chip::Logging::Log(chip::Logging::kLogModule_Shell, chip::Logging::kLogCategory_Progress, "---- PairingCommand::Pair.1");
    CHIP_ERROR err = ctx.controller.PairDevice(remote_id, params);
    chip::Logging::Log(chip::Logging::kLogModule_Shell, chip::Logging::kLogCategory_Progress, "---- PairingCommand::Pair.2, err:%u", err.AsInteger());

    return err;
}



void Draft::OnDeviceConnectedFn(void * context, chip::Controller::Device * device)
{
    chip::Logging::Log(chip::Logging::kLogModule_Shell, chip::Logging::kLogCategory_Progress, "---- Draft::OnDeviceConnectedFn");

}

void Draft::OnDeviceConnectionFailureFn(void * context, NodeId deviceId, CHIP_ERROR error)
{
    chip::Logging::Log(chip::Logging::kLogModule_Shell, chip::Logging::kLogCategory_Progress, "---- Draft::OnDeviceConnectionFailureFn");

}



// Callback::Callback<OnDeviceConnected> *
// void (void *, chip::Controller::Device *)