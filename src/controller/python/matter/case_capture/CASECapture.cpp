/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <controller/python/matter/case_capture/CASECapture.h>

#include <cstring>

#include <controller/python/matter/native/ChipMainLoopWork.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TypeTraits.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/StatusReport.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/MessageHeader.h>

static_assert(CONFIG_BUILD_FOR_HOST_UNIT_TEST,
              "CASECapture.cpp requires CONFIG_BUILD_FOR_HOST_UNIT_TEST; this file is only valid for the Python test build.");

namespace {

PychipCaseCapturedHeaders MakeHeaders(const chip::PacketHeader & packetHeader, const chip::PayloadHeader & payloadHeader)
{
    PychipCaseCapturedHeaders headers{};
    headers.sessionType = chip::to_underlying(packetHeader.GetSessionType());
    headers.sFlag       = packetHeader.HasSourceNodeId() ? 1u : 0u;
    headers.dsiz        = packetHeader.HasDestinationGroupId() ? 2u : packetHeader.HasDestinationNodeId() ? 1u : 0u;
    headers.protocolId  = payloadHeader.GetProtocolID().GetProtocolId();
    headers.opcode      = payloadHeader.GetMessageType();
    headers.isInitiator = payloadHeader.IsInitiator() ? 1u : 0u;
    headers.present     = 1u;
    return headers;
}

// Captures inbound Sigma2 / Sigma2_Resume / StatusReport for Python tests.
class CaseReceivedMessageObserver : public chip::Messaging::TestOnlyReceivedMessageObserver
{
public:
    void OnMessageReceived(const chip::PacketHeader & packetHeader, const chip::PayloadHeader & payloadHeader,
                           const chip::System::PacketBufferHandle & msgBuf) override
    {
        if (!payloadHeader.HasProtocol(chip::Protocols::SecureChannel::Id))
        {
            return;
        }

        using chip::Protocols::SecureChannel::MsgType;
        const auto opcode = static_cast<MsgType>(payloadHeader.GetMessageType());

        if (opcode == MsgType::CASE_Sigma2)
        {
            mSigma2 = MakeHeaders(packetHeader, payloadHeader);
            return;
        }

        if (opcode == MsgType::CASE_Sigma2Resume)
        {
            mSigma2Resume = MakeHeaders(packetHeader, payloadHeader);
            return;
        }

        if (opcode == MsgType::StatusReport)
        {
            mStatusReport             = MakeHeaders(packetHeader, payloadHeader);
            mStatusReportParsed       = 0;
            mStatusReportGeneralCode  = 0;
            mStatusReportProtocolId   = 0;
            mStatusReportProtocolCode = 0;
            if (!msgBuf.IsNull())
            {
                // Use Retain() to create a second handle because Parse() consumes the handle; the original must stay valid so
                // ExchangeManager can keep processing the message.
                chip::Protocols::SecureChannel::StatusReport report;
                if (report.Parse(msgBuf.Retain()) == CHIP_NO_ERROR)
                {
                    mStatusReportGeneralCode  = chip::to_underlying(report.GetGeneralCode());
                    mStatusReportProtocolId   = report.GetProtocolId().ToFullyQualifiedSpecForm();
                    mStatusReportProtocolCode = report.GetProtocolCode();
                    mStatusReportParsed       = 1;
                }
            }
        }
    }

    void ResetSlots()
    {
        mSigma2                   = PychipCaseCapturedHeaders{};
        mSigma2Resume             = PychipCaseCapturedHeaders{};
        mStatusReport             = PychipCaseCapturedHeaders{};
        mStatusReportParsed       = 0;
        mStatusReportGeneralCode  = 0;
        mStatusReportProtocolId   = 0;
        mStatusReportProtocolCode = 0;
    }

    void FillSnapshot(PychipCaseCaptureSnapshot & out) const
    {
        out.sigma2                   = mSigma2;
        out.sigma2Resume             = mSigma2Resume;
        out.statusReport             = mStatusReport;
        out.statusReportParsed       = mStatusReportParsed;
        out.statusReportGeneralCode  = mStatusReportGeneralCode;
        out.statusReportProtocolId   = mStatusReportProtocolId;
        out.statusReportProtocolCode = mStatusReportProtocolCode;
    }

private:
    PychipCaseCapturedHeaders mSigma2{};
    PychipCaseCapturedHeaders mSigma2Resume{};
    PychipCaseCapturedHeaders mStatusReport{};
    uint8_t mStatusReportParsed        = 0;
    uint16_t mStatusReportGeneralCode  = 0;
    uint32_t mStatusReportProtocolId   = 0;
    uint16_t mStatusReportProtocolCode = 0;
};

CaseReceivedMessageObserver gCaseObserver;

} // namespace

extern "C" {

PyChipError pychip_case_capture_set_observer(chip::Controller::DeviceCommissioner * devCtrl)
{
    VerifyOrReturnError(devCtrl != nullptr, ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));

    chip::MainLoopWork::ExecuteInMainLoop([devCtrl] {
        if (devCtrl->ExchangeMgr()->GetTestOnlyReceivedMessageObserver() != &gCaseObserver)
        {
            devCtrl->ExchangeMgr()->SetTestOnlyReceivedMessageObserver(&gCaseObserver);
        }
    });
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_case_capture_reset(void)
{
    chip::MainLoopWork::ExecuteInMainLoop([] { gCaseObserver.ResetSlots(); });
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_case_capture_get_snapshot(PychipCaseCaptureSnapshot * out)
{
    VerifyOrReturnError(out != nullptr, ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));

    std::memset(out, 0, sizeof(*out));
    chip::MainLoopWork::ExecuteInMainLoop([out] { gCaseObserver.FillSnapshot(*out); });
    return ToPyChipError(CHIP_NO_ERROR);
}

} // extern "C"
