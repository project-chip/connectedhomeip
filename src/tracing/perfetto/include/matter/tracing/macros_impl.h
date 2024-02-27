/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

/* Ensure we do not have double tracing macros defined */
#if defined(MATTER_TRACE_BEGIN)
#error "Tracing macros seem to be double defined"
#endif

#include <perfetto.h>

PERFETTO_DEFINE_CATEGORIES(perfetto::Category("Matter").SetDescription("Matter trace events"));

#define MATTER_TRACE_BEGIN(label, group) TRACE_EVENT_BEGIN("Matter", label, "class_name", group)
#define MATTER_TRACE_END(label, group) TRACE_EVENT_END("Matter")
#define MATTER_TRACE_INSTANT(label, group) TRACE_EVENT_INSTANT("Matter", label, "class_name", group)
#define MATTER_TRACE_SCOPE(label, group) TRACE_EVENT("Matter", label, "class_name", group)

#define MATTER_TRACE_COUNTER(label)                                                                                                \
    do                                                                                                                             \
    {                                                                                                                              \
        static int count##_label = 0;                                                                                              \
        TRACE_COUNTER("Matter", label, ++count##_label);                                                                           \
    } while (0)
