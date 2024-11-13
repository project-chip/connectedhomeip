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

#include "ICDCommand.h"

#include <app/icd/client/DefaultICDClientStorage.h>
#include <crypto/DefaultSessionKeystore.h>
#include <crypto/RawKeySessionKeystore.h>
#include <string>

using namespace ::chip;
using namespace ::chip::app;

CHIP_ERROR ICDListCommand::RunCommand()
{
    app::ICDClientInfo info;
    auto iter = CHIPCommand::sICDClientStorage.IterateICDClientInfo();
    char icdAesKeyHex[Crypto::kAES_CCM128_Key_Length * 2 + 1];
    char icdHmacKeyHex[Crypto::kHMAC_CCM128_Key_Length * 2 + 1];
    if (iter == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    app::DefaultICDClientStorage::ICDClientInfoIteratorWrapper clientInfoIteratorWrapper(iter);
    fprintf(stderr, "  +------------------------------------------------------------------------------------------+\n");
    fprintf(stderr, "  | %-88s |\n", "Known ICDs:");
    fprintf(stderr, "  +------------------------------------------------------------------------------------------+\n");
    fprintf(stderr, "  | %20s | %20s | %15s | %15s | %16s | %10s |\n", "Fabric Index:Peer Node ID", "Fabric Index:CheckIn Node ID",
            "Start Counter", "Counter Offset", "MonitoredSubject", "ClientType");

    while (iter->Next(info))
    {
        fprintf(stderr, "  +------------------------------------------------------------------------------------------+\n");
        fprintf(stderr,
                "  | %3" PRIu32 ":" ChipLogFormatX64 " | %3" PRIu32 ":" ChipLogFormatX64 " | %15" PRIu32 " | %15" PRIu32
                " | " ChipLogFormatX64 " | %10u |\n",
                static_cast<uint32_t>(info.peer_node.GetFabricIndex()), ChipLogValueX64(info.peer_node.GetNodeId()),
                static_cast<uint32_t>(info.check_in_node.GetFabricIndex()), ChipLogValueX64(info.check_in_node.GetNodeId()),
                info.start_icd_counter, info.offset, ChipLogValueX64(info.monitored_subject),
                static_cast<uint8_t>(info.client_type));

        static_assert(std::is_same<decltype(CHIPCommand::sSessionKeystore), Crypto::RawKeySessionKeystore>::value,
                      "The following BytesToHex can copy/encode the key bytes from sharedKey to hexadecimal format, which only "
                      "works for RawKeySessionKeystore");
        Encoding::BytesToHex(info.aes_key_handle.As<Crypto::Symmetric128BitsKeyByteArray>(), Crypto::kAES_CCM128_Key_Length,
                             icdAesKeyHex, sizeof(icdAesKeyHex), chip::Encoding::HexFlags::kNullTerminate);
        fprintf(stderr, "  | aes key:  %60s                   |\n", icdAesKeyHex);
        Encoding::BytesToHex(info.hmac_key_handle.As<Crypto::Symmetric128BitsKeyByteArray>(), Crypto::kHMAC_CCM128_Key_Length,
                             icdHmacKeyHex, sizeof(icdHmacKeyHex), chip::Encoding::HexFlags::kNullTerminate);
        fprintf(stderr, "  | hmac key: %60s                   |\n", icdHmacKeyHex);
    }

    fprintf(stderr, "  +------------------------------------------------------------------------------------------+\n");
    SetCommandExitStatus(CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ICDWaitForDeviceCommand::RunCommand()
{
    if (!IsPeerLIT())
    {
        ChipLogError(chipTool, "The device is not a registered LIT-ICD device.");
        return CHIP_ERROR_NOT_FOUND;
    }
    mInterestedNode = ScopedNodeId(GetDestinationId(), CurrentCommissioner().GetFabricIndex());
    ChipLogProgress(chipTool, "Please trigger the device active mode.");
    return CHIP_NO_ERROR;
}

void ICDWaitForDeviceCommand::OnCheckInComplete(const chip::app::ICDClientInfo & clientInfo)
{
    DefaultCheckInDelegate::OnCheckInComplete(clientInfo);

    if (clientInfo.peer_node != mInterestedNode)
    {
        ChipLogDetail(chipTool, "The node " ChipLogFormatScopedNodeId " is not the one we are interested in.",
                      ChipLogValueScopedNodeId(clientInfo.peer_node));
        return;
    }

    ChipLogDetail(chipTool, "Received check-in message from the node, send stay active request to the device.");
    mInterestedNode = ScopedNodeId();

    // Intentionally call RunCommand, since it includes all necessary steps for SendCommand.
    CHIP_ERROR err = ClusterCommand::RunCommand();
    if (err != CHIP_NO_ERROR)
    {
        SetCommandExitStatus(err);
        return;
    }
}

CHIP_ERROR ICDWaitForDeviceCommand::SendCommand(DeviceProxy * device,
                                                std::vector<chip::EndpointId> /* not used, always send to endpoint 0 */)
{
    Clusters::IcdManagement::Commands::StayActiveRequest::Type request;
    request.stayActiveDuration = mStayActiveDurationSeconds;
    return ClusterCommand::SendCommand(device, kRootEndpointId, Clusters::IcdManagement::Id,
                                       Clusters::IcdManagement::Commands::StayActiveRequest::Id, request);
}

namespace {
DefaultCheckInDelegate * sCheckInDelegate;
}

void registerCommandsICD(Commands & commands, CredentialIssuerCommands * credsIssuerConfig)
{
    const char * name = "ICD";

    auto icdWaitForDeviceCommand = make_unique<ICDWaitForDeviceCommand>(credsIssuerConfig);

    // This should be safe within chip-tool, since the lifespan of Commands is longer than any commands and the CHIPStack.
    // So this object will not be used after free within chip-tool.
    sCheckInDelegate = static_cast<ICDWaitForDeviceCommand *>(icdWaitForDeviceCommand.get());

    commands_list list = {
        make_unique<ICDListCommand>(credsIssuerConfig),
        std::move(icdWaitForDeviceCommand),
    };

    commands.RegisterCommandSet(name, list, "Commands for client-side ICD management.");
}

DefaultCheckInDelegate * ChipToolCheckInDelegate()
{
    return sCheckInDelegate;
}
