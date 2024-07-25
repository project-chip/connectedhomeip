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
#pragma once

#include <os/signpost.h>
#include <tracing/backend.h>

#define MATTER_TRACE_BEGIN(label, group) os_signpost_interval_begin(__DARWIN_MATTER_SIGNPOST_LOGGER(), OS_SIGNPOST_ID_EXCLUSIVE, group "-" label)
#define MATTER_TRACE_END(label, group) os_signpost_interval_end(__DARWIN_MATTER_SIGNPOST_LOGGER(), OS_SIGNPOST_ID_EXCLUSIVE, group "-" label)
#define MATTER_TRACE_INSTANT(label, group) os_signpost_event_emit(__DARWIN_MATTER_SIGNPOST_LOGGER(), OS_SIGNPOST_ID_EXCLUSIVE, group "-" label)

#define MATTER_TRACE_COUNTER(label)                                                                                        \
    do {                                                                                                                   \
        static unsigned int count##_label = 0;                                                                             \
        os_signpost_event_emit(__DARWIN_MATTER_SIGNPOST_LOGGER(), OS_SIGNPOST_ID_EXCLUSIVE, label, "%u", ++count##_label); \
    } while (0)

#define _CONCAT_IMPL(a, b) a##b
#define _MACRO_CONCAT(a, b) _CONCAT_IMPL(a, b)

#define MATTER_TRACE_SCOPE(label, group) ::chip::Tracing::signposts::Scoped _MACRO_CONCAT(_trace_scope, __COUNTER__)(label, group)

#define MATTER_SDK_SIGNPOST_NAME "com.csa.matter.signpost"
#define __DARWIN_MATTER_SIGNPOST_LOGGER() chip::Tracing::signposts::GetMatterSignpostLogger()

namespace chip {
namespace Tracing {
    namespace signposts {

        os_log_t GetMatterSignpostLogger();

        class Scoped {
        public:
            inline Scoped(const char * label, const char * group)
                : mLabel(label)
                , mGroup(group)
            {
                os_signpost_interval_begin(__DARWIN_MATTER_SIGNPOST_LOGGER(), OS_SIGNPOST_ID_EXCLUSIVE, MATTER_SDK_SIGNPOST_NAME, "%s-%s", group, label);
            }
            inline ~Scoped() { os_signpost_interval_end(__DARWIN_MATTER_SIGNPOST_LOGGER(), OS_SIGNPOST_ID_EXCLUSIVE, MATTER_SDK_SIGNPOST_NAME, "%s-%s", mGroup, mLabel); }

        private:
            const char * mLabel;
            const char * mGroup;
        };

        class DarwinTracingBackend : public ::chip::Tracing::Backend {
        public:
            DarwinTracingBackend();

            typedef void (^MetricEventHandler)(MetricEvent event);

            void SetMetricEventHandler(MetricEventHandler callback);
            void LogMetricEvent(const MetricEvent & event) override;

        private:
            MetricEventHandler mClientCallback;
        };

    } // namespace signposts
} // namespace Tracing
} // namespace chip
