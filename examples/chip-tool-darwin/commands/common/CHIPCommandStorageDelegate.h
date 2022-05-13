#import <CHIP/CHIP.h>
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface CHIPToolPersistentStorageDelegate : NSObject <CHIPPersistentStorageDelegate>
- (nullable NSData *)valueForKey:(NSString *)key;
// value is nullable because NSKeyValueCoding has a selector collision with us.
- (BOOL)setValue:(nullable NSData *)value forKey:(NSString *)key;
- (BOOL)removeValueForKey:(NSString *)key;
@end

NS_ASSUME_NONNULL_END
