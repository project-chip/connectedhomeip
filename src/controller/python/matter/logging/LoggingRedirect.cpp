/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/core/CHIPConfig.h>
#include <lib/support/StringBuilder.h>
#include <lib/support/logging/CHIPLogging.h>

namespace {

constexpr size_t kMaxLogMessageLength = CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE;
static_assert(kMaxLogMessageLength > 16, "Log message max size is too small");

using PythonLogCallback = void (*)(uint8_t category, const char * module, const char * message);

PythonLogCallback sPythonLogCallback;

void ENFORCE_FORMAT(3, 0) NativeLoggingCallback(const char * module, uint8_t category, const char * msg, va_list args)
{
    if (sPythonLogCallback == nullptr)
    {
        return;
    }
    chip::StringBuilder<kMaxLogMessageLength> formatter;
    formatter.AddFormatV(msg, args).AddMarkerIfOverflow();

    sPythonLogCallback(category, module, formatter.c_str());
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
