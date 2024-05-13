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

#pragma once

#include <app/CommandSender.h>
#include <protocols/bdx/BdxTransferServerDelegate.h>

#include <map>
#include <string>

class BDXDiagnosticLogsServerDelegate : public chip::bdx::BDXTransferServerDelegate
{
public:
    static BDXDiagnosticLogsServerDelegate & GetInstance() { return sInstance; }

    void AddFileDesignator(chip::app::CommandSender * sender, const chip::CharSpan & fileDesignator);
    void RemoveFileDesignator(chip::app::CommandSender * sender);

    /////////// BDXTransferServerDelegate Interface /////////
    CHIP_ERROR OnTransferBegin(chip::bdx::BDXTransferProxy * transfer) override;
    CHIP_ERROR OnTransferEnd(chip::bdx::BDXTransferProxy * transfer, CHIP_ERROR error) override;
    CHIP_ERROR OnTransferData(chip::bdx::BDXTransferProxy * transfer, const chip::ByteSpan & data) override;

private:
    BDXDiagnosticLogsServerDelegate() = default;

    std::map<chip::app::CommandSender *, std::string> mFileDesignators;
    static BDXDiagnosticLogsServerDelegate sInstance;
};
