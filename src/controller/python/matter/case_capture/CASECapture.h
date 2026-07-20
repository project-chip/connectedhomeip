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

#pragma once

#include <stdint.h>

#include <controller/CHIPDeviceController.h>
#include <controller/python/matter/native/PyChipError.h>

extern "C" {

// Layout must match the ctypes mirror in case_capture/__init__.py.
struct PychipCaseCapturedHeaders
{
    uint8_t sessionType;
    uint8_t sFlag; // 0/1
    uint8_t dsiz;  // 0 = neither, 1 = NodeID, 2 = GroupID
    uint16_t protocolId;
    uint8_t opcode;
    uint8_t isInitiator;
    uint8_t present;
};

struct PychipCaseCaptureSnapshot
{
    PychipCaseCapturedHeaders sigma2;
    PychipCaseCapturedHeaders sigma2Resume;
    PychipCaseCapturedHeaders statusReport;
    uint8_t statusReportParsed; // 1 if the StatusReport body decoded; if 0, the *Code fields below are not meaningful.
    uint16_t statusReportGeneralCode;
    uint32_t statusReportProtocolId;
    uint16_t statusReportProtocolCode;
};

PyChipError pychip_case_capture_set_observer(chip::Controller::DeviceCommissioner * devCtrl);
PyChipError pychip_case_capture_reset(void);
PyChipError pychip_case_capture_get_snapshot(PychipCaseCaptureSnapshot * out);

} // extern "C"
