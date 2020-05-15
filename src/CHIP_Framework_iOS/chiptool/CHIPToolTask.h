//
//  CHIPToolTask.h
//  chiptool
//
//  Created by Mike Fullerton on 4/7/17.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

extern void CHIPPrintf(NSString * format, ...) NS_FORMAT_FUNCTION(1, 2);

@interface NSDictionary (UserArguments)
// expected is in same for as third parameter of getopt
+ (instancetype)dictionaryWithArgc:(int)argc argv:(char * const *)argv expected:(const char *)expected;
- (id)objectForKey:(id)aKey withDefault:(id)defaultValue;
@end

@interface CHIPToolTask : NSObject
@property (readonly, strong, nonatomic, nullable) NSDictionary * arguments;
@property (readonly, strong, nonatomic, nullable) NSArray<NSString *> * expectedParameters;

/** pass expected parameter into this like
    self = [super initWithExpectedParameters:[@[ @"a", @"b" ]];
 */
- (id)initWithExpectedParameters:(NSArray<NSString *> * _Nullable)expectedParameters;

/**
    main overidde point, this is where you do your work
 */
- (void)performTask;

/**
    call ths from performTask to get your parameters
 */
- (id _Nullable)argumentForKey:(NSString *)key withDefault:(id _Nullable)defaultValue;

/**
    blocks until done
 */
- (void)waitForEventsToComplete;

/**
    call this to tell task to stop blocking
 */
- (void)finishWaitingForEvent;

/**
    wait for x seconds
 */
- (void)waitForDuration:(NSTimeInterval)duration;

/**
    override these to set how CHIPTool interacts with the command before it is run/instantiated
 */
+ (NSString *)commandName;
+ (NSString *)usage;
+ (NSString *)help;
+ (NSArray<NSString *> * _Nullable)expectedParameters;

/**
    does the command connect to imagent?
    defaults TO YES
 */
+ (BOOL)commandShouldConnect;

/**
    INTERNAL
 */

/**
   called by CHIPTool to start the task
*/
+ (int)performTask:(int)argc argv:(char * _Nonnull const * _Nonnull)argv;

+ (void)registerToolTask;
@end

NS_ASSUME_NONNULL_END

// HELPER FOR TESTS

#define CHIPTAssertNotNil(__OBJECT__)                                                                                              \
    ({                                                                                                                             \
        if ((__OBJECT__) == nil) {                                                                                                 \
            CHIPPrintf(@"Assertion failed: '%s' is unexpectedly nil (%s, %d)", #__OBJECT__, __FILE__, __LINE__);                   \
            exit(1);                                                                                                               \
        }                                                                                                                          \
    })

#define CHIPTAssert(__CONDITION__)                                                                                                 \
    ({                                                                                                                             \
        if ((__CONDITION__) == NO) {                                                                                               \
            CHIPPrintf(@"Assertion failed: '%s' is unexpectedly false (%s, %d)", #__CONDITION__, __FILE__, __LINE__);              \
            exit(1);                                                                                                               \
        }                                                                                                                          \
    })

#define CHIPTAssertNil(__OBJECT__)                                                                                                 \
    ({                                                                                                                             \
        id __in_object = __OBJECT__;                                                                                               \
        if ((__in_object) != nil) {                                                                                                \
            CHIPPrintf(@"Assertion failed: '%s' is unexpectedly NOT nil (%s, %d) -> %@", #__OBJECT__, __FILE__, __LINE__,          \
                [__in_object description]);                                                                                        \
            exit(1);                                                                                                               \
        }                                                                                                                          \
    })

#define CHIPTAssertNotError(__ERROR__)                                                                                             \
    ({                                                                                                                             \
        id __in_object = __ERROR__;                                                                                                \
        if ((__in_object) != nil) {                                                                                                \
            CHIPPrintf(@"Assertion failed: %@ (%s, %d)", [__in_object localizedDescription], __FILE__, __LINE__);                  \
            exit(1);                                                                                                               \
        }                                                                                                                          \
    })

#define CHIPTAssertEqual(__LHS__, __RHS__)                                                                                         \
    ({                                                                                                                             \
        if ((__LHS__) != (__RHS__)) {                                                                                              \
            CHIPPrintf(@"Assertion failed: %s != %s in (%s, %d)", #__LHS__, #__RHS__, __FILE__, __LINE__);                         \
            exit(1);                                                                                                               \
        }                                                                                                                          \
    })

#define CHIPRegisterToolTask()                                                                                                     \
    +(void) load { [self registerToolTask]; }

#define CHIPRegisterToolTaskWithName(NAME, HELP, PARAMS)                                                                           \
    CHIPRegisterToolTask() + (NSString *) commandName { return NAME; }                                                             \
    +(NSString *) help { return HELP; }                                                                                            \
    +(NSArray<NSString *> *) expectedParameters { return PARAMS; }
