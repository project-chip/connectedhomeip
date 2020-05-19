//
//  CHIPToolTask.m
//  chiptool
//
//  Created by Mike Fullerton on 4/7/17.
//

#import "CHIPToolTask.h"
#import "CHIPToolCommandManager.h"

@interface CHIPToolTask ()
@property (readwrite, strong, nonatomic, nullable) NSDictionary * arguments;
@property (readwrite, strong, nonatomic, nullable) NSArray<NSString *> * expectedParameters;
@property (readwrite, assign) BOOL waitingForEvent;
- (void)willPerformTaskWithParameters:(int)argc argv:(char * const *)argv;

@end

@implementation CHIPToolTask

- (id)init
{
    return [self initWithExpectedParameters:nil];
}

- (id)initWithExpectedParameters:(NSArray<NSString *> * _Nullable)parameters
{
    self = [super init];
    if (self) {
        _expectedParameters = parameters;
        if (!_expectedParameters) {
            _expectedParameters = [[self class] expectedParameters];
        }
    }
    return self;
}

- (void)performTask
{
}

- (void)finishTask
{
    [self finishWaitingForEvent];
}

- (void)willPerformTaskWithParameters:(int)argc argv:(char * const *)argv
{

    //    for(int i = 0; i < argc; i++) {
    //        CHIPPrintf(@"arg[%d] == %s", i, argv[i]);
    //    }

    NSMutableString * expectedParameters = [NSMutableString string];
    for (NSString * expected in self.expectedParameters) {
        [expectedParameters appendFormat:@"%@:", expected];
    }

    self.arguments = [NSDictionary dictionaryWithArgc:argc argv:argv expected:[expectedParameters UTF8String]];
}

- (id)argumentForKey:(NSString *)key withDefault:(id)defaultValue
{
    return [self.arguments objectForKey:key withDefault:defaultValue];
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

+ (int)performTask:(int)argc argv:(char * const *)argv
{
    @try {
        CHIPToolTask * task = [[[self class] alloc] init];
        [task willPerformTaskWithParameters:argc argv:argv];
        [task performTask];
    } @catch (NSException * ex) {
        CHIPPrintf(@"Call stack:\n%@", [ex callStackSymbols]);
        return 1;
    }
    return 0;
}

- (void)finishWaitingForEvent
{
    self.waitingForEvent = NO;
}

- (void)waitForDuration:(NSTimeInterval)duration
{
    NSTimeInterval finishTime = [NSDate timeIntervalSinceReferenceDate] + duration;
    while (finishTime > [NSDate timeIntervalSinceReferenceDate]) {
        @autoreleasepool {
            [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
        }
    }
    CHIPPrintf(@"Done waiting for interval");
}

- (void)waitForEventsToCompleteWithCompletion:(dispatch_block_t)completion
{
    self.waitingForEvent = YES;
    while (self.waitingForEvent) {
        @autoreleasepool {
            [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
        }
    }

    CHIPPrintf(@"Done waiting for event");
    if (completion) {
        completion();
    }
}

- (void)waitForEventsToComplete
{
    [self waitForEventsToCompleteWithCompletion:nil];
}

+ (NSString *)commandName
{
    return NSStringFromClass([self class]);
}

+ (BOOL)commandShouldConnect
{
    return YES;
}

+ (NSString *)usage
{
    return @"";
}

+ (NSString *)help
{
    return @"";
}

+ (void)registerToolTask
{
    [[CHIPToolCommandManager sharedInstance] registerToolTaskClass:[self class]];
}

+ (void)load
{
    //   CHIPSetAlwaysUseSingletonProxies(YES);
}

+ (NSArray<NSString *> *)expectedParameters
{
    return nil;
}

@end

@implementation NSDictionary (UserArguments)
+ (instancetype)dictionaryWithArgc:(int)argc argv:(char * const *)argv expected:(const char *)expected
{
    NSMutableDictionary * args = [NSMutableDictionary new];
    int ch;
    while ((ch = getopt(argc, argv, expected)) != -1) {
        [args setObject:[NSString stringWithUTF8String:optarg] forKey:[NSString stringWithFormat:@"%c", ch]];
    }
    return args;
}
- (id)objectForKey:(id)aKey withDefault:(id)defaultValue
{
    id value = [self objectForKey:aKey];
    return value ? value : defaultValue;
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
