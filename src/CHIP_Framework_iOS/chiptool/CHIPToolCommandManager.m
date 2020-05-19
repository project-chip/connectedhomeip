//
//  RegisterToolToolTasks.m
//  chiptool
//
//  Created by Mike Fullerton on 5/21/17.
//

#import "CHIPToolCommandManager.h"

typedef void (^CommandListVisitor)(chiptool_command_t * command, NSInteger idx, BOOL * stop);

char * _CHIPCreateStringForCommand(NSString * fromString);
NSInteger CHIPLegacyCommandListCount(const chiptool_command_t * list);

void CHIPToolVisitEachCommandInLegacyCommandList(const chiptool_command_t * list, CommandListVisitor visitor);

char * _CHIPCreateStringForCommand(NSString * fromString)
{
    if (fromString) {
        const char * c_string = [fromString cStringUsingEncoding:NSUTF8StringEncoding];
        if (c_string) {
            NSInteger len = [fromString length] + 1;
            char * newString = malloc(len);
            strncpy(newString, c_string, len);
            return newString;
        }
    }

    return nil;
}

void CHIPToolVisitEachCommandInLegacyCommandList(const chiptool_command_t * list, CommandListVisitor visitor)
{
    if (visitor) {
        BOOL stop = NO;
        NSInteger idx = 0;
        for (chiptool_command_t * c = (chiptool_command_t *) list; c->c_name; ++c) {
            visitor(c, idx++, &stop);
            if (stop) {
                break;
            }
        }
    }
}

NSInteger CHIPLegacyCommandListCount(const chiptool_command_t * list)
{
    __block NSInteger count = 0;
    CHIPToolVisitEachCommandInLegacyCommandList(list, ^(chiptool_command_t * command, NSInteger idx, BOOL * stop) {
        count++;
    });
    return count;
}

@interface CHIPToolCommandManager ()
@end

@implementation CHIPToolCommandManager {
    chiptool_command_t * _allCommands;
}

+ (instancetype)sharedInstance
{
    static CHIPToolCommandManager * manager = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        manager = [[CHIPToolCommandManager alloc] init];
    });
    return manager;
}

- (void)dealloc
{
    free(_allCommands);
    _allCommands = nil;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        _allCommands = nil;
    }
    return self;
}

- (int)executeCommand:(const chiptool_command_t *)command argc:(int)argc argv:(char * const *)argv
{
    if (command) {
        if (command->c_func) {
            return command->c_func(argc, argv);
        }
    }
    return -1;
}

- (void)configureWithLegacyCommands:(const chiptool_command_t *)legacyCommands
{

    NSInteger totalCount = CHIPLegacyCommandListCount(legacyCommands);
    if (totalCount > 0) {
        _allCommands = calloc(totalCount, sizeof(chiptool_command_t));
        __block NSInteger lastIndex = 0;
        CHIPToolVisitEachCommandInLegacyCommandList(legacyCommands, ^(chiptool_command_t * command, NSInteger idx, BOOL * stop) {
            self->_allCommands[idx].c_name = command->c_name;
            self->_allCommands[idx].c_func = command->c_func;
            self->_allCommands[idx].c_help = command->c_help;
            self->_allCommands[idx].c_usage = command->c_usage;
            lastIndex = idx;
        });
    }
}

@end

void CHIPPrintf(NSString * format, ...)
{
    @autoreleasepool {
        va_list args;
        va_start(args, format);
        NSString * message = [[NSString alloc] initWithFormat:format arguments:args];
        printf("%s\n", [message UTF8String]);

        fflush(stdout);

        va_end(args);
    }
}
