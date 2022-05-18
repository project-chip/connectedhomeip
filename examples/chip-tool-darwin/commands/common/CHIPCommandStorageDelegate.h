#import <CHIP/CHIP.h>
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface CHIPToolPersistentStorageDelegate : NSObject <CHIPPersistentStorageDelegate>
- (nullable NSData *)storageDataForKey:(NSString *)key;
- (BOOL)setStorageData:(NSData *)value forKey:(NSString *)key;
- (BOOL)removeStorageDataForKey:(NSString *)key;
- (BOOL)deleteAllStorage;
@end

NS_ASSUME_NONNULL_END
