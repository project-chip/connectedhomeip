#include "CHIPCommandStorageDelegate.h"

#import <Matter/Matter.h>

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
    NSLog(@"Removing keys: %@ %@", allKeys, domain);
    for (id key in allKeys) {
        NSLog(@"Removing key: %@", key);
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
    NSLog(@"CHIPPersistentStorageDelegate Get Value for Key: %@, value %@", key, value);
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
