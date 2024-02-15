/*
 *   Copyright (c) 2024 Project CHIP Authors
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

#include "../common/CHIPCommandBridge.h"

class DownloadLogCommand : public CHIPCommandBridge
{
public:
    DownloadLogCommand() : CHIPCommandBridge("download")
    {
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId, "Node to download the logs from.");
        AddArgument("log-type", 0, 2, &mLogType,
                    "The type of log being requested. This should correspond to a value in the enum MTRDiagnosticLogType.");
        AddArgument("timeout", 0, UINT16_MAX, &mTimeout,
                    "The timeout for getting the log. If the timeout expires, completion will be called with whatever has been "
                    "retrieved by that point (which might be none or a partial log). If the timeout is set to 0, the request will "
                    "not expire and completion will not be called until the log is fully retrieved or an error occurs.");
        AddArgument("async", 0, 1, &mIsAsyncCommand,
                    "By default the command waits for the download to finish before returning. If async is true the command will "
                    "not wait and the download will proceed in the background");
        AddArgument("filepath", &mFilePath, "An optional filepath to save the download log content to.");
    }

    /////////// CHIPCommandBridge Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override
    {
        return chip::System::Clock::Seconds16(mTimeout > 0 ? mTimeout + 10 : 300);
    }

private:
    chip::NodeId mNodeId;
    uint8_t mLogType;
    uint16_t mTimeout;
    chip::Optional<char *> mFilePath;
    chip::Optional<bool> mIsAsyncCommand;
};
