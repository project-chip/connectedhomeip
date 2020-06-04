/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Apache Software Foundation (ASF)
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

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#ifndef CHIP_SHELL_PROMPT
#define CHIP_SHELL_PROMPT "> "
#endif // CHIP_SHELL_PROMPT

#ifndef CHIP_SHELL_MAX_MODULES
#define CHIP_SHELL_MAX_MODULES 4
#endif // CHIP_SHELL_MAX_MODULES

#ifndef CHIP_SHELL_MAX_LINE_SIZE
#define CHIP_SHELL_MAX_LINE_SIZE 256
#endif // CHIP_SHELL_MAX_LINE_SIZE

#ifndef CHIP_SHELL_MAX_TOKENS
#define CHIP_SHELL_MAX_TOKENS 10
#endif // CHIP_SHELL_MAX_TOKENS

static const struct chip_shell_cmd * the_cmd_set[CHIP_SHELL_MAX_MODULES];
static unsigned the_cmd_set_size[CHIP_SHELL_MAX_MODULES];
static unsigned the_cmd_set_count;

int shell_line_read(char * buffer, size_t max)
{
    char * inptr = buffer;
    int read     = 0;
    bool done    = false;

    // Read in characters until we get a new line or we hit our max size.
    while (((inptr - buffer) < max) && !done)
    {
        if (read == 0)
        {
            read = streamer_read(streamer_get(), (uint8_t *) inptr, 1);
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
                if (isprint((int) *inptr))
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

int shell_line_tokenize(char * buffer, char ** tokens, int max_tokens)
{
    int len    = strlen(buffer);
    int cursor = 0;

    // The first token starts at the beginning.
    tokens[cursor++] = &buffer[0];

    for (int i = 0; i < len && cursor < max_tokens; i++)
    {
        if (buffer[i] == ' ')
        {
            buffer[i]        = 0;
            tokens[cursor++] = &buffer[i + 1];
        }
    }

    return cursor;
}

void chip_shell_command_foreach(on_command_t * on_command, void * arg)
{
    for (unsigned i = 0; i < the_cmd_set_count; i++)
    {
        for (unsigned j = 0; j < the_cmd_set_size[i]; j++)
        {
            on_command(&the_cmd_set[i][j], arg);
        }
    }
}

int chip_shell_register(const struct chip_shell_cmd * command_set, unsigned count)
{
    if (the_cmd_set_count >= CHIP_SHELL_MAX_MODULES)
    {
        // DFLT_LOG_ERROR("Max number of modules reached\n");
        assert(0);
    }

    the_cmd_set[the_cmd_set_count]      = command_set;
    the_cmd_set_size[the_cmd_set_count] = count;
    ++the_cmd_set_count;

    return 0;
}

void chip_shell_task(void * arg)
{
    int retval;
    int argc;
    char * argv[CHIP_SHELL_MAX_TOKENS];
    char line[CHIP_SHELL_MAX_LINE_SIZE];

    // Initialize the default streamer that was linked.
    streamer_init(streamer_get());

    // Register default commands.
    chip_shell_cmd_init();

    while (1)
    {
        streamer_printf(streamer_get(), CHIP_SHELL_PROMPT);

        shell_line_read(line, sizeof(line));
        argc = shell_line_tokenize(line, argv, CHIP_SHELL_MAX_TOKENS);

        // Find the command
        for (unsigned i = 0; i < the_cmd_set_count; i++)
        {
            for (unsigned j = 0; j < the_cmd_set_size[i]; j++)
            {
                if (strcmp(argv[0], the_cmd_set[i][j].cmd_name) == 0)
                {
                    // Execute the command!
                    retval = the_cmd_set[i][j].cmd_func(argc - 1, argv + 1);

                    if (retval)
                    {
                        streamer_printf(streamer_get(), "Error %s: %d\r\n", argv[0], retval);
                    }
                    else
                    {
                        streamer_printf(streamer_get(), "Done\r\n", argv[0]);
                    }
                    break;
                }
            }
        }
    }
}
