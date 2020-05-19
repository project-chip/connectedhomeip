
#import <readline/readline.h>
#import <ctype.h>
#import <stdio.h>
#import <stdlib.h>
#import <string.h>
#import <unistd.h>
#import <CoreFoundation/CFRunLoop.h>

#import "CHIPToolCommandManager.h"
#import "setup_payload_operations.h"

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
const chiptool_command_t legacy_commands[] = { { "help", help, "[command ...]", "Show all commands. Or show usage for a command." },

    { "generate-qr-code", setup_payload_operation_generate_qr_code,
        "[-f file-path]\n"
        "    -f File path of payload.\n",
        "Generate qr code from payload in text file." },

    { "generate-manual-code", setup_payload_operation_generate_manual_code,
        "[-f file-path]\n"
        "    -f File path of payload.\n",
        "Generate manuald code from payload in text file." },

    // Last one
    {} };

/* Global variables. */
typedef chiptool_command_t command;
command * commands = nil;

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
    }
    return 0;
}

/* Print a (hopefully) useful usage message. */
static int usage(void)
{
    printf("Usage: %s [-h] [command] [opt ...]\n"
           "%s commands are:\n",
        prog_name, prog_name);
    printf("");
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
        result = [[CHIPToolCommandManager sharedInstance] executeCommand:c argc:argc argv:argv];
        if (result == 2)
            fprintf(stderr, "Usage: %s %s\n        %s\n", c->c_name, c->c_usage, c->c_help);

        return result;
    } else {
        CHIPPrintf(@"unknown command \"%s\"", argv[0]);
        return 1;
    }
}

#import <sys/select.h>
#import <sys/types.h>
#import <unistd.h>

int main(int argc, char * const * argv)
{
    int result = 0;
    int do_help = 0;
    int ch;

    [[CHIPToolCommandManager sharedInstance] configureWithLegacyCommands:legacy_commands];
    commands = [[CHIPToolCommandManager sharedInstance] allCommands];

    /* Remember my name. */
    prog_name = strrchr(argv[0], '/');
    prog_name = prog_name ? prog_name + 1 : argv[0];
    /* Do getopt stuff for global options. */
    optind = 1;
    optreset = 1;
    while ((ch = getopt(argc, argv, "h")) != -1) {
        switch (ch) {
        case 'h':
            do_help = 1;
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
    } else {
        result = usage();
    }
    return result;
}
