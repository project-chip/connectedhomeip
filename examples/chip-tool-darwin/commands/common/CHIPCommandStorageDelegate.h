#import <CHIP/CHIP.h>
#import <Foundation/Foundation.h>

@interface CHIPToolPersistentStorageDelegate : NSObject <CHIPPersistentStorageDelegate>
- (NSData *)valueForKey:(NSString *)key;
- (BOOL)setValue:(NSData *)value forKey:(NSString *)key;
- (BOOL)removeValueForKey:(NSString *)key;
@end
