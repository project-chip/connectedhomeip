/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
