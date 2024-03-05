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

using namespace ::chip;

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

        static_assert(std::is_same<decltype(CHIPCommand::sSessionKeystore), Crypto::RawKeySessionKeystore>::value,
                      "The following BytesToHex can copy/encode the key bytes from sharedKey to hexadecimal format, which only "
                      "works for RawKeySessionKeystore");
        Encoding::BytesToHex(info.aes_key_handle.As<Crypto::Symmetric128BitsKeyByteArray>(), Crypto::kAES_CCM128_Key_Length,
                             icdAesKeyHex, sizeof(icdAesKeyHex), chip::Encoding::HexFlags::kNullTerminate);
        fprintf(stderr, "  | aes key: %60s |\n", icdAesKeyHex);
        Encoding::BytesToHex(info.hmac_key_handle.As<Crypto::Symmetric128BitsKeyByteArray>(), Crypto::kHMAC_CCM128_Key_Length,
                             icdHmacKeyHex, sizeof(icdHmacKeyHex), chip::Encoding::HexFlags::kNullTerminate);
        fprintf(stderr, "  | hmac key: %60s |\n", icdHmacKeyHex);
    }

    fprintf(stderr, "  +-----------------------------------------------------------------------------+\n");
    SetCommandExitStatus(CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}

void registerCommandsICD(Commands & commands, CredentialIssuerCommands * credsIssuerConfig)
{
    const char * name = "ICD";

    commands_list list = {
        make_unique<ICDListCommand>(credsIssuerConfig),
    };

    commands.RegisterCommandSet(name, list, "Commands for client-side ICD management.");
}
