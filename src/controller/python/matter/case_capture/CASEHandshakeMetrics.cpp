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

#include <controller/python/matter/case_capture/CASEHandshakeMetrics.h>

#include <cstring>

#include <controller/python/matter/case_capture/CASEHandshakeMetricsBackend.h>
#include <controller/python/matter/case_capture/CompletedCASEHandshakeQueue.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>

static_assert(CONFIG_BUILD_FOR_HOST_UNIT_TEST,
              "CASEHandshakeMetrics.cpp requires CONFIG_BUILD_FOR_HOST_UNIT_TEST; it is only valid for the Python test build.");

extern "C" {

PyChipError pychip_case_handshake_metrics_start_notifications(uint32_t depth)
{
    VerifyOrReturnError(depth <= chip::python::kCASEHandshakeMetricsNotificationQueueMaxDepth,
                        ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));

    // The notification queue carries its own lock, so it is opened on the calling thread.
    // Registering the backend does go through the event loop, because that is where the tracing
    // registry lives.
    chip::python::CompletedCASEHandshakes().Open(depth == 0 ? chip::python::kCASEHandshakeMetricsNotificationQueueDefaultDepth
                                                            : depth);
    chip::python::StartCASEHandshakeMetricsBackend();
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_case_handshake_metrics_stop_notifications(void)
{
    chip::python::CompletedCASEHandshakes().Close();
    chip::python::StopCASEHandshakeMetricsBackend();
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_case_handshake_metrics_wait_for_notification(PychipCASEHandshakeMetricsRecord * out, uint32_t timeoutMs,
                                                                uint8_t * received, uint32_t * dropped)
{
    VerifyOrReturnError(out != nullptr && received != nullptr && dropped != nullptr, ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));

    *out                  = PychipCASEHandshakeMetricsRecord{};
    uint32_t droppedCount = 0;
    const bool gotRecord  = chip::python::CompletedCASEHandshakes().Wait(*out, timeoutMs, droppedCount);

    *received = gotRecord ? 1u : 0u;
    *dropped  = droppedCount;
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_case_handshake_metrics_get_abandoned_count(uint32_t * abandoned)
{
    VerifyOrReturnError(abandoned != nullptr, ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));

    *abandoned = chip::python::AbandonedCASEHandshakeCount();
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_case_handshake_metrics_get_record_size(uint32_t * size)
{
    VerifyOrReturnError(size != nullptr, ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));

    *size = static_cast<uint32_t>(sizeof(PychipCASEHandshakeMetricsRecord));
    return ToPyChipError(CHIP_NO_ERROR);
}

} // extern "C"
