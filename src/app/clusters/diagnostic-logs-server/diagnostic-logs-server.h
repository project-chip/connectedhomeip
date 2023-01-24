/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandlerInterface.h>
#include <lib/support/BytesCircularBuffer.h>

#include <array>

/// A reference implementation for DiagnosticLogs source.
class DiagnosticLogsCommandHandler : public chip::app::CommandHandlerInterface
{
public:
    static constexpr const uint16_t kDiagnosticLogsEndpoint   = 0;
    static constexpr const uint16_t kDiagnosticLogsBufferSize = 4 * 1024; // 4K internal memory to store text logs

    DiagnosticLogsCommandHandler() :
        CommandHandlerInterface(chip::MakeOptional<chip::EndpointId>(chip::EndpointId(kDiagnosticLogsEndpoint)),
                                chip::app::Clusters::DiagnosticLogs::Id),
        mBuffer(mStorage.data(), mStorage.size())
    {}

    // Inherited from CommandHandlerInterface
    void InvokeCommand(HandlerContext & handlerContext) override;

    CHIP_ERROR PushLog(const chip::ByteSpan & payload);

private:
    std::array<uint8_t, kDiagnosticLogsBufferSize> mStorage;
    chip::BytesCircularBuffer mBuffer;
};
