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

#include "matter_shell.h"
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

constexpr const char kShellPrompt[] = "matterCli > ";

// max > 1
void ReadLine(char * buffer, size_t max)
{
    size_t line_sz = 0;

    // Read in characters until we get a line ending or EOT.
    for (bool done = false; !done;)
    {
        // Stop reading if we've run out of space in the buffer (still need to null-terminate).
        if (line_sz >= max - 1u)
        {
            buffer[max - 1] = '\0';
            break;
        }

        chip::WaitForShellActivity();
        if (streamer_read(streamer_get(), buffer + line_sz, 1) != 1)
        {
            continue;
        }

        // Process character we just read.
        switch (buffer[line_sz])
        {
        case '\r':
        case '\n':
            streamer_printf(streamer_get(), "\r\n");
            buffer[line_sz] = '\0';
            line_sz++;
            done = true;
            break;
        case 0x7F:
            // Do not accept backspace character (i.e. don't increment line_sz) and remove 1 additional character if it exists.
            if (line_sz >= 1u)
            {
                streamer_printf(streamer_get(), "\b \b");
                line_sz--;
            }
            break;
        default:
            if (isprint(static_cast<int>(buffer[line_sz])) || buffer[line_sz] == '\t')
            {
                streamer_printf(streamer_get(), "%c", buffer[line_sz]);
                line_sz++;
            }
            break;
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
            // Don't treat the previous character as a separator if this one is 0
            // otherwise the trailing space will become a token
            if (!IsSeparator(buffer[i + 1]) && buffer[i + 1] != 0)
            {
                tokens[cursor++] = &buffer[i + 1];
            }
        }
    }
    // If for too many arguments, overwrite last entry with guard.
    if (cursor >= max_tokens)
    {
        cursor = max_tokens - 1;
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

        if (retval != CHIP_NO_ERROR)
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
    streamer_printf(streamer_get(), kShellPrompt);
}

} // namespace

namespace chip {
namespace Shell {

void Engine::RunMainLoop()
{
    streamer_printf(streamer_get(), kShellPrompt);

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
