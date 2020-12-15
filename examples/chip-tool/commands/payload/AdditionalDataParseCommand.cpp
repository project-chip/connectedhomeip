/*
 *   Copyright (c) 2020 Project CHIP Authors
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

#include "AdditionalDataParseCommand.h"

#include <setup_payload/AdditionalDataPayloadParser.h>
#include <setup_payload/AdditionalDataPayload.h>

using namespace ::chip;

CHIP_ERROR AdditionalDataParseCommand::Run(PersistentStorage & storage, NodeId localId, NodeId remoteId)
{
    std::string payload(mPayload);
    AdditionalDataPayload resultPayload;
    CHIP_ERROR err = CHIP_NO_ERROR;
    SuccessOrExit(err);
    err = AdditionalDataPayloadParser(payload).populatePayload(resultPayload);
    ChipLogProgress(chipTool, "AdditionalDataParseCommand, RotatingDeviceId=%s", resultPayload.rotatingDeviceId.c_str());
exit:
    return err;
}
