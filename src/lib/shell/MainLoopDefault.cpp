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

#include "streamer.h"
#include <lib/shell/Engine.h>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>

#include <ctype.h>
#include <string.h>

using chip::FormatCHIPError;
using chip::Platform::MemoryAlloc;
using chip::Platform::MemoryFree;
using chip::Shell::Engine;
using chip::Shell::streamer_get;

namespace {

void ReadLine(char * buffer, size_t max)
{
    ssize_t read = 0;
    bool done    = false;
    char * inptr = buffer;

    // Read in characters until we get a new line or we hit our max size.
    while (((inptr - buffer) < static_cast<int>(max)) && !done)
    {
        if (read == 0)
        {
            read = streamer_read(streamer_get(), inptr, 1);
        }

        // Process any characters we just read in.
        while (read > 0)
        {
            switch (*inptr)
            {
            case '\r':
            case '\n':
                streamer_printf(streamer_get(), "\r\n");
                *inptr = 0; // null terminate
                done   = true;
                break;
            case 0x7F:
                // delete backspace character + 1 more
                inptr -= 2;
                if (inptr >= buffer - 1)
                {
                    streamer_printf(streamer_get(), "\b \b");
                }
                else
                {
                    inptr = buffer - 1;
                }
                break;
            default:
                if (isprint(static_cast<int>(*inptr)) || *inptr == '\t')
                {
                    streamer_printf(streamer_get(), "%c", *inptr);
                }
                else
                {
                    inptr--;
                }
                break;
            }

            inptr++;
            read--;
        }
    }
}

bool IsSeparator(char ch)
{
    return (ch == ' ') || (ch == '\t') || (ch == '\r') || (ch == '\n');
}

bool IsEscape(char ch)
{
    return (ch == '\\');
}

bool IsEscapable(char ch)
{
    return IsSeparator(ch) || IsEscape(ch);
}

int TokenizeLine(char * buffer, char ** tokens, int max_tokens)
{
    size_t len = strlen(buffer);
    int cursor = 0;
    size_t i   = 0;

    // Strip leading spaces
    while (buffer[i] && buffer[i] == ' ')
    {
        i++;
    }

    if (len <= i)
    {
        return 0;
    }

    // The first token starts at the beginning.
    tokens[cursor++] = &buffer[i];

    for (; i < len && cursor < max_tokens; i++)
    {
        if (IsEscape(buffer[i]) && IsEscapable(buffer[i + 1]))
        {
            // include the null terminator: strlen(cmd) = strlen(cmd + 1) + 1
            memmove(&buffer[i], &buffer[i + 1], strlen(&buffer[i]));
        }
        else if (IsSeparator(buffer[i]))
        {
            buffer[i] = 0;
            if (!IsSeparator(buffer[i + 1]))
            {
                tokens[cursor++] = &buffer[i + 1];
            }
        }
    }

    tokens[cursor] = nullptr;

    return cursor;
}

void ProcessShellLine(intptr_t args)
{
    int argc;
    char * argv[CHIP_SHELL_MAX_TOKENS];

    char * line = reinterpret_cast<char *>(args);
    argc        = TokenizeLine(line, argv, CHIP_SHELL_MAX_TOKENS);

    if (argc > 0)
    {
        CHIP_ERROR retval = Engine::Root().ExecCommand(argc, argv);

        if (retval)
        {
            char errorStr[160];
            bool errorStrFound = FormatCHIPError(errorStr, sizeof(errorStr), retval);
            if (!errorStrFound)
            {
                errorStr[0] = 0;
            }
            streamer_printf(streamer_get(), "Error %s: %s\r\n", argv[0], errorStr);
        }
        else
        {
            streamer_printf(streamer_get(), "Done\r\n", argv[0]);
        }
    }
    MemoryFree(line);
    streamer_printf(streamer_get(), CHIP_SHELL_PROMPT);
}

} // namespace

namespace chip {
namespace Shell {

void Engine::RunMainLoop()
{
    Engine::Root().RegisterDefaultCommands();
    streamer_printf(streamer_get(), CHIP_SHELL_PROMPT);

    while (true)
    {
        char * line = static_cast<char *>(Platform::MemoryAlloc(CHIP_SHELL_MAX_LINE_SIZE));
        ReadLine(line, CHIP_SHELL_MAX_LINE_SIZE);
#if CONFIG_DEVICE_LAYER
        DeviceLayer::PlatformMgr().ScheduleWork(ProcessShellLine, reinterpret_cast<intptr_t>(line));
#else
        ProcessShellLine(reinterpret_cast<intptr_t>(line));
#endif
    }
}

} // namespace Shell
} // namespace chip
