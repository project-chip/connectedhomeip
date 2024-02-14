/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <platform/Darwin/Tracing.h>
#include <tracing/metric_event.h>

namespace chip {
namespace Tracing {
namespace signposts {
    os_log_t GetMatterSignpostLogger()
    {
        static dispatch_once_t onceToken;
        static os_log_t logger;
            dispatch_once(&onceToken, ^{
                logger = os_log_create("com.csa.matter.signposts", "com.csa.matter.sdk");
            });
            return logger;
    }

    DarwinTracingBackend::DarwinTracingBackend()
        : mClientCallback(nullptr)
    {}

    void DarwinTracingBackend::SetLogEventClientCallback(LogEventClientCallback callback)
    {
        mClientCallback = callback;
    }

    void DarwinTracingBackend::LogEvent(MetricEvent & event)
    {
        ChipLogProgress(DeviceLayer, "Receive scalar event, type: %s, value: %u",
                        event.key, event.value.uvalue);
        if (mClientCallback) {
            ChipLogProgress(DeviceLayer, "Invoking client callback %p", mClientCallback);
            mClientCallback(event);
        }
    }

} // namespace signposts
} // namespace Tracing
} // namespace chip

