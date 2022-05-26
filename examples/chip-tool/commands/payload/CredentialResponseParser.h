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

#include "../clusters/DataModelLogger.h"
#include "../common/Command.h"
#include <setup_payload/SetupPayload.h>

using namespace ::chip;

class AttestationElementsParser : public Command, public DataModelLogger
{
public:
    AttestationElementsParser() : Command("attestation-elements-parse") { AddArgument("payload", &mAttResponse); }
    CHIP_ERROR Run() override;

private:
    ByteSpan mAttResponse;
    CHIP_ERROR ParseAttestationElements(const ByteSpan & responsePayload);
};

class NOCCSRElementsParser : public Command, public DataModelLogger
{

public:
    NOCCSRElementsParser() : Command("noccsr-elements-parse") { AddArgument("payload", &mCSRResponse); }
    CHIP_ERROR Run() override;

private:
    ByteSpan mCSRResponse;
    CHIP_ERROR ParseNOCCSRResponse(const ByteSpan & responsePayload);
};