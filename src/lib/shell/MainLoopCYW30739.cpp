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
            streamer_printf(streamer_get(), "Done\r\n");
        }
    }
    streamer_printf(streamer_get(), CHIP_SHELL_PROMPT);
}

} // namespace

namespace chip {
namespace Shell {

static char sInputBuffer[CHIP_SHELL_MAX_LINE_SIZE];
static uint16_t sInputLength;

void ProcessInput(intptr_t args)
{
    char input[32];
    const ssize_t read_count = streamer_read(streamer_get(), input, sizeof(input));
    for (uint8_t i = 0; i < read_count; i++)
    {
        switch (input[i])
        {
        case '\r':
        case '\n':
            streamer_printf(streamer_get(), "\r\n");
            sInputBuffer[sInputLength] = '\0';
            ProcessShellLine(reinterpret_cast<intptr_t>(sInputBuffer));
            sInputLength = 0;
            break;

        case '\b':
        case 127:
            if (sInputLength > 0)
            {
                streamer_printf(streamer_get(), "\b \b");
                sInputBuffer[--sInputLength] = '\0';
            }
            break;

        default:
            if (sInputLength < MATTER_ARRAY_SIZE(sInputBuffer) - 1)
            {
                sInputBuffer[sInputLength++] = input[i];
            }
            if (isprint(static_cast<int>(input[i])) || input[i] == '\t')
            {
                streamer_printf(streamer_get(), "%c", input[i]);
            }
            break;
        }
    }
}

void Engine::RunMainLoop()
{
    sInputLength = 0;

    streamer_printf(streamer_get(), CHIP_SHELL_PROMPT);
}

} // namespace Shell
} // namespace chip
