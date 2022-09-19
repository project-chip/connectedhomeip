#include "CHIPCommandStorageDelegate.h"

#import <Matter/Matter.h>

#define LOG_DEBUG_PERSISTENT_STORAGE_DELEGATE 0

NSString * const kCHIPToolDefaultsDomain = @"com.apple.chiptool";

id MTRGetDomainValueForKey(NSString * domain, NSString * key)
{
    id value = (id) CFBridgingRelease(CFPreferencesCopyAppValue((CFStringRef) key, (CFStringRef) domain));
    if (value) {
        return value;
    }
    return nil;
}

BOOL MTRSetDomainValueForKey(NSString * domain, NSString * key, id value)
{
    CFPreferencesSetAppValue((CFStringRef) key, (__bridge CFPropertyListRef _Nullable)(value), (CFStringRef) domain);
    return CFPreferencesAppSynchronize((CFStringRef) domain) == true;
}

BOOL MTRRemoveDomainValueForKey(NSString * domain, NSString * key)
{
    CFPreferencesSetAppValue((CFStringRef) key, nullptr, (CFStringRef) domain);
    return CFPreferencesAppSynchronize((CFStringRef) domain) == true;
}

id CHIPGetDomainKeyList(NSString * domain)
{
    id value
        = (id) CFBridgingRelease(CFPreferencesCopyKeyList((CFStringRef) domain, kCFPreferencesCurrentUser, kCFPreferencesAnyHost));
    if (value) {
        return value;
    }
    return nil;
}

BOOL CHIPClearAllDomain(NSString * domain)
{

    NSArray * allKeys = CHIPGetDomainKeyList(domain);
#if LOG_DEBUG_PERSISTENT_STORAGE_DELEGATE
    NSLog(@"Removing keys: %@ %@", allKeys, domain);
#endif
    for (id key in allKeys) {
#if LOG_DEBUG_PERSISTENT_STORAGE_DELEGATE
        NSLog(@"Removing key: %@", key);
#endif
        if (!MTRRemoveDomainValueForKey(domain, (NSString *) key)) {
            return NO;
        }
    }
    return YES;
}

@implementation CHIPToolPersistentStorageDelegate

- (BOOL)deleteAllStorage
{
    return CHIPClearAllDomain(kCHIPToolDefaultsDomain);
}

// MARK: CHIPPersistentStorageDelegate

- (nullable NSData *)storageDataForKey:(NSString *)key
{
    NSData * value = MTRGetDomainValueForKey(kCHIPToolDefaultsDomain, key);
#if LOG_DEBUG_PERSISTENT_STORAGE_DELEGATE
    NSLog(@"CHIPPersistentStorageDelegate Get Value for Key: %@, value %@", key, value);
#endif
    return value;
}

- (BOOL)setStorageData:(NSData *)value forKey:(NSString *)key
{
    return MTRSetDomainValueForKey(kCHIPToolDefaultsDomain, key, value);
}

- (BOOL)removeStorageDataForKey:(NSString *)key
{
    if (MTRGetDomainValueForKey(kCHIPToolDefaultsDomain, key) == nil) {
        return NO;
    }
    return MTRRemoveDomainValueForKey(kCHIPToolDefaultsDomain, key);
}

@end
