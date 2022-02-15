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
#include <platform/CHIPDeviceLayer.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using chip::FormatCHIPError;
using chip::Shell::Engine;
using chip::Shell::streamer_get;

namespace {

intptr_t ReadLine(char * buffer, size_t max)
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

    return inptr - buffer;
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

    VerifyOrExit((len - i) > 0, cursor = 0);

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

exit:
    return cursor;
}

} // namespace

namespace chip {
namespace Shell {

void Engine::RunMainLoop()
{
    CHIP_ERROR retval;
    int argc;
    char * argv[CHIP_SHELL_MAX_TOKENS];
    char line[CHIP_SHELL_MAX_LINE_SIZE];

    Engine::Root().RegisterDefaultCommands();

    while (true)
    {
        streamer_printf(streamer_get(), CHIP_SHELL_PROMPT);

        ReadLine(line, sizeof(line));
        argc = TokenizeLine(line, argv, CHIP_SHELL_MAX_TOKENS);

        if (argc > 0)
        {
            retval = Engine::Root().ExecCommand(argc, argv);

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
        else
        {
            // Empty input has no output -- just display prompt
        }
    }
}

} // namespace Shell
} // namespace chip
