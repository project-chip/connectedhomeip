/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

// NXP implementation of the Joint Fabric Administrator platform hooks declared
// in JFAPlatform.h. This brings up a DeviceCommissioner that reuses the running
// Server's fabric table and persistent storage, adapted from the Linux
// CommissionerMain.cpp reference so that no Linux example-platform code is
// required.

#include "JFAPlatform.h"

#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <credentials/GroupDataProvider.h>
#include <credentials/TestOnlyLocalCertificateAuthority.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <crypto/PSASessionKeystore.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/ScopedMemoryBuffer.h>
#include <lib/support/TestGroupData.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/KeyValueStoreManager.h>

using namespace chip;
using namespace chip::Controller;
using namespace chip::Credentials;

namespace chip {

namespace {

// Persistent storage delegate backed by the platform KeyValueStore. Used for
// the commissioner's fabric-independent storage.
class JFAServerStorageDelegate : public PersistentStorageDelegate
{
    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override
    {
        size_t bytesRead = 0;
        CHIP_ERROR err   = DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get(key, buffer, size, &bytesRead);
        size             = static_cast<uint16_t>(bytesRead);
        return err;
    }

    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override
    {
        return DeviceLayer::PersistedStorage::KeyValueStoreMgr().Put(key, value, size);
    }

