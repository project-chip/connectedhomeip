/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 *  @file
 *    This file contains the definition for CHIP's Operational Credentials
 *    cluster class.
 */

#pragma once

#include <controller/CHIPCluster.h>
#include <core/CHIPCallback.h>
#include <lib/support/Span.h>

namespace chip {
namespace Controller {

typedef void (*OperationalCredentialsClusterOpCSRResponseCallback)(void * context, chip::ByteSpan CSR, chip::ByteSpan CSRNonce,
                                                                   chip::ByteSpan VendorReserved1, chip::ByteSpan VendorReserved2,
                                                                   chip::ByteSpan VendorReserved3, chip::ByteSpan Signature);
typedef void (*OperationalCredentialsClusterOpCertResponseCallback)(void * context, uint8_t StatusCode, uint64_t FabricIndex,
                                                                    uint8_t * DebugText);

typedef void (*DefaultSuccessCallback)(void * context);
typedef void (*DefaultFailureCallback)(void * context, uint8_t status);

constexpr ClusterId kOperationalCredentialsClusterIdLocal  = 0x003E;
constexpr ClusterId kTrustedRootCertificatesClusterIdLocal = 0x003F;

class DLL_EXPORT OperationalCredentialsProvisioner : public ClusterBase
{
public:
    OperationalCredentialsProvisioner() : ClusterBase(kOperationalCredentialsClusterIdLocal) {}
    ~OperationalCredentialsProvisioner() {}

    // Cluster Commands
    CHIP_ERROR AddOpCert(Callback::Cancelable * onSuccessCallback, Callback::Cancelable * onFailureCallback, chip::ByteSpan noc,
                         chip::ByteSpan iCACertificate, chip::ByteSpan iPKValue, chip::NodeId caseAdminNode,
                         uint16_t adminVendorId);
    CHIP_ERROR OpCSRRequest(Callback::Cancelable * onSuccessCallback, Callback::Cancelable * onFailureCallback,
                            chip::ByteSpan cSRNonce);
    CHIP_ERROR RemoveAllFabrics(Callback::Cancelable * onSuccessCallback, Callback::Cancelable * onFailureCallback);
    CHIP_ERROR RemoveFabric(Callback::Cancelable * onSuccessCallback, Callback::Cancelable * onFailureCallback,
                            chip::FabricId fabricId, chip::NodeId nodeId, uint16_t vendorId);
    CHIP_ERROR SetFabric(Callback::Cancelable * onSuccessCallback, Callback::Cancelable * onFailureCallback, uint16_t vendorId);
    CHIP_ERROR UpdateFabricLabel(Callback::Cancelable * onSuccessCallback, Callback::Cancelable * onFailureCallback,
                                 chip::ByteSpan label);

    // Cluster Attributes
    CHIP_ERROR DiscoverAttributes(Callback::Cancelable * onSuccessCallback, Callback::Cancelable * onFailureCallback);
    CHIP_ERROR ReadAttributeFabricsList(Callback::Cancelable * onSuccessCallback, Callback::Cancelable * onFailureCallback);
    CHIP_ERROR ReadAttributeClusterRevision(Callback::Cancelable * onSuccessCallback, Callback::Cancelable * onFailureCallback);

private:
    static constexpr CommandId kAddOpCertCommandId         = 0x06;
    static constexpr CommandId kOpCSRRequestCommandId      = 0x04;
    static constexpr CommandId kRemoveAllFabricsCommandId  = 0x0B;
    static constexpr CommandId kRemoveFabricCommandId      = 0x0A;
    static constexpr CommandId kSetFabricCommandId         = 0x00;
    static constexpr CommandId kUpdateFabricLabelCommandId = 0x09;
};

// TODO - Remove TrustedRootCertificate cluster once it merges with OperationalCredentials cluster
// Keeping the code in this file for time being, as it'll eventually be removed.
class DLL_EXPORT TrustedRootCertificatesProvisioner : public ClusterBase
{
public:
    TrustedRootCertificatesProvisioner() : ClusterBase(kTrustedRootCertificatesClusterIdLocal) {}
    ~TrustedRootCertificatesProvisioner() {}

    // Cluster Commands
    CHIP_ERROR AddTrustedRootCertificate(Callback::Cancelable * onSuccessCallback, Callback::Cancelable * onFailureCallback,
                                         chip::ByteSpan rootCertificate);
    CHIP_ERROR RemoveTrustedRootCertificate(Callback::Cancelable * onSuccessCallback, Callback::Cancelable * onFailureCallback,
                                            chip::ByteSpan trustedRootIdentifier);

    // Cluster Attributes
    CHIP_ERROR DiscoverAttributes(Callback::Cancelable * onSuccessCallback, Callback::Cancelable * onFailureCallback);
    CHIP_ERROR ReadAttributeClusterRevision(Callback::Cancelable * onSuccessCallback, Callback::Cancelable * onFailureCallback);

private:
    static constexpr CommandId kAddTrustedRootCertificateCommandId    = 0x00;
    static constexpr CommandId kRemoveTrustedRootCertificateCommandId = 0x01;
};

} // namespace Controller
} // namespace chip
