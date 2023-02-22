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

#pragma once

#include <controller/python/chip/crypto/p256keypair.h>
#include <lib/core/CASEAuthTag.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPMem.h>

struct pychip_OpCredsContext;

/**
 * Parameters for allocating a CHIP Commissioner.
 *
 * Note: The difference between a controller and a commissioner is the commissioner contains a CA which allows it to sign NOC by
 * itself. A controller can be used to commission other devices with the help of externally implemented CA.
 */
struct pychip_OpCreds_AllocateCommissionerParams
{
    pychip_OpCredsContext * context;
    chip::FabricId fabricId;
    chip::NodeId nodeId;
    chip::VendorId adminVendorId;
    const char * paaTrustStorePath;
    bool useTestCommissioner;
    bool enableServerInteractions;
    chip::CASEAuthTag * caseAuthTags;
    uint32_t caseAuthTagLen;
    chip::python::pychip_P256Keypair * operationalKey;
};

/**
 * Parameters for allocating a CHIP Controller.
 *
 * Note: The difference between a controller and a commissioner is the commissioner contains a CA which allows it to sign NOC by
 * itself. A controller can be used to commission other devices with the help of externally implemented CA.
 *
 * To initialize a controller, an NOC and a RCAC must be provided, and an optional ICAC may be provided. The NodeId and the FabricId
 * will be extracted from the provided NOC.
 */
struct pychip_OpCreds_AllocateControllerParams
{
    chip::VendorId adminVendorId;
    bool enableServerInteractions;
    chip::python::pychip_P256Keypair * operationalKey;

    const uint8_t * noc;
    uint32_t nocLen;
    const uint8_t * icac;
    uint32_t icacLen;
    const uint8_t * rcac;
    uint32_t rcacLen;
    const uint8_t * ipk;
    uint32_t ipkLen;
};
