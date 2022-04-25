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

void CHIPSetDomainValueForKey(NSString * domain, NSString * key, id value)
{
    CFPreferencesSetAppValue((CFStringRef) key, (__bridge CFPropertyListRef _Nullable)(value), (CFStringRef) domain);
    CFPreferencesAppSynchronize((CFStringRef) domain);
}

void CHIPRemoveDomainValueForKey(NSString * domain, NSString * key)
{
    CFPreferencesSetAppValue((CFStringRef) key, nullptr, (CFStringRef) domain);
    CFPreferencesAppSynchronize((CFStringRef) domain);
}

@implementation CHIPToolPersistentStorageDelegate

// MARK: CHIPPersistentStorageDelegate

- (NSString *)CHIPGetKeyValue:(NSString *)key
{
    NSString * value = CHIPGetDomainValueForKey(kCHIPToolDefaultsDomain, key);
    NSLog(@"CHIPPersistentStorageDelegate Get Value for Key: %@, value %@", key, value);
    return value;
}

- (void)CHIPSetKeyValue:(NSString *)key value:(NSString *)value
{
    CHIPSetDomainValueForKey(kCHIPToolDefaultsDomain, key, value);
}

- (void)CHIPDeleteKeyValue:(NSString *)key
{
    CHIPRemoveDomainValueForKey(kCHIPToolDefaultsDomain, key);
}

@end
