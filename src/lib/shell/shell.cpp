/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2017 Google LLC
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

/**
 *    @file
 *      Source implementation for a generic shell API for CHIP examples.
 */

#include "shell.h"
#include "commands.h"

#include <core/CHIPError.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

using namespace chip::Logging;

namespace chip {
namespace Shell {

Shell Shell::theShellRoot;

int shell_line_read(char * buffer, size_t max)
{
    int read     = 0;
    bool done    = false;
    char * inptr = buffer;

    // Read in characters until we get a new line or we hit our max size.
    while (((inptr - buffer) < (int) max) && !done)
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
                if (isprint((int) *inptr) || *inptr == '\t')
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

    return (inptr - buffer);
}

void Shell::ForEachCommand(shell_command_iterator_t * on_command, void * arg)
{
    for (unsigned i = 0; i < _commandSetCount; i++)
    {
        for (unsigned j = 0; j < _commandSetSize[i]; j++)
        {
            if (on_command(&_commandSet[i][j], arg))
            {
                return;
            }
        }
    }
}

void Shell::RegisterCommands(shell_command_t * command_set, unsigned count)
{
    if (_commandSetCount >= CHIP_SHELL_MAX_MODULES)
    {
        ChipLogError(Shell, "Max number of modules reached\n");
        assert(0);
    }

    _commandSet[_commandSetCount]     = command_set;
    _commandSetSize[_commandSetCount] = count;
    ++_commandSetCount;
}

int Shell::ExecCommand(int argc, char * argv[])
{
    int retval = CHIP_ERROR_INVALID_ARGUMENT;

    // Find the command
    for (unsigned i = 0; i < _commandSetCount; i++)
    {
        for (unsigned j = 0; j < _commandSetSize[i]; j++)
        {
            if (strcmp(argv[0], _commandSet[i][j].cmd_name) == 0)
            {
                // Execute the command!
                retval = _commandSet[i][j].cmd_func(argc - 1, argv + 1);
                break;
            }
        }
    }

    return retval;
}

static bool IsSeparator(char aChar)
{
    return (aChar == ' ') || (aChar == '\t') || (aChar == '\r') || (aChar == '\n');
}

static bool IsEscape(char aChar)
{
    return (aChar == '\\');
}

static bool IsEscapable(char aChar)
{
    return IsSeparator(aChar) || IsEscape(aChar);
}

int Shell::TokenizeLine(char * buffer, char ** tokens, int max_tokens)
{
    int len    = strlen(buffer);
    int cursor = 0;
    int i      = 0;

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

    tokens[cursor] = 0;

exit:
    return cursor;
}

void Shell::TaskLoop(void * arg)
{
    int retval;
    int argc;
    char * argv[CHIP_SHELL_MAX_TOKENS];
    char line[CHIP_SHELL_MAX_LINE_SIZE];

    // Initialize the default streamer that was linked.
    streamer_init(streamer_get());

    theShellRoot.RegisterDefaultCommands();

    while (1)
    {
        streamer_printf(streamer_get(), CHIP_SHELL_PROMPT);

        shell_line_read(line, sizeof(line));
        argc = shell_line_tokenize(line, argv, CHIP_SHELL_MAX_TOKENS);

        if (argc > 0)
        {
            retval = theShellRoot.ExecCommand(argc, argv);

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
        else
        {
            // Empty input has no output -- just display prompt
        }
    }
}

} // namespace Shell
} // namespace chip
