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

#include "TraceDecoder.h"
#include "TraceDecoderArgumentParser.h"

#include <lib/support/CHIPPlatformMemory.h>
#include <platform/CHIPDeviceLayer.h>

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = chip::Platform::MemoryInit();
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(NotSpecified, "Platform::MemoryInit() failure: %s", chip::ErrorStr(err));
        return EXIT_FAILURE;
    }

    VerifyOrReturnError(CHIP_NO_ERROR == TraceDecoderArgumentParser::GetInstance().ParseArguments(argc, argv), EXIT_FAILURE);

    auto decoder = chip::trace::TraceDecoder();
    decoder.SetOptions(TraceDecoderArgumentParser::GetInstance().options);

    err = decoder.ReadFile(TraceDecoderArgumentParser::GetInstance().logFile);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(NotSpecified, "Can not decode file: %s. Error: %s", TraceDecoderArgumentParser::GetInstance().logFile,
                     chip::ErrorStr(err));
        return EXIT_FAILURE;
    }

    chip::Platform::MemoryShutdown();
    return EXIT_SUCCESS;
}
