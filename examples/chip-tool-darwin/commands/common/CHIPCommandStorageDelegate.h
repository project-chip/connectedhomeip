#import <CHIP/CHIP.h>
#import <Foundation/Foundation.h>

@interface CHIPToolPersistentStorageDelegate : NSObject <CHIPPersistentStorageDelegate>
- (NSString *)CHIPGetKeyValue:(NSString *)key;
- (void)CHIPSetKeyValue:(NSString *)key value:(NSString *)value;
- (void)CHIPDeleteKeyValue:(NSString *)key;
@end
