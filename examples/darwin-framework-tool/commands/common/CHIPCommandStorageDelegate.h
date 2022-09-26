#import <Foundation/Foundation.h>
#import <Matter/Matter.h>

NS_ASSUME_NONNULL_BEGIN

@interface CHIPToolPersistentStorageDelegate : NSObject <MTRStorage>
- (nullable NSData *)storageDataForKey:(NSString *)key;
- (BOOL)setStorageData:(NSData *)value forKey:(NSString *)key;
- (BOOL)removeStorageDataForKey:(NSString *)key;
- (BOOL)deleteAllStorage;
@end

NS_ASSUME_NONNULL_END