    CHIP_ERROR SyncDeleteKeyValue(const char * key) override
    {
        return DeviceLayer::PersistedStorage::KeyValueStoreMgr().Delete(key);
    }
};

AutoCommissioner gAutoCommissioner;
DeviceCommissioner gCommissioner;
JFAServerStorageDelegate gServerStorage;
ExampleOperationalCredentialsIssuer gOpCredsIssuer;
Crypto::PSASessionKeystore gSessionKeystore;
NodeId gLocalId               = kMaxOperationalNodeId;
bool gCommissionerInitialized = false;

} // namespace

CHIP_ERROR JFAInitCommissioner(FabricId fabricId, FabricIndex commissionerFabricIndex)
{
    VerifyOrReturnError(!gCommissionerInitialized, CHIP_NO_ERROR);

    Controller::FactoryInitParams factoryParams;
    Controller::SetupParams params;

    factoryParams.fabricIndependentStorage = &gServerStorage;
    factoryParams.fabricTable              = &Server::GetInstance().GetFabricTable();
    factoryParams.sessionKeystore          = &gSessionKeystore;
    factoryParams.enableServerInteractions = true;
    // Running alongside the existing Server: do not overwrite its DNS-SD port.
    factoryParams.preventDnssdPortOverwrite = true;
    factoryParams.dataModelProvider         = chip::app::CodegenDataModelProviderInstance(nullptr);

    // Reuse the server's already-initialized GroupDataProvider to avoid a
    // second PSA key derivation that fails on embedded targets.
    factoryParams.groupDataProvider = chip::Credentials::GetGroupDataProvider();

    params.operationalCredentialsDelegate = &gOpCredsIssuer;

    uint16_t vendorId = 0;
    ReturnErrorOnFailure(DeviceLayer::GetDeviceInstanceInfoProvider()->GetVendorId(vendorId));
    params.controllerVendorId = static_cast<VendorId>(vendorId);

    if (commissionerFabricIndex == kUndefinedFabricIndex && fabricId != kUndefinedFabricId)
    {
        for (const auto & fb : Server::GetInstance().GetFabricTable())
        {
            if (fb.GetFabricId() == fabricId)
            {
                commissionerFabricIndex = fb.GetFabricIndex();
                break;
            }
        }
    }

    const AttestationTrustStore * testingRootStore = GetTestAttestationTrustStore();
    auto * dacVerifier                             = GetDefaultDACVerifier(testingRootStore, nullptr);
    VerifyOrReturnError(dacVerifier != nullptr, CHIP_ERROR_INTERNAL);
    SetDeviceAttestationVerifier(dacVerifier);

    Platform::ScopedMemoryBuffer<uint8_t> noc;
    VerifyOrReturnError(noc.Alloc(Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan nocSpan(noc.Get(), Controller::kMaxCHIPDERCertLength);

    Platform::ScopedMemoryBuffer<uint8_t> icac;
    VerifyOrReturnError(icac.Alloc(Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan icacSpan(icac.Get(), Controller::kMaxCHIPDERCertLength);

    Platform::ScopedMemoryBuffer<uint8_t> rcac;
    VerifyOrReturnError(rcac.Alloc(Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan rcacSpan(rcac.Get(), Controller::kMaxCHIPDERCertLength);
    Crypto::P256Keypair ephemeralKey;

    if (commissionerFabricIndex != kUndefinedFabricIndex)
    {
        params.fabricIndex.SetValue(commissionerFabricIndex);
        params.removeFromFabricTableOnShutdown = false;
        ChipLogProgress(JointFabric, "JFA Commissioner reusing existing fabric index %u",
                        static_cast<unsigned>(commissionerFabricIndex));
    }
    else
    {
        // Only initialize the creds issuer (which generates a PSA key) when we
        // actually need to issue a new NOC for a brand-new fabric.
        ReturnErrorOnFailure(gOpCredsIssuer.Initialize(gServerStorage));

        const FabricId commissionerFabricId = (fabricId == kUndefinedFabricId) ? static_cast<FabricId>(1) : fabricId;
        gOpCredsIssuer.SetFabricIdForNextNOCRequest(commissionerFabricId);

        ReturnErrorOnFailure(ephemeralKey.Initialize(Crypto::ECPKeyTarget::ECDSA));
        ReturnErrorOnFailure(gOpCredsIssuer.GenerateNOCChainAfterValidation(gLocalId, commissionerFabricId, chip::kUndefinedCATs,
                                                                            ephemeralKey.Pubkey(), rcacSpan, icacSpan, nocSpan));
        params.operationalKeypair = &ephemeralKey;
        params.controllerRCAC     = rcacSpan;
        params.controllerICAC     = icacSpan;
        params.controllerNOC      = nocSpan;
    }

    params.defaultCommissioner      = &gAutoCommissioner;
    params.enableServerInteractions = true;

    CommissioningParameters commissioningParams = gAutoCommissioner.GetCommissioningParameters();
    commissioningParams.SetCheckForMatchingFabric(true);
    ReturnErrorOnFailure(gAutoCommissioner.SetCommissioningParameters(commissioningParams));

    auto & factory = Controller::DeviceControllerFactory::GetInstance();
    ReturnErrorOnFailure(factory.Init(factoryParams));
    ReturnErrorOnFailure(factory.SetupCommissioner(params, gCommissioner));

    FabricIndex fabricIndex = gCommissioner.GetFabricIndex();
    VerifyOrReturnError(fabricIndex != kUndefinedFabricIndex, CHIP_ERROR_INTERNAL);

    uint8_t compressedFabricId[sizeof(uint64_t)] = { 0 };
    MutableByteSpan compressedFabricIdSpan(compressedFabricId);
    ReturnErrorOnFailure(gCommissioner.GetCompressedFabricIdBytes(compressedFabricIdSpan));

    ByteSpan defaultIpk = chip::GroupTesting::DefaultIpkValue::GetDefaultIpk();
    ReturnLogErrorOnFailure(chip::Credentials::SetSingleIpkEpochKey(chip::Credentials::GetGroupDataProvider(), fabricIndex,
                                                                    defaultIpk, compressedFabricIdSpan));

    // Advertise operational since we are an admin.
    ReturnLogErrorOnFailure(app::DnssdServer::Instance().AdvertiseOperational());

    gCommissionerInitialized = true;

    ChipLogProgress(JointFabric,
                    "JFA InitCommissioner nodeId=0x" ChipLogFormatX64 " fabricId=0x" ChipLogFormatX64 " fabricIndex=0x%x",
                    ChipLogValueX64(gCommissioner.GetNodeId()), ChipLogValueX64(gCommissioner.GetFabricId()),
                    static_cast<unsigned>(fabricIndex));

    return CHIP_NO_ERROR;
}

Controller::DeviceCommissioner * JFAGetDeviceCommissioner()
{
    return gCommissionerInitialized ? &gCommissioner : nullptr;
}

} // namespace chip
