/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2017 Nest Labs, Inc.
 *    All rights reserved.
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
 *      Support functions for parsing command-line arguments.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include "CHIPArgParser.hpp"

#if CHIP_CONFIG_ENABLE_ARG_PARSER

#include <climits>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <support/SafeInt.h>

#include <support/CHIPMem.h>
#include <support/CHIPMemString.h>

/*
 * TODO: Revisit these if and when fabric ID and node ID support has
 *       been integrated into the stack.
 */
#ifndef CHIP_ARG_PARSER_PARSE_FABRIC_ID
#define CHIP_ARG_PARSER_PARSE_FABRIC_ID 0
#endif // CHIP_ARG_PARSER_PARSE_FABRIC_ID

#ifndef CHIP_ARG_PARSER_PARSE_NODE_ID
#define CHIP_ARG_PARSER_PARSE_NODE_ID 0
#endif // CHIP_ARG_PARSER_PARSE_NODE_ID

namespace chip {
namespace ArgParser {

using namespace chip;

static char * MakeShortOptions(OptionSet ** optSets);
static struct option * MakeLongOptions(OptionSet ** optSets);
static int32_t SplitArgs(char * argStr, char **& argList, char * initialArg = nullptr);
static bool GetNextArg(char *& parsePoint);
static size_t CountOptionSets(OptionSet * optSets[]);
static size_t CountAllOptions(OptionSet * optSets[]);
static void FindOptionByIndex(OptionSet ** optSets, int optIndex, OptionSet *& optSet, OptionDef *& optDef);
static void FindOptionById(OptionSet ** optSets, int optId, OptionSet *& optSet, OptionDef *& optDef);
static const char ** MakeUniqueHelpGroupNamesList(OptionSet * optSets[]);
static void PutStringWithNewLine(FILE * s, const char * str);
static void PutStringWithBlankLine(FILE * s, const char * str);
#if CHIP_CONFIG_ENABLE_ARG_PARSER_SANTIY_CHECK
static bool SanityCheckOptions(OptionSet * optSets[]);
static bool HelpTextContainsLongOption(const char * optName, const char * helpText);
static bool HelpTextContainsShortOption(char optChar, const char * helpText);
#endif // CHIP_CONFIG_ENABLE_ARG_PARSER_SANTIY_CHECK

static inline bool IsShortOptionChar(int ch)
{
    // According to 'std::isgraph(int ch)' documentation, its behavior is
    // undefined if the value of |ch| is not representable as unsigned char
    // and is not equal to EOF. To prevent from this, additional condition
    // has to be used here.
    return ch > 255 ? false : isgraph(ch);
}

/**
 * @brief
 * The list of OptionSets passed to the currently active ParseArgs() call.
 *
 * @details
 * This value will be NULL when no call to ParseArgs() is in progress.
 */
OptionSet ** gActiveOptionSets = nullptr;

/**
 * @brief
 * Pointer to function used to print errors that occur during argument parsing.
 *
 * @details
 * Applications should call PrintArgError() to report errors in their option and
 * non-option argument handling functions, rather than printing directly to
 * stdout/stderr.
 *
 * Defaults to a pointer to the `DefaultPrintArgError()` function.
 */
void (*PrintArgError)(const char * msg, ...) = DefaultPrintArgError;

/**
 * @fn bool ParseArgs(const char *progName, int argc, char *argv[], OptionSet *optSets[], NonOptionArgHandlerFunct nonOptArgHandler,
 * bool ignoreUnknown)
 *
 * @brief
 * Parse a set of command line-style arguments, calling handling functions to process each
 * option and non-option argument.
 *
 * @param[in]  progName             The name of the program or context in which the arguments are
 *                                  being parsed.  This string will be used to prefix error
 *                                  messages and warnings.
 * @param[in]  argc                 The number of arguments to be parsed, plus 1.
 * @param[in]  argv                 An array of argument strings to be parsed.  The array length must
 * 									be 1 greater than the value specified for argc, and
 * argv[argc] must be set to NULL.  Argument parsing begins with the *second* array element (argv[1]); element 0 is ignored.
 * @param[in]  optSets              A list of pointers to `OptionSet` structures that define the legal
 *                                  options.  The supplied list must be terminated with a NULL.
 * @param[in]  nonOptArgHandler     A pointer to a function that will be called once option parsing
 *                                  is complete with any remaining non-option arguments .  The function
 *                                  is called regardless of whether any arguments remain.  If a NULL
 *                                  is passed `ParseArgs()` will report an error if any non-option
 *                                  arguments are present.
 * @param[in]  ignoreUnknown        If true, silently ignore any unrecognized options.
 *
 * @return                          `true` if all options and non-option arguments were parsed
 *                                  successfully; `false` if an option was unrecognized or if one of
 *                                  the handler functions failed (i.e. returned false).
 *
 *
 * @details
 * ParseArgs() takes a list of arguments (`argv`) and parses them according to a set of supplied
 * option definitions.  The function supports both long (--opt) and short (-o) options and implements
 * the same option syntax as the GNU getopt_long(3) function.
 *
 * Option definitions are passed to ParseArgs() as an array of OptionSet structures (`optSets`).
 * Each OptionSet contains an array of option definitions and a handler function. ParseArgs()
 * processes option arguments in the given order, calling the respective handler function for
 * each recognized option.  Once all options have been parsed, a separate non-option handler
 * function (`nonOptArgHandler`) is called once to process any remaining arguments.
 *
 *
 * ## OPTION SETS
 *
 * An OptionSet contains a set of option definitions along with a pointer to a handler function
 * that will be called when one of the associated options is encountered.  Option sets also
 * contain help text describing the syntax and purpose of each option (see OPTION HELP below).
 * Option sets are designed to allow the creation of re-usable collections of related options.
 * This simplifies the effort needed to maintain multiple applications that accept similar options
 * (e.g. test applications).
 *
 * There are two patterns for defining OptionSets--one can either initialize an instance of the
 * OptionSet struct itself, e.g. as a static global, or subclass OptionSetBase and provide a
 * constructor.  The latter uses a pure virtual `HandleOption()` function to delegate option
 * handling to the subclass.
 *
 * Lists of OptionSets are passed to the ParseArgs() function as a NULL-terminated array of pointers.
 * E.g.:
 *
 *     static OptionSet gToolOptions =
 *     {
 *         HandleOption,        // handler function
 *         gToolOptionDefs,  // array of option definitions
 *         "GENERAL OPTIONS",   // help group
 *         gToolOptionHelp   // option help text
 *     };
 *
 *     static OptionSet *gOptionSets[] =
 *     {
 *         &gToolOptions,
 *         &gNetworkOptions,
 *         &gTestingOptions,
 *         &gHelpOptions,
 *         NULL
 *     };
 *
 *     int main(int argc, char *argv[])
 *     {
 *         if (!ParseArgs("test-app", argc, argv, gOptionSets))
 *         {
 *             ...
 *         }
 *     }
 *
 *
 * ## OPTION DEFINITIONS
 *
 * Options are defined using the `OptionDef` structure. Option definitions are organized as an array
 * of OptionDef elements, where each element contains: the name of the option, a integer id that is
 * used to identify the option, and whether the option expects/allows an argument.  The end of the
 * option array is signaled by a NULL Name field.  E.g.:
 *
 *     enum
 *     {
 *         kOpt_Listen = 1000,
 *         kOpt_Length,
 *         kOpt_Count,
 *     };
 *
 *     static OptionDef gToolOptionDefs[] =
 *     {
 *         // NAME         REQUIRES/ALLOWS ARG?  ID/SHORT OPTION CHAR
 *         // ============================================================
 *         {  "listen",    kNoArgument,          kOpt_Listen     },
 *         {  "length",    kArgumentRequired,    kOpt_Length     },
 *         {  "count",     kArgumentRequired,    kOpt_Count      },
 *         {  "num",       kArgumentRequired,    kOpt_Count      }, // alias for --count
 *         {  "debug",     kArgumentOptional,    'd'             },
 *         {  "help",      kNoArgument,          'h'             },
 *         {  NULL }
 *     };
 *
 *
 * ## OPTION IDS
 *
 * Option ids identify options to the code that handles them (the OptionHandler function). Option ids
 * are relative to the OptionSet in which they appear, and thus may be reused across different
 * OptionSets (however see SHORT OPTIONS below).  Common convention is to start numbering option ids
 * at 1000, however any number > 128 can be used.  Alias options can be created by using the same
 * option id with different option names.
 *
 *
 * ## SHORT OPTIONS
 *
 * Unlike getopt_long(3), ParseArgs() does not take a separate string specifying the list of short
 * option characters.  Rather, any option whose id value falls in the range of graphical ASCII
 * characters will allow that character to be used as a short option.
 *
 * ParseArgs() requires that short option characters be unique across *all* OptionSets.  Because of
 * this, the use of short options is discouraged for any  OptionSets that are shared across programs
 * due to the significant chance for collisions.  Short options characters may be reused within a
 * single OptionSet to allow for the creation of alias long option names.
 *
 *
 * ## OPTION HELP
 *
 * Each OptionSet contains an `OptionHelp` string that describes the purpose and syntax of the
 * associated options.  These strings are used by the `PrintOptionHelp()` function to generate
 * option usage information.
 *
 * By convention, option help strings consist of a syntax example following by a textual
 * description of the option.  If the option has a short version, or an alias name, it is given
 * before primary long name.  For consistency, syntax lines are indented with 2 spaces, while
 * description lines are indented with 7 spaces.  A single blank line follows each option
 * description, including the last one.
 *
 * E.g.:
 *
 *     static const char *const gToolOptionHelp =
 *         "  --listen\n"
 *         "       Listen and respond to requests sent from another node.\n"
 *         "\n"
 *         "  --length <num>\n"
 *         "       Send requests with the specified number of bytes in the payload.\n"
 *         "\n"
 *         "  --num, --count <num>\n"
 *         "       Send the specified number of requests and exit.\n"
 *         "\n"
 *         "  -d, --debug [<level>]\n"
 *         "       Set debug logging to the given level. (Default: 1)\n"
 *         "\n"
 *         "  -h, --help\n"
 *         "       Print help information.\n"
 *         "\n";
 *
 *
 * ## OPTION HELP GROUPS
 *
 * OptionSets contain a `HelpGroupName` string which is used to group options together in the
 * help output.  The `PrintOptionHelp()` function uses the HelpGroupName as a section title in
 * the generated usage output.  If multiple OptionSets have the same HelpGroupName,
 * PrintOptionHelp() will print the option help for the different OptionSets together under
 * a common section title.
 *
 */
bool ParseArgs(const char * progName, int argc, char * argv[], OptionSet * optSets[], NonOptionArgHandlerFunct nonOptArgHandler,
               bool ignoreUnknown)
{
    bool res = false;
    char optName[64];
    char * optArg;
    char * shortOpts         = nullptr;
    struct option * longOpts = nullptr;
    OptionSet * curOptSet;
    OptionDef * curOpt;
    bool handlerRes;

    // The getopt() functions do not support recursion, so exit immediately with an
    // error if called recursively.
    if (gActiveOptionSets != nullptr)
    {
        PrintArgError("INTERNAL ERROR: ParseArgs() called recursively\n", progName);
        return false;
    }

    // The C standard mandates that argv[argc] == NULL and certain versions of getopt() require this
    // to function properly.  So fail if this is not true.
    if (argv[argc] != nullptr)
    {
        PrintArgError("INTERNAL ERROR: argv[argc] != NULL\n", progName);
        return false;
    }

    // Set gActiveOptionSets to the current option set list.
    gActiveOptionSets = optSets;

#if CHIP_CONFIG_ENABLE_ARG_PARSER_SANTIY_CHECK
    if (!SanityCheckOptions(optSets))
        goto done;
#endif

    // Generate a short options string in the format expected by getopt_long().
    shortOpts = MakeShortOptions(optSets);
    if (shortOpts == nullptr)
    {
        PrintArgError("%s: Memory allocation failure\n", progName);
        goto done;
    }

    // Generate a list of long option structures in the format expected by getopt_long().
    longOpts = MakeLongOptions(optSets);
    if (longOpts == nullptr)
    {
        PrintArgError("%s: Memory allocation failure\n", progName);
        goto done;
    }

    // Force getopt() to reset its internal state.
    optind = 0;

    // Process any option arguments...
    while (true)
    {
        int id;
        int optIndex = -1;

        // Attempt to match the current option argument (argv[optind]) against the defined long and short options.
        optarg = nullptr;
        optopt = 0;
        id     = getopt_long(argc, argv, shortOpts, longOpts, &optIndex);

        // Stop if there are no more options.
        if (id == -1)
            break;

        // If the current option is unrecognized, fail with an error message unless ignoreUnknown == true.
        if (id == '?')
        {
            if (ignoreUnknown)
                continue;
            if (optopt != 0)
                PrintArgError("%s: Unknown option: -%c\n", progName, optopt);
            else
                PrintArgError("%s: Unknown option: %s\n", progName, argv[optind - 1]);
            goto done;
        }

        // If the option was recognized, but it is lacking an argument, fail with
        // an error message.
        if (id == ':')
        {
            // NOTE: with the way getopt_long() works, it is impossible to tell whether the option that
            // was missing an argument was a long option or a short option.
            PrintArgError("%s: Missing argument for %s option\n", progName, argv[optind - 1]);
            goto done;
        }

        // If a long option was matched...
        if (optIndex != -1)
        {

            // Locate the option set and definition using the index value returned by getopt_long().
            FindOptionByIndex(optSets, optIndex, curOptSet, curOpt);

            // Form a string containing the name of the option as it appears on the command line.
            snprintf(optName, sizeof(optName), "--%s", curOpt->Name);
        }

        // Otherwise a short option was matched...
        else
        {
            // Locate the option set and definition using the option id.
            FindOptionById(optSets, id, curOptSet, curOpt);

            // Form a string containing the name of the short option as it would appears on the
            // command line if given by itself.
            snprintf(optName, sizeof(optName), "-%c", id);
        }

        // Prevent handlers from inadvertently using the getopt global optarg.
        optArg = optarg;
        optarg = nullptr;

        // Call the option handler function defined for the matching option set.
        // Exit immediately if the option handler failed.
        handlerRes = curOptSet->OptionHandler(progName, curOptSet, id, optName, optArg);
        if (!handlerRes)
            goto done;
    }

    // If supplied, call the non-option argument handler with the remaining arguments (if any).
    if (nonOptArgHandler != nullptr)
    {
        if (!nonOptArgHandler(progName, argc - optind, argv + optind))
            goto done;
    }

    // otherwise, if there are additional arguments, fail with an error.
    else if (optind < argc)
    {
        PrintArgError("%s: Unexpected argument: %s\n", progName, argv[optind]);
        goto done;
    }

    res = true;

done:

    if (shortOpts != nullptr)
        chip::Platform::MemoryFree(shortOpts);
    if (longOpts != nullptr)
        chip::Platform::MemoryFree(longOpts);

    gActiveOptionSets = nullptr;

    return res;
}

bool ParseArgs(const char * progName, int argc, char * argv[], OptionSet * optSets[], NonOptionArgHandlerFunct nonOptArgHandler)
{
    return ParseArgs(progName, argc, argv, optSets, nonOptArgHandler, false);
}

bool ParseArgs(const char * progName, int argc, char * argv[], OptionSet * optSets[])
{
    return ParseArgs(progName, argc, argv, optSets, nullptr, false);
}

/**
 * @brief
 * Parse a set of arguments from a given string.
 *
 * @param[in]  progName             The name of the program or context in which the arguments are
 *                                  being parsed.  This string will be used to prefix error
 *                                  messages and warnings.
 * @param[in]  argStr               A string containing options and arguments to be parsed.
 * @param[in]  optSets              A list of pointers to `OptionSet` structures that define the legal
 *                                  options.  The supplied list must be terminated with a NULL.
 * @param[in]  nonOptArgHandler     A pointer to a function that will be called once option parsing
 *                                  is complete with any remaining non-option arguments .  The function
 *                                  is called regardless of whether any arguments remain.  If a NULL
 *                                  is passed `ParseArgs()` will report an error if any non-option
 *                                  arguments are present.
 * @param[in]  ignoreUnknown        If true, silently ignore any unrecognized options.
 *
 * @return                          `true` if all options and non-option arguments were parsed
 *                                  successfully; `false` if an option was unrecognized, if one of
 *                                  the handler functions failed (i.e. returned false) or if an
 *                                  internal error occurred.
 *
 * @details
 * ParseArgsFromString() splits a given string (`argStr`) into a set of arguments and parses the
 * arguments using the ParseArgs() function.
 *
 * The syntax of the input strings is similar to unix shell command syntax, but with a simplified
 * quoting scheme.  Specifically:
 *
 * - Arguments are delimited by whitespace, unless the whitespace is quoted or escaped.
 *
 * - A backslash escapes the following character, causing it to be treated as a normal character.
 * The backslash itself is stripped.
 *
 * - Single or double quotes begin/end quoted substrings.  Within a substring, the only special
 * characters are backslash, which escapes the next character, and the corresponding end quote.
 * The begin/end quote characters are stripped.
 *
 * E.g.:
 *
 *     --listen --count 10 --sw-version '1.0 (DEVELOPMENT)' "--hostname=nest.com"
 *
 */
bool ParseArgsFromString(const char * progName, const char * argStr, OptionSet * optSets[],
                         NonOptionArgHandlerFunct nonOptArgHandler, bool ignoreUnknown)
{
    char ** argv = nullptr;
    int argc;
    bool res;

    chip::Platform::ScopedMemoryString argStrCopy(argStr, strlen(argStr));
    if (!argStrCopy)
    {
        PrintArgError("%s: Memory allocation failure\n", progName);
        return false;
    }

    argc = SplitArgs(argStrCopy.Get(), argv, const_cast<char *>(progName));
    if (argc < 0)
    {
        PrintArgError("%s: Memory allocation failure\n", progName);
        return false;
    }

    res = ParseArgs(progName, argc, argv, optSets, nonOptArgHandler, ignoreUnknown);

    chip::Platform::MemoryFree(argv);

    return res;
}

bool ParseArgsFromString(const char * progName, const char * argStr, OptionSet * optSets[],
                         NonOptionArgHandlerFunct nonOptArgHandler)
{
    return ParseArgsFromString(progName, argStr, optSets, nonOptArgHandler, false);
}

bool ParseArgsFromString(const char * progName, const char * argStr, OptionSet * optSets[])
{
    return ParseArgsFromString(progName, argStr, optSets, nullptr, false);
}

/**
 * @brief
 * Parse a set of arguments from a named environment variable
 *
 * @param[in]  progName             The name of the program or context in which the arguments are
 *                                  being parsed.  This string will be used to prefix error
 *                                  messages and warnings.
 * @param[in]  varName              The name of the environment variable.
 * @param[in]  optSets              A list of pointers to `OptionSet` structures that define the legal
 *                                  options.  The supplied list must be terminated with a NULL.
 * @param[in]  nonOptArgHandler     A pointer to a function that will be called once option parsing
 *                                  is complete with any remaining non-option arguments .  The function
 *                                  is called regardless of whether any arguments remain.  If a NULL
 *                                  is passed `ParseArgs()` will report an error if any non-option
 *                                  arguments are present.
 * @param[in]  ignoreUnknown        If true, silently ignore any unrecognized options.
 *
 * @return                          `true` if all options and non-option arguments were parsed
 *                                  successfully, or if the specified environment variable is not set;
 *                                  `false` if an option was unrecognized, if one of the handler
 *                                  functions failed (i.e. returned false) or if an internal error
 *                                  occurred.
 *
 * @details
 * ParseArgsFromEnvVar() reads a named environment variable and passes the value to `ParseArgsFromString()`
 * for parsing.  If the environment variable is not set, the function does nothing.
 */

bool ParseArgsFromEnvVar(const char * progName, const char * varName, OptionSet * optSets[],
                         NonOptionArgHandlerFunct nonOptArgHandler, bool ignoreUnknown)
{
    const char * argStr = getenv(varName);
    if (argStr == nullptr)
        return true;
    return ParseArgsFromString(progName, argStr, optSets, nonOptArgHandler, ignoreUnknown);
}

bool ParseArgsFromEnvVar(const char * progName, const char * varName, OptionSet * optSets[])
{
    return ParseArgsFromEnvVar(progName, varName, optSets, nullptr, false);
}

bool ParseArgsFromEnvVar(const char * progName, const char * varName, OptionSet * optSets[],
                         NonOptionArgHandlerFunct nonOptArgHandler)
{
    return ParseArgsFromEnvVar(progName, varName, optSets, nonOptArgHandler, false);
}

/**
 * @brief
 * Print the help text for a specified list of options to a stream.
 *
 * @param[in]  optSets              A list of pointers to `OptionSet` structures that contain the
 *                                  help text to print.
 * @param[in]  s                    The FILE stream to which the help text should be printed.
 *
 */
void PrintOptionHelp(OptionSet * optSets[], FILE * s)
{
    // Get a list of the unique help group names for the given option sets.
    const char ** helpGroupNames = MakeUniqueHelpGroupNamesList(optSets);
    if (helpGroupNames == nullptr)
    {
        PrintArgError("Memory allocation failure\n");
        return;
    }

    // For each help group...
    for (size_t nameIndex = 0; helpGroupNames[nameIndex] != nullptr; nameIndex++)
    {
        // Print the group name.
        PutStringWithBlankLine(s, helpGroupNames[nameIndex]);

        // Print the option help text for all options that have the same group name.
        for (size_t optSetIndex = 0; optSets[optSetIndex] != nullptr; optSetIndex++)
            if (strcasecmp(helpGroupNames[nameIndex], optSets[optSetIndex]->HelpGroupName) == 0)
            {
                PutStringWithBlankLine(s, optSets[optSetIndex]->OptionHelp);
            }
    }

    chip::Platform::MemoryFree(helpGroupNames);
}

/**
 * @brief
 * Print an error message associated with argument parsing.
 *
 * @param[in]  msg   The message to be printed.
 *
 * @details
 * Default function used to print error messages that arise due to the parsing
 * of arguments.
 *
 * Applications should call through the PrintArgError function pointer, rather
 * than calling this function directly.
 */
void DefaultPrintArgError(const char * msg, ...)
{
    va_list ap;

    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    va_end(ap);
}

/**
 * Parse a string as a boolean value.
 *
 * This function accepts the following input values (case-insensitive):
 * "true", "yes", "t", "y", "1", "false", "no", "f", "n", "0".
 *
 * @param[in]  str    A pointer to a NULL-terminated C string representing
 *                    the value to parse.
 * @param[out] output A reference to storage for a bool to which the parsed
 *                    value will be stored on success.
 *
 * @return true on success; otherwise, false on failure.
 */
bool ParseBoolean(const char * str, bool & output)
{
    if (strcasecmp(str, "true") == 0 || strcasecmp(str, "yes") == 0 ||
        ((str[0] == '1' || str[0] == 't' || str[0] == 'T' || str[0] == 'y' || str[0] == 'Y') && str[1] == 0))
    {
        output = true;
        return true;
    }

    if (strcasecmp(str, "false") == 0 || strcasecmp(str, "no") == 0 ||
        ((str[0] == '0' || str[0] == 'f' || str[0] == 'F' || str[0] == 'n' || str[0] == 'N') && str[1] == 0))
    {
        output = false;
        return true;
    }

    return false;
}

/**
 * Parse and attempt to convert a string to a 64-bit unsigned integer,
 * applying the appropriate interpretation based on the base parameter.
 *
 * @param[in]  str    A pointer to a NULL-terminated C string representing
 *                    the integer to parse.
 * @param[out] output A reference to storage for a 64-bit unsigned integer
 *                    to which the parsed value will be stored on success.
 * @param[in]  base   The base according to which the string should be
 *                    interpreted and parsed. If 0 or 16, the string may
 *                    be hexadecimal and prefixed with "0x". Otherwise, a 0
 *                    is implied as 10 unless a leading 0 is encountered in
 *                    which 8 is implied.
 *
 * @return true on success; otherwise, false on failure.
 */
bool ParseInt(const char * str, uint64_t & output, int base)
{
    char * parseEnd;

    errno  = 0;
    output = strtoull(str, &parseEnd, base);

    return parseEnd > str && *parseEnd == 0 && (output != ULLONG_MAX || errno == 0);
}

/**
 * Parse and attempt to convert a string to a 32-bit unsigned integer,
 * applying the appropriate interpretation based on the base parameter.
 *
 * @param[in]  str    A pointer to a NULL-terminated C string representing
 *                    the integer to parse.
 * @param[out] output A reference to storage for a 32-bit unsigned integer
 *                    to which the parsed value will be stored on success.
 * @param[in]  base   The base according to which the string should be
 *                    interpreted and parsed. If 0 or 16, the string may
 *                    be hexadecimal and prefixed with "0x". Otherwise, a 0
 *                    is implied as 10 unless a leading 0 is encountered in
 *                    which 8 is implied.
 *
 * @return true on success; otherwise, false on failure.
 */
bool ParseInt(const char * str, uint32_t & output, int base)
{
    char * parseEnd;
    unsigned long v;

    errno = 0;
    v     = strtoul(str, &parseEnd, base);
    if (!CanCastTo<uint32_t>(v))
    {
        return false;
    }
    output = static_cast<uint32_t>(v);

    return parseEnd > str && *parseEnd == 0 && (v != ULONG_MAX || errno == 0);
}

/**
 * Parse and attempt to convert a string to a 32-bit signed integer,
 * applying the appropriate interpretation based on the base parameter.
 *
 * @param[in]  str    A pointer to a NULL-terminated C string representing
 *                    the integer to parse.
 * @param[out] output A reference to storage for a 32-bit signed integer
 *                    to which the parsed value will be stored on success.
 * @param[in]  base   The base according to which the string should be
 *                    interpreted and parsed. If 0 or 16, the string may
 *                    be hexadecimal and prefixed with "0x". Otherwise, a 0
 *                    is implied as 10 unless a leading 0 is encountered in
 *                    which 8 is implied.
 *
 * @return true on success; otherwise, false on failure.
 */
bool ParseInt(const char * str, int32_t & output, int base)
{
    char * parseEnd;
    long v;

    errno = 0;
    v     = strtol(str, &parseEnd, base);
    if (!CanCastTo<int32_t>(v))
    {
        return false;
    }
    output = static_cast<int32_t>(v);

    return parseEnd > str && *parseEnd == 0 && ((v != LONG_MIN && v != LONG_MAX) || errno == 0);
}

/**
 * Parse and attempt to convert a string interpreted as a decimal
 * value to a 64-bit unsigned integer, applying the appropriate
 * interpretation based on the base parameter.
 *
 * @param[in]  str    A pointer to a NULL-terminated C string representing
 *                    the integer to parse.
 * @param[out] output A reference to storage for a 64-bit unsigned integer
 *                    to which the parsed value will be stored on success.
 *
 * @return true on success; otherwise, false on failure.
 */
bool ParseInt(const char * str, uint64_t & output)
{
    const int base = 10;

    return ParseInt(str, output, base);
}

/**
 * Parse and attempt to convert a string interpreted as a decimal
 * value to a 32-bit unsigned integer, applying the appropriate
 * interpretation based on the base parameter.
 *
 * @param[in]  str    A pointer to a NULL-terminated C string representing
 *                    the integer to parse.
 * @param[out] output A reference to storage for a 32-bit unsigned integer
 *                    to which the parsed value will be stored on success.
 *
 * @return true on success; otherwise, false on failure.
 */
bool ParseInt(const char * str, uint32_t & output)
{
    const int base = 10;

    return ParseInt(str, output, base);
}

/**
 * Parse and attempt to convert a string interpreted as a decimal
 * value to a 32-bit signed integer, applying the appropriate
 * interpretation based on the base parameter.
 *
 * @param[in]  str    A pointer to a NULL-terminated C string representing
 *                    the integer to parse.
 * @param[out] output A reference to storage for a 32-bit signed integer
 *                    to which the parsed value will be stored on success.
 *
 * @return true on success; otherwise, false on failure.
 */
bool ParseInt(const char * str, int32_t & output)
{
    const int base = 10;

    return ParseInt(str, output, base);
}

/**
 * Parse and attempt to convert a string interpreted as a decimal
 * value to a 16-bit unsigned integer, applying the appropriate
 * interpretation based on the base parameter.
 *
 * @param[in]  str    A pointer to a NULL-terminated C string representing
 *                    the integer to parse.
 * @param[out] output A reference to storage for a 16-bit unsigned integer
 *                    to which the parsed value will be stored on success.
 *
 * @return true on success; otherwise, false on failure.
 */
bool ParseInt(const char * str, uint16_t & output)
{
    const int base    = 10;
    uint32_t output32 = 0;

    if ((ParseInt(str, output32, base)) && (output32 <= USHRT_MAX))
    {
        output = ((1 << 16) - 1) & output32;
        return true;
    }

    return false;
}

/**
 * Parse and attempt to convert a string interpreted as a decimal
 * value to a 16-bit signed integer, applying the appropriate
 * interpretation based on the base parameter.
 *
 * @param[in]  str    A pointer to a NULL-terminated C string representing
 *                    the integer to parse.
 * @param[out] output A reference to storage for a 16-bit signed integer
 *                    to which the parsed value will be stored on success.
 *
 * @return true on success; otherwise, false on failure.
 */
bool ParseInt(const char * str, int16_t & output)
{
    const int base   = 10;
    int32_t output32 = 0;

    if ((ParseInt(str, output32, base)) && (output32 <= SHRT_MAX))
    {
        output = static_cast<int16_t>(UINT16_MAX & output32);
        return true;
    }

    return false;
}

/**
 * Parse and attempt to convert a string interpreted as a decimal
 * value to a 8-bit unsigned integer, applying the appropriate
 * interpretation based on the base parameter.
 *
 * @param[in]  str    A pointer to a NULL-terminated C string representing
 *                    the integer to parse.
 * @param[out] output A reference to storage for a 8-bit unsigned integer
 *                    to which the parsed value will be stored on success.
 *
 * @return true on success; otherwise, false on failure.
 */
bool ParseInt(const char * str, uint8_t & output)
{
    const int base    = 10;
    uint32_t output32 = 0;

    if ((ParseInt(str, output32, base)) && (output32 <= UCHAR_MAX))
    {
        output = ((1 << 8) - 1) & output32;
        return true;
    }

    return false;
}

#if CHIP_ARG_PARSER_PARSE_NODE_ID
/**
 * Parse a CHIP node id in text form.
 *
 * @param[in]  str    A pointer to a NULL-terminated C string containing
 *                    the node id to parse.
 * @param[out] output A reference to an uint64_t lvalue in which the parsed
 *                    value will be stored on success.
 *
 * @return true if the value was successfully parsed; false if not.
 *
 * @details
 * The ParseNodeId() function accepts either a 64-bit node id given in hex
 * format (with or without a leading '0x'), or the words 'any' or 'all' which
 * are interpreted as meaning the Any node id (0xFFFFFFFFFFFFFFFF).
 */
bool ParseNodeId(const char * str, uint64_t & nodeId)
{
    char * parseEnd;

    if (strcasecmp(str, "any") == 0 || strcasecmp(str, "all") == 0)
    {
        nodeId = kAnyNodeId;
        return true;
    }

    errno  = 0;
    nodeId = strtoull(str, &parseEnd, 16);
    return parseEnd > str && *parseEnd == 0 && (nodeId != ULLONG_MAX || errno == 0);
}
#endif // CHIP_ARG_PARSER_PARSE_NODE_ID

#if CHIP_ARG_PARSER_PARSE_FABRIC_ID
/**
 * Parse a CHIP fabric id in text form.
 *
 * @param[in]  str              A pointer to a NULL-terminated C string containing
 *                              the fabric id to parse.
 * @param[out] output           A reference to an uint64_t lvalue in which the
 *                              parsed value will be stored on success.
 * @param[in]  allowReserved    If true, allow the parsing of fabric ids in the
 *                              reserved range.
 *
 * @return true if the value was successfully parsed; false if not.
 *
 * @details
 * The ParseFabricId() function accepts a 64-bit fabric id given in hex format,
 * with or without a leading '0x'.
 */
bool ParseFabricId(const char * str, uint64_t & fabricId, bool allowReserved)
{
    char * parseEnd;

    errno    = 0;
    fabricId = strtoull(str, &parseEnd, 16);
    return parseEnd > str && *parseEnd == 0 && (fabricId != ULLONG_MAX || errno == 0) &&
        (allowReserved || fabricId < kReservedFabricIdStart);
}
#endif // CHIP_ARG_PARSER_PARSE_FABRIC_ID

/**
 * Parse and attempt to convert a string to a 16-bit unsigned subnet
 * ID, interpretting the string as hexadecimal.
 *
 * @param[in]     str       A pointer to a NULL-terminated C string
 *                          representing the subnet ID, formatted as a
 *                          hexadecimal, to parse.
 * @param[in,out] subnetId  A reference to storage for a 16-bit unsigned
 *                          integer to which the parsed subnet ID value
 *                          will be stored on success.
 *
 * @return true on success; otherwise, false on failure.
 */
bool ParseSubnetId(const char * str, uint16_t & subnetId)
{
    char * parseEnd;
    unsigned long temp;
    bool valid;

    // Reset errno per the strtoul manual page.

    errno = 0;

    // Attempt to parse the subnet ID as a hexadecimal number.

    temp = strtoul(str, &parseEnd, 16);

    // Determine if the parse and conversion were valid.

    valid = (parseEnd > str &&                    // Parsed some valid hexadecimal digits
             *parseEnd == 0 &&                    // Encountered no invalid hexadecimal digits
             (temp != ULONG_MAX || errno == 0) && // No overflow (ERANGE) or invalid base (EINVAL) errors
             temp <= USHRT_MAX);                  // Parsed value is valid for the domain (subnet ID)

    if (valid)
    {
        subnetId = static_cast<uint16_t>(temp);
    }

    return valid;
}

/**
 * Parse a string of bytes given in hex form.
 *
 * @param[in]  hexStr           A pointer to the string to parse.
 * @param[in]  strLen           The number of characters in hexStr to parse.
 * @param[in]  outBuf           A pointer to a buffer into which the parse bytes will
 *                              be stored.
 * @param[in]  outBufSize       The size of the buffer pointed at by `outBuf`.
 * @param[out] outDataLen       A reference to an integer that will receive the total
 *                              number of bytes parsed.  In the event outBuf is not
 *                              big enough to hold the given number of bytes, `outDataLen`
 *                              will be set to UINT32_MAX.
 *
 * @return true if the value was successfully parsed; false if the input data is malformed,
 * or if `outBuf` is too small.
 *
 * @details
 * ParseHexString() expects the input to be in the form of pairs of hex digits (upper or lower case).
 * Hex pairs can optionally be separated by any of the following characters: colon, semicolon, comma, period or dash.
 * Additionally, whitespace characters anywhere in the input string are ignored.
 */
bool ParseHexString(const char * hexStr, uint32_t strLen, uint8_t * outBuf, uint32_t outBufSize, uint32_t & outDataLen)
{
    bool isFirstNibble     = true;
    uint8_t firstNibbleVal = 0;
    const char * p         = hexStr;
    uint32_t dataLen       = 0;

    outDataLen = 0;

    for (; strLen > 0; p++, strLen--)
    {
        char c = *p;
        uint8_t nibbleVal;

        if (c == 0)
            break;
        if (c >= '0' && c <= '9')
            nibbleVal = static_cast<uint8_t>(c - '0');
        else if (c >= 'a' && c <= 'f')
            nibbleVal = static_cast<uint8_t>(10 + (c - 'a'));
        else if (c >= 'A' && c <= 'F')
            nibbleVal = static_cast<uint8_t>(10 + (c - 'A'));
        else if (isspace(c))
            continue;
        else if (isFirstNibble && (c == ':' || c == ';' || c == ',' || c == '.' || c == '-'))
            continue;
        else
        {
            outDataLen = static_cast<decltype(strLen)>(p - hexStr);
            return false;
        }

        if (isFirstNibble)
        {
            firstNibbleVal = nibbleVal;
            isFirstNibble  = false;
        }
        else
        {
            if (outBufSize == 0)
            {
                outDataLen = UINT32_MAX;
                return false;
            }

            *outBuf = static_cast<uint8_t>(firstNibbleVal << 4 | nibbleVal);

            outBuf++;
            outBufSize--;
            dataLen++;

            isFirstNibble = true;
        }
    }

    if (!isFirstNibble)
    {
        outDataLen = static_cast<decltype(strLen)>(p - hexStr);
        return false;
    }

    outDataLen = dataLen;

    return true;
}

// ===== HelpOptions Methods =====

HelpOptions::HelpOptions(const char * appName, const char * appUsage, const char * appVersion) :
    HelpOptions(appName, appUsage, appVersion, nullptr)
{}

HelpOptions::HelpOptions(const char * appName, const char * appUsage, const char * appVersion, const char * appDesc)
{
    // clang-format off
    static OptionDef optionDefs[] =
    {
        { "help",      kNoArgument, 'h' },
        { "version",   kNoArgument, 'v' },
        { }
    };
    // clang-format on
    OptionDefs = optionDefs;

    HelpGroupName = "HELP OPTIONS";

    OptionHelp = "  -h, --help\n"
                 "       Print this output and then exit.\n"
                 "\n"
                 "  -v, --version\n"
                 "       Print the version and then exit.\n"
                 "\n";

    AppName    = appName;
    AppUsage   = appUsage;
    AppVersion = appVersion;
    AppDesc    = appDesc;
}

/**
 * Print a short description of the command's usage followed by instructions on how to get more help.
 */
void HelpOptions::PrintBriefUsage(FILE * s)
{
    PutStringWithNewLine(s, AppUsage);
    fprintf(s, "Try `%s --help' for more information.\n", AppName);
}

/**
 * Print the full usage information, including information on all available options.
 */
void HelpOptions::PrintLongUsage(OptionSet ** optSets, FILE * s)
{
    PutStringWithBlankLine(s, AppUsage);
    if (AppDesc != nullptr)
    {
        PutStringWithBlankLine(s, AppDesc);
    }
    PrintOptionHelp(optSets, s);
}

void HelpOptions::PrintVersion(FILE * s)
{
    fprintf(s, "%s ", AppName);
    PutStringWithNewLine(s, (AppVersion != nullptr) ? AppVersion : "(unknown version)");
}

bool HelpOptions::HandleOption(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg)
{
    switch (id)
    {
    case 'h':
        PrintLongUsage(gActiveOptionSets, stdout);
        exit(EXIT_SUCCESS);
        break;
    case 'v':
        PrintVersion(stdout);
        exit(EXIT_SUCCESS);
        break;
    default:
        PrintArgError("%s: INTERNAL ERROR: Unhandled option: %s\n", progName, name);
        return false;
    }

    return true;
}

// ===== Private/Internal Methods =====

OptionSetBase::OptionSetBase()
{
    OptionHandler = CallHandleFunct;
}

bool OptionSetBase::CallHandleFunct(const char * progName, OptionSet * optSet, int id, const char * name, const char * arg)
{
    return static_cast<OptionSetBase *>(optSet)->HandleOption(progName, optSet, id, name, arg);
}

static char * MakeShortOptions(OptionSet ** optSets)
{
    size_t i = 0;

    // Count the number of options.
    size_t totalOptions = CountAllOptions(optSets);

    // Allocate a block of memory big enough to hold the maximum possible size short option string.
    // The buffer needs to be big enough to hold up to 3 characters per short option plus an initial
    // ":" and a terminating null.
    size_t arraySize = 2 + (totalOptions * 3);
    char * shortOpts = static_cast<char *>(chip::Platform::MemoryAlloc(arraySize));
    if (shortOpts == nullptr)
        return nullptr;

    // Prefix the string with ':'.  This tells getopt() to signal missing option arguments distinct
    // from unknown options.
    shortOpts[i++] = ':';

    // For each option set...
    for (; *optSets != nullptr; optSets++)
    {
        // For each option in the current option set...
        for (OptionDef * optDef = (*optSets)->OptionDefs; optDef->Name != nullptr; optDef++)
        {
            // If the option id (val) is suitable as a short option character, add it to the short
            // option string. Append ":" if the option requires an argument and "::" if the argument
            // is optional.
            if (IsShortOptionChar(optDef->Id))
            {
                shortOpts[i++] = static_cast<char>(optDef->Id);
                if (optDef->ArgType != kNoArgument)
                    shortOpts[i++] = ':';
                if (optDef->ArgType == kArgumentOptional)
                    shortOpts[i++] = ':';
            }
        }
    }

    // Terminate the short options string.
    shortOpts[i++] = 0;

    return shortOpts;
}

static struct option * MakeLongOptions(OptionSet ** optSets)
{
    size_t totalOptions = CountAllOptions(optSets);

