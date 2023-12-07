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

#include <crypto/DefaultSessionKeystore.h>
#include <crypto/RawKeySessionKeystore.h>

using namespace ::chip;

CHIP_ERROR ICDListCommand::RunCommand()
{
    app::ICDClientInfo info;
    auto iter = GetICDClientStorage().IterateICDClientInfo();
    char icdSymmetricKeyHex[Crypto::kAES_CCM128_Key_Length * 2 + 1];

    fprintf(stderr, "  +-----------------------------------------------------------------------------+\n");
    fprintf(stderr, "  | %-75s |\n", "Known ICDs:");
    fprintf(stderr, "  +-----------------------------------------------------------------------------+\n");
    fprintf(stderr, "  | %20s | %15s | %15s | %16s |\n", "Fabric Index:Node ID", "Start Counter", "Counter Offset",
            "MonitoredSubject");

    while (iter->Next(info))
    {
        fprintf(stderr, "  +-----------------------------------------------------------------------------+\n");
        fprintf(stderr, "  | %3" PRIu32 ":" ChipLogFormatX64 " | %15" PRIu32 " | %15" PRIu32 " | " ChipLogFormatX64 " |\n",
                static_cast<uint32_t>(info.peer_node.GetFabricIndex()), ChipLogValueX64(info.peer_node.GetNodeId()),
                info.start_icd_counter, info.offset, ChipLogValueX64(info.monitored_subject));

        if (std::is_same<Crypto::DefaultSessionKeystore, Crypto::RawKeySessionKeystore>::value)
        {
            // The following cast is valid only when `DefaultSessionKeystore` is `RawKeySessionKeystore`.
            Encoding::BytesToHex(info.shared_key.As<Crypto::Symmetric128BitsKeyByteArray>(), Crypto::kAES_CCM128_Key_Length,
                                 icdSymmetricKeyHex, sizeof(icdSymmetricKeyHex), chip::Encoding::HexFlags::kNullTerminate);
            fprintf(stderr, "  | Symmetric Key: %60s |\n", icdSymmetricKeyHex);
        }
    }

    fprintf(stderr, "  +-----------------------------------------------------------------------------+\n");

    iter->Release();
    SetCommandExitStatus(CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}

void registerCommandsICD(Commands & commands, CredentialIssuerCommands * credsIssuerConfig)
{
    const char * clusterName = "ICD";

    commands_list clusterCommands = {
        make_unique<ICDListCommand>(credsIssuerConfig),
    };

    commands.RegisterCommandSet(clusterName, clusterCommands, "Commands for client-side ICD management.");
}
