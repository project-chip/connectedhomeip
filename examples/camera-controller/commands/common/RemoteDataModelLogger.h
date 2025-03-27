/*
 *   Copyright (c) 2025 Project CHIP Authors
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

#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventHeader.h>
#include <app/MessageDef/StatusIB.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/dnssd/Resolver.h>

class RemoteDataModelLoggerDelegate
{
public:
    CHIP_ERROR virtual LogJSON(const char *) = 0;
    virtual ~RemoteDataModelLoggerDelegate(){};
};

namespace RemoteDataModelLogger {
CHIP_ERROR LogAttributeAsJSON(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data);
CHIP_ERROR LogErrorAsJSON(const chip::app::ConcreteDataAttributePath & path, const chip::app::StatusIB & status);
CHIP_ERROR LogCommandAsJSON(const chip::app::ConcreteCommandPath & path, chip::TLV::TLVReader * data);
CHIP_ERROR LogErrorAsJSON(const chip::app::ConcreteCommandPath & path, const chip::app::StatusIB & status);
CHIP_ERROR LogEventAsJSON(const chip::app::EventHeader & header, chip::TLV::TLVReader * data);
CHIP_ERROR LogErrorAsJSON(const chip::app::EventHeader & header, const chip::app::StatusIB & status);
CHIP_ERROR LogErrorAsJSON(const CHIP_ERROR & error);
CHIP_ERROR LogGetCommissionerNodeId(chip::NodeId value);
CHIP_ERROR LogGetCommissionerRootCertificate(const char * value);
CHIP_ERROR LogIssueNOCChain(const char * noc, const char * icac, const char * rcac, const char * ipk);
CHIP_ERROR LogDiscoveredNodeData(const chip::Dnssd::CommissionNodeData & nodeData);
void SetDelegate(RemoteDataModelLoggerDelegate * delegate);
}; // namespace RemoteDataModelLogger