    // Allocate an array to hold the list of long options.
    size_t arraySize         = (sizeof(struct option) * (totalOptions + 1));
    struct option * longOpts = static_cast<struct option *>(chip::Platform::MemoryAlloc(arraySize));
    if (longOpts == nullptr)
        return nullptr;

    // For each option set...
    size_t i = 0;
    for (; *optSets != nullptr; optSets++)
    {
        // Copy the option definitions into the long options array.
        for (OptionDef * optDef = (*optSets)->OptionDefs; optDef->Name != nullptr; optDef++)
        {
            longOpts[i].name    = optDef->Name;
            longOpts[i].has_arg = static_cast<int>(optDef->ArgType);
            longOpts[i].flag    = nullptr;
            longOpts[i].val     = optDef->Id;
            i++;
        }
    }

    // Terminate the long options array.
    longOpts[i].name = nullptr;

    return longOpts;
}

static int32_t SplitArgs(char * argStr, char **& argList, char * initialArg)
{
    enum
    {
        InitialArgListSize = 10
    };
    size_t argListSize = 0;
    int32_t argCount   = 0;

    // Allocate an array to hold pointers to the arguments.
    argList = static_cast<char **>(chip::Platform::MemoryAlloc(sizeof(char *) * InitialArgListSize));
    if (argList == nullptr)
        return -1;
    argListSize = InitialArgListSize;

    // If an initial argument was supplied, make it the first argument in the array.
    if (initialArg != nullptr)
    {
        argList[0] = initialArg;
        argCount   = 1;
    }

    // Parse arguments from the input string until it is exhausted.
    while (true)
    {
        char * nextArg = argStr;

        // Get the argument in the input string.  Note that this modifies the string buffer.
        if (!GetNextArg(argStr))
            break;

        // Grow the arg list array if needed. Note that we reserve one slot at the end of the array
        // for a NULL entry.
        if (argListSize == static_cast<size_t>(argCount + 1))
        {
            argListSize *= 2;
            argList = static_cast<char **>(chip::Platform::MemoryRealloc(argList, argListSize));
            if (argList == nullptr)
                return -1;
        }

        // Append the argument.
        argList[argCount++] = nextArg;
    }

    // Set the last element in the array to NULL, but do not include this in the count of elements.
    // This is mandated by the C standard and some versions of getopt_long() depend on it.
    argList[argCount] = nullptr;

    return argCount;
}

static bool GetNextArg(char *& parsePoint)
{
    char quoteChar = 0;
    char * argEnd  = parsePoint;

    // Skip any leading whitespace.
    while (*parsePoint != 0 && isspace(*parsePoint))
        parsePoint++;

    // Return false if there are no further arguments.
    if (*parsePoint == 0)
        return false;

    // Iterate over characters until we find the end of an argument.
    // As we iterate, we will accumulate the unquoted and unescaped
    // argument characters in the input buffer starting at the initial
    // parsePoint position.
    while (*parsePoint != 0)
    {
        // If the current character is a backslash that is not at the end of
        // the string, skip the backslash but copy the following character
        // verbatim into the argument string.
        if (*parsePoint == '\\' && *(parsePoint + 1) != 0)
        {
            parsePoint++;
        }

        // Otherwise, if not within a quoted substring...
        else if (quoteChar == 0)
        {
            // Whitespace marks the end of the argument.
            if (isspace(*parsePoint))
            {
                parsePoint++;
                break;
            }

            // If the character is a quote character, enter quoted substring mode.
            if (*parsePoint == '"' || *parsePoint == '\'')
            {
                quoteChar = *parsePoint++;
                continue;
            }
        }

        // Otherwise, the parse point is within a quoted substring, so...
        else
        {
            // A corresponding quote character marks the end of the quoted string.
            if (*parsePoint == quoteChar)
            {
                quoteChar = 0;
                parsePoint++;
                continue;
            }
        }

        // Copy the current character to the end of the argument string.
        *argEnd++ = *parsePoint++;
    }

    // Terminate the argument string.
    *argEnd = 0;

    return true;
}

static size_t CountOptionSets(OptionSet ** optSets)
{
    size_t count = 0;
    for (; *optSets != nullptr; optSets++)
        count++;
    return count;
}

static size_t CountAllOptions(OptionSet ** optSets)
{
    size_t count = 0;
    for (; *optSets != nullptr; optSets++)
        for (OptionDef * optDef = (*optSets)->OptionDefs; optDef->Name != nullptr; optDef++)
            count++;
    return count;
}

static void FindOptionByIndex(OptionSet ** optSets, int optIndex, OptionSet *& optSet, OptionDef *& optDef)
{
    for (optSet = *optSets; optSet != nullptr; optSet = *++optSets)
        for (optDef = (*optSets)->OptionDefs; optDef->Name != nullptr; optDef++)
            if (optIndex-- == 0)
                return;
    optSet = nullptr;
    optDef = nullptr;
}

static void FindOptionById(OptionSet ** optSets, int optId, OptionSet *& optSet, OptionDef *& optDef)
{
    for (optSet = *optSets; optSet != nullptr; optSet = *++optSets)
        for (optDef = (*optSets)->OptionDefs; optDef->Name != nullptr; optDef++)
            if (optDef->Id == optId)
                return;
    optSet = nullptr;
    optDef = nullptr;
}

static const char ** MakeUniqueHelpGroupNamesList(OptionSet * optSets[])
{
    size_t numOptSets = CountOptionSets(optSets);
    size_t numGroups  = 0;

    const char ** groupNames = static_cast<const char **>(chip::Platform::MemoryAlloc(sizeof(const char *) * (numOptSets + 1)));
    if (groupNames == nullptr)
        return nullptr;

    for (size_t optSetIndex = 0; optSetIndex < numOptSets; optSetIndex++)
    {
        if (optSets[optSetIndex] != nullptr && optSets[optSetIndex]->OptionDefs[0].Name != nullptr)
        {
            for (size_t i = 0; i < numGroups; i++)
                if (strcasecmp(groupNames[i], optSets[optSetIndex]->HelpGroupName) == 0)
                    goto skipDup;
            groupNames[numGroups++] = optSets[optSetIndex]->HelpGroupName;
        skipDup:;
        }
    }

    groupNames[numGroups] = nullptr;

    return groupNames;
}

static void PutStringWithNewLine(FILE * s, const char * str)
{
    size_t strLen = strlen(str);
    fputs(str, s);
    if (strLen == 0 || str[strLen - 1] != '\n')
        fputs("\n", s);
}

static void PutStringWithBlankLine(FILE * s, const char * str)
{
    size_t strLen = strlen(str);
    fputs(str, s);
    if (strLen < 1 || str[strLen - 1] != '\n')
        fputs("\n", s);
    if (strLen < 2 || str[strLen - 2] != '\n')
        fputs("\n", s);
}

#if CHIP_CONFIG_ENABLE_ARG_PARSER_SANTIY_CHECK

static bool SanityCheckOptions(OptionSet * optSets[])
{
    bool res = true;

    // Verify OptionHandler pointer
    for (OptionSet ** optSetP = optSets; *optSetP != nullptr; optSetP++)
    {
        if ((*optSetP)->OptionHandler == nullptr)
        {
            PrintArgError("INTERNAL ERROR: Null OptionHandler in OptionSet (%s)\n", (*optSetP)->HelpGroupName);
            res = false;
        }
    }

    // Verify that no two option sets use the same short option character.
    // (Re-use of the same short option character is allowed within a single option set
    // to allow for aliasing of long options).
    for (OptionSet ** optSetP = optSets; *optSetP != nullptr; optSetP++)
        for (OptionDef * optionDef = (*optSetP)->OptionDefs; optionDef->Name != nullptr; optionDef++)
            if (IsShortOptionChar(optionDef->Id))
            {
                for (OptionSet ** optSetP2 = optSets; *optSetP2 != nullptr; optSetP2++)
                    if (optSetP2 != optSetP)
                    {
                        for (OptionDef * optionDef2 = (*optSetP2)->OptionDefs; optionDef2->Name != nullptr; optionDef2++)
                            if (optionDef->Id == optionDef2->Id)
                            {
                                PrintArgError("INTERNAL ERROR: Multiple command line options configured to use "
                                              "the same short option character (-%c): --%s, --%s\n",
                                              optionDef->Id, optionDef->Name, optionDef2->Name);
                                res = false;
                            }
                    }
            }

    // Fail if the option help texts do not contain a description for each option, including both
    // the option's long and short forms.
    for (OptionSet ** optSetP = optSets; *optSetP != nullptr; optSetP++)
        for (OptionDef * optionDef = (*optSetP)->OptionDefs; optionDef->Name != nullptr; optionDef++)
        {
            if (!HelpTextContainsLongOption(optionDef->Name, (*optSetP)->OptionHelp))
            {
                PrintArgError("INTERNAL ERROR: No help text defined for option: --%s\n", optionDef->Name);
                res = false;
            }

            if (IsShortOptionChar(optionDef->Id) &&
                !HelpTextContainsShortOption(static_cast<char>(optionDef->Id), (*optSetP)->OptionHelp))
            {
                PrintArgError("INTERNAL ERROR: No help text defined for option: -%c\n", optionDef->Id);
                res = false;
            }
        }

    return res;
}

static bool HelpTextContainsLongOption(const char * optName, const char * helpText)
{
    size_t nameLen = strlen(optName);

    for (const char * p = helpText; (p = strstr(p, optName)) != nullptr; p += nameLen)
        if ((p - helpText) >= 2 && p[-1] == '-' && p[-2] == '-' && !isalnum(p[nameLen]) && p[nameLen] != '-')
            return true;

    return false;
}

static bool HelpTextContainsShortOption(char optChar, const char * helpText)
{
    char optStr[3];
    optStr[0] = '-';
    optStr[1] = optChar;
    optStr[2] = 0;

    for (const char * p = helpText; (p = strstr(p, optStr)) != nullptr; p += 2)
        if ((p == helpText || (!isalnum(p[-1]) && p[-1] != '-')) && !isalnum(p[2]) && p[2] != '-')
            return true;

    return false;
}

#endif // CHIP_CONFIG_ENABLE_ARG_PARSER_SANTIY_CHECK

} // namespace ArgParser
} // namespace chip

#endif // CHIP_CONFIG_ENABLE_ARG_PARSER
