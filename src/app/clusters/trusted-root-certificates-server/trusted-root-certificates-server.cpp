/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/****************************************************************************
 * @file
 * @brief Implementation of Trusted Root Certificates Cluster
 ***************************************************************************/

#include <app/Command.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <lib/core/PeerId.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <transport/AdminPairingTable.h>

#include "gen/af-structs.h"
#include "gen/attribute-id.h"
#include "gen/attribute-type.h"
#include "gen/cluster-id.h"
#include "gen/command-id.h"

using namespace chip;
using namespace ::chip::Transport;

static AdminPairingInfo * retrieveCurrentAdmin()
{
    uint64_t fabricId = emberAfCurrentCommand()->SourceNodeId();
    return GetGlobalAdminPairingTable().FindAdminForNode(fabricId);
}

bool emberAfTrustedRootCertificatesClusterAddTrustedRootCertificateCallback(chip::app::Command * commandObj,
                                                                            chip::ByteSpan RootCertificate)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: commissioner has added a trusted root Cert");

    // Fetch current admin
    AdminPairingInfo * admin = retrieveCurrentAdmin();
    VerifyOrExit(admin != nullptr, status = EMBER_ZCL_STATUS_FAILURE);
    VerifyOrExit(admin->SetRootCert(RootCertificate) == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_FAILURE);

    VerifyOrExit(GetGlobalAdminPairingTable().Store(admin->GetAdminId()) == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_FAILURE);

exit:
    emberAfSendImmediateDefaultResponse(status);
    if (status == EMBER_ZCL_STATUS_FAILURE)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed AddTrustedRootCert request.");
    }

    return true;
}

bool emberAfTrustedRootCertificatesClusterRemoveTrustedRootCertificateCallback(chip::app::Command * commandObj,
                                                                               chip::ByteSpan TrustedRootIdentifier)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
