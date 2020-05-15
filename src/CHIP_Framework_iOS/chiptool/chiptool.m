

#import <readline/readline.h>
#import <ctype.h>
#import <stdio.h>
#import <stdlib.h>
#import <string.h>
#import <unistd.h>
#import <CoreFoundation/CFRunLoop.h>

#import "CHIPToolCommandManager.h"

/* Maximum length of an input line in interactive mode. */
#define MAX_LINE_LEN 4096
/* Maximum number of arguments on an input line in interactive mode. */
#define MAX_ARGS 32
/* Entry in commands array for a command. */

/* The default prompt. */
const char * prompt_string = "chiptool> ";

/* The name of this program. */
const char * prog_name;

/* Forward declarations of static functions. */
static int help(int argc, char * const * argv);

/*
 * The command array itself.
 * Add commands here at will.
 * Matching is done on a prefix basis.  The first command in the array
 * gets matched first.
 */
const chiptool_legacy_command_t legacy_commands[]
    = { { "help", help, "[command ...]", "Show all commands. Or show usage for a command.", NO, NO },

          { "say-hello", say_hello, "", "Says hello.", YES, YES },
          /* Please consider using an CHIPToolTask */
          /* SEE CHIPToolTask.h and REGISTER_TOOL_TASKS_HERE.m */

          // Last one
          {} };

/* Global variables. */
typedef chiptool_command_t command;
command * commands = nil;
int do_quiet = 0;
int do_verbose = 0;
int do_wait = 0;
int never_wait = 0;

/* Return 1 if name matches command. */
static int match_command(const char * command_name, const char * name) { return !strncmp(command_name, name, strlen(name)); }
/* The help command. */
static int help(int argc, char * const * argv)
{
    const command * c;
    if (argc > 1) {
        char * const * arg;
        for (arg = argv + 1; *arg; ++arg) {
            int found = 0;
            for (c = commands; c->c_name; ++c) {
                if (match_command(c->c_name, *arg)) {
                    found = 1;
                    break;
                }
            }
            if (found)
                CHIPPrintf(@"Usage: %s %s", c->c_name, c->c_usage);
            else {
                CHIPPrintf(@"%s: no such command: %s", argv[0], *arg);
                return 1;
            }
        }
    } else {
        NSMutableArray * sortedHelpCommands = [NSMutableArray array];
        for (c = commands; c->c_name; ++c) {
            [sortedHelpCommands addObject:[NSString stringWithFormat:@"%-40s %s", c->c_name, c->c_help]];
        }

        [sortedHelpCommands sortUsingComparator:^(NSString * lhs, NSString * rhs) {
            return [lhs compare:rhs];
        }];

        for (NSString * help in sortedHelpCommands) {
            CHIPPrintf(@"   %@", help);
        }

        //        for (c = commands; c->c_name; ++c)
        //            CHIPPrintf(@"    %-40s %s", c->c_name, c->c_help);
    }
    return 0;
}
/* States for split_line parser. */
typedef NS_ENUM(NSInteger, parse_state) { SKIP_WS, READ_ARG, READ_ARG_ESCAPED, QUOTED_ARG, QUOTED_ARG_ESCAPED };

/* Run the run loop for a bit */
static void receive_notifications(void)
{
    @autoreleasepool {
        /* Run the CFRunloop to get any pending notifications. */
        while (CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.0, TRUE) == kCFRunLoopRunHandledSource)
            ;
    }
}

/* Run the run loop for 2 seconds */
static void runloop_short(void)
{
    if (never_wait == 0) {
        @autoreleasepool {
            [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:10]];
        }
    }
}

