/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lib/support/logging/CHIPLogging.h>
#include <stdio.h>

namespace {

constexpr size_t kMaxLogMessageLength = 256;

using PythonLogCallback = void (*)(uint8_t category, const char * module, const char * message);

PythonLogCallback sPythonLogCallback;

void ENFORCE_FORMAT(3, 0) NativeLoggingCallback(const char * module, uint8_t category, const char * msg, va_list args)
{
    if (sPythonLogCallback == nullptr)
    {
        return;
    }
    char buffer[kMaxLogMessageLength];
    vsnprintf(buffer, sizeof(buffer), msg, args);
    buffer[sizeof(buffer) - 1] = 0;

    sPythonLogCallback(category, module, buffer);
}

} // namespace

extern "C" void pychip_logging_set_callback(PythonLogCallback callback)
{
    if (callback == nullptr)
    {
        chip::Logging::SetLogRedirectCallback(nullptr);
        sPythonLogCallback = callback;
    }
    else
    {
        sPythonLogCallback = callback;
        chip::Logging::SetLogRedirectCallback(NativeLoggingCallback);
    }
}
