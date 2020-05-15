//
//  RegisterToolToolTasks.h
//  chiptool
//
//  Created by Mike Fullerton on 5/21/17.
//

#import <Foundation/Foundation.h>

extern void CHIPPrintf(NSString * format, ...) NS_FORMAT_FUNCTION(1, 2);

typedef int (*command_func)(int argc, char * const * argv);

typedef struct chiptool_command_t {
    const char * c_name; /* name of the command. */
    command_func c_func; /* function to execute the command. */
    const char * c_usage; /* usage string for command. */
    const char * c_help; /* help string for (or description of) command. */
} chiptool_command_t;

@interface CHIPToolCommandManager : NSObject

@property (readonly, assign, nonatomic) chiptool_command_t * allCommands;

+ (instancetype)sharedInstance;

- (void)configureWithLegacyCommands:(const chiptool_command_t *)commandArray;

- (int)executeCommand:(const chiptool_command_t *)command argc:(int)argc argv:(char * const *)argv;
@end