/* Split a line into multiple arguments and return them in *pargc and *pargv. */
static void split_line(char * line, int * pargc, char * const ** pargv)
{
    static char * argvec[MAX_ARGS + 1];
    int argc = 0;
    char * ptr = line;
    char * dst = line;
    parse_state state = SKIP_WS;
    int quote_ch = 0;
    for (ptr = line; *ptr; ++ptr) {
        if (state == SKIP_WS) {
            if (isspace(*ptr))
                continue;
            if (*ptr == '"' || *ptr == '\'') {
                quote_ch = *ptr;
                state = QUOTED_ARG;
                argvec[argc] = dst;
                continue; /* Skip the quote. */
            } else {
                state = READ_ARG;
                argvec[argc] = dst;
            }
        }
        if (state == READ_ARG) {
            if (*ptr == '\\') {
                state = READ_ARG_ESCAPED;
                continue;
            } else if (isspace(*ptr)) {
                /* 0 terminate each arg. */
                *dst++ = '\0';
                argc++;
                state = SKIP_WS;
                if (argc >= MAX_ARGS)
                    break;
            } else
                *dst++ = *ptr;
        }
        if (state == QUOTED_ARG) {
            if (*ptr == '\\') {
                state = QUOTED_ARG_ESCAPED;
                continue;
            }
            if (*ptr == quote_ch) {
                /* 0 terminate each arg. */
                *dst++ = '\0';
                argc++;
                state = SKIP_WS;
                if (argc >= MAX_ARGS)
                    break;
            } else
                *dst++ = *ptr;
        }
        if (state == READ_ARG_ESCAPED) {
            *dst++ = *ptr;
            state = READ_ARG;
        }
        if (state == QUOTED_ARG_ESCAPED) {
            *dst++ = *ptr;
            state = QUOTED_ARG;
        }
    }
    if (state != SKIP_WS) {
        /* Terminate last arg. */
        *dst = '\0';
        argc++;
    }
    /* Teminate arg vector. */
    argvec[argc] = NULL;
    *pargv = argvec;
    *pargc = argc;
}
/* Print a (hopefully) useful usage message. */
static int usage(void)
{
    printf("Usage: %s [-h] [-i] [-l] [-q] [-v] [command] [opt ...]\n"
           "    -i    Run in interactive mode.\n"
           "    -l    Run /usr/bin/leaks -nocontext before exiting.\n"
           "    -q    Be less verbose.\n"
           "    -v    Be more verbose about what's going on.\n"
           "%s commands are:\n",
        prog_name, prog_name);
    help(0, NULL);
    return 2;
}
/* Execute a single command. */
static int execute_command(BOOL oneShot, int argc, char * const * argv)
{
    const command * c;
    int found = 0;
    /* Nothing to do. */
    if (argc == 0)
        return 0;

    if (match_command("quit", argv[0])) {
        exit(0);
        return 0;
    }

    for (c = commands; c->c_name; ++c) {
        if (match_command(c->c_name, argv[0])) {
            found = 1;
            break;
        }
    }
    if (found) {
        int result;

        @autoreleasepool {
            if (c->should_connect) {

                if (oneShot)
                    receive_notifications();

                BOOL run_short = c->should_wait;

                /* Reset getopt for command proc. */
                optind = 1;
                optreset = 1;

                result = [[CHIPToolCommandManager sharedInstance] executeCommand:c argc:argc argv:argv];
                if (result == 2)
                    fprintf(stderr, "Usage: %s %s\n        %s\n", c->c_name, c->c_usage, c->c_help);

                if (oneShot)
                    receive_notifications();

                if (run_short && oneShot)
                    runloop_short();
            } else {
                result = [[CHIPToolCommandManager sharedInstance] executeCommand:c argc:argc argv:argv];
                if (result == 2)
                    fprintf(stderr, "Usage: %s %s\n        %s\n", c->c_name, c->c_usage, c->c_help);
            }
        }

        return result;
    } else {
        CHIPPrintf(@"unknown command \"%s\"", argv[0]);
        return 1;
    }
}

/* A static variable for holding the line. */
static char * line_read = (char *) NULL;
static int my_readline_done = 0;

static void _handle_command(char * command)
{
    if (!command || command[0] == '\0')
        return;

    NSString * commandString = command ? [NSString stringWithUTF8String:command] : nil;
    char * const * av;
    int ac;

    split_line(command, &ac, &av);

    int result = execute_command(NO, ac, av);

    if (result == -1) {
        result = 0;
    }

    if (result && !do_quiet && av && av[0]) {
        fprintf(stderr, "%s: returned %d\n", av[0], result);
    }

    if (commandString) {
        add_history([commandString UTF8String]);
        my_readline_done = 1;
    }
}

#import <sys/select.h>
#import <sys/types.h>
#import <unistd.h>

