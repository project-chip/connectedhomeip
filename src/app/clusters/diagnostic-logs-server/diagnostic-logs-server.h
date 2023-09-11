/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
