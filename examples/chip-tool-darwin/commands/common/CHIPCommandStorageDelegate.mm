#include "CHIPCommandStorageDelegate.h"

NSString * const kCHIPToolDefaultsDomain = @"com.apple.chiptool";

id CHIPGetDomainValueForKey(NSString * domain, NSString * key)
{
    id value = (id) CFBridgingRelease(CFPreferencesCopyAppValue((CFStringRef) key, (CFStringRef) domain));
    if (value) {
        return value;
    }
    return nil;
}

BOOL CHIPSetDomainValueForKey(NSString * domain, NSString * key, id value)
{
    CFPreferencesSetAppValue((CFStringRef) key, (__bridge CFPropertyListRef _Nullable)(value), (CFStringRef) domain);
    return CFPreferencesAppSynchronize((CFStringRef) domain) == true;
}

void CHIPRemoveDomainValueForKey(NSString * domain, NSString * key)
{
    CFPreferencesSetAppValue((CFStringRef) key, nullptr, (CFStringRef) domain);
    CFPreferencesAppSynchronize((CFStringRef) domain);
}

@implementation CHIPToolPersistentStorageDelegate

// MARK: CHIPPersistentStorageDelegate

- (nullable NSData *)valueForKey:(NSString *)key
{
    NSData * value = CHIPGetDomainValueForKey(kCHIPToolDefaultsDomain, key);
    NSLog(@"CHIPPersistentStorageDelegate Get Value for Key: %@, value %@", key, value);
    return value;
}

- (BOOL)setValue:(NSData *)value forKey:(NSString *)key
{
    return CHIPSetDomainValueForKey(kCHIPToolDefaultsDomain, key, value);
}

- (BOOL)removeValueForKey:(NSString *)key
{
    if (CHIPGetDomainValueForKey(kCHIPToolDefaultsDomain, key) == nil) {
        return NO;
    }
    CHIPRemoveDomainValueForKey(kCHIPToolDefaultsDomain, key);
    return YES;
}

@end