static char * my_readline(const char * prompt)
{
    line_read = NULL;
    my_readline_done = 0;
    rl_callback_handler_remove();
    rl_callback_handler_install(prompt, _handle_command);
    while (YES) {
        NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
        fd_set rset;
        int ret;
        int max;

        FD_ZERO(&rset);
        FD_SET(0, &rset);
        max = 1;

        struct timeval timeout;

        timeout.tv_sec = 0;
        timeout.tv_usec = 1000;
        NSAutoreleasePool * innerPool = [[NSAutoreleasePool alloc] init];
        [[NSRunLoop mainRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.000001]];
        [innerPool drain];

        ret = select(max, &rset, NULL, NULL, &timeout);

        if (ret < 0) {
            if (errno == EINTR) {
                [pool drain];
                continue;
            }

            perror("bash: select");
            exit(1);
        }

        if (FD_ISSET(0, &rset)) {
            rl_callback_read_char();
            if (my_readline_done) {
                [pool drain];
                break;
            }
        }

        [pool drain];
    }
    return line_read;
}

/* Read a string, and return a pointer to it.
 Returns NULL on EOF. */
static char * rl_gets(const char * prompt)
{
    /* If the buffer has already been allocated,
     return the memory to the free pool. */
    if (line_read) {
        free(line_read);
        line_read = (char *) NULL;
    }

    /* Get a line from the user. */
    int show_prompt = isatty(0);
    line_read = my_readline(show_prompt ? prompt : "");

    /* If the line has any text in it,
     save it on the history. */
    if (line_read && *line_read)
        add_history(line_read);

    return (line_read);
}

int main(int argc, char * const * argv)
{
    CHIPSetEmbeddedTempDirectory();

    int result = 0;
    int do_help = 0;
    int do_interactive = 0;
    int do_leaks = 0;
    int ch;

    [[CHIPToolCommandManager sharedInstance] configureWithLegacyCommands:legacy_commands];
    commands = [[CHIPToolCommandManager sharedInstance] allCommands];

    /* Remember my name. */
    prog_name = strrchr(argv[0], '/');
    prog_name = prog_name ? prog_name + 1 : argv[0];
    /* Do getopt stuff for global options. */
    optind = 1;
    optreset = 1;
    while ((ch = getopt(argc, argv, "hilp:qvwm")) != -1) {
        switch (ch) {
        case 'h':
            do_help = 1;
            break;

        case 'i':
            do_interactive = 1;
            break;

        case 'w':
            do_wait = 1;
            break;

        case 'l':
            do_leaks = 1;
            break;

        case 'q':
            do_quiet = 1;
            break;

        case 'm':
            _CHIPOverrideCanLogMessageBodies(@"Messages", YES);
            break;

        case 'v':
            do_verbose = 1;

            // Enable verbose CHIP logging
            _CHIPLogForceEnable(YES);
            _CHIPLogForceEnableEverything(YES);
            _CHIPLogForceWriteToStdout(YES);

            break;

        case '?':
        default:
            return usage();
        }
    }

    argc -= optind;
    argv += optind;

    if (do_help) {
        /* Munge argc/argv so that argv[0] is something. */
        return help(argc + 1, argv - 1);

    } else if (argc > 0) {
        result = execute_command(YES, argc, argv);

        if (do_wait && never_wait == 0) {
            while (YES) {
                @try {
                    [[NSRunLoop currentRunLoop] run];
                } @catch (NSException * exception) {
                    CHIPWarn("Agent exception caught on main thread: %@", exception);
                }
            }
        }
    }

    else if (do_interactive) {
        /* In interactive mode we just read commands and run them until readline returns NULL. */
        /* Only show prompt string if stdin is a tty. */
        for (;;) {
            char *const *av, *input;
            int ac;

            input = rl_gets(prompt_string);

            if (!input || input[0] == '\0')
                continue;

            split_line(input, &ac, &av);

            result = execute_command(NO, ac, av);

            if (result == -1) {
                result = 0;
                break;
            }
            if (result && !do_quiet && av && av[0]) {
                fprintf(stderr, "%s: returned %d\n", av[0], result);
            }
        }
    } else {
        result = usage();
    }

    if (do_leaks) {
        char * const argvec[3] = { "leaks", "-nocontext", NULL };
        leaks(2, argvec);
    }
    return result;
}
