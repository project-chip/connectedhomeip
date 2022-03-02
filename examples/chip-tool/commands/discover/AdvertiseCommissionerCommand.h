/*
 *   Copyright (c) 2022 Project CHIP Authors
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

#include <app/server/Dnssd.h>
#include <commands/common/CHIPCommand.h>
#include <commands/common/CredentialIssuerCommands.h>

class AdvertiseCommissionerCommand : public CHIPCommand
{
public:
    AdvertiseCommissionerCommand(CredentialIssuerCommands * credsIssuerConfig) :
        CHIPCommand("advertise-commissioner", credsIssuerConfig)
    {
        AddArgument("interface-id", -1, INT32_MAX, &mInterfaceId);
        AddArgument("custom-txt-record", &mCustomTxtRecord);
    }

    /////////// Command Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(INT16_MAX); }

private:
    chip::Optional<int32_t> mInterfaceId;
    chip::Optional<char *> mCustomTxtRecord;
};
