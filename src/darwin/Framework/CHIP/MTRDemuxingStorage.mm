/**
 *    Copyright (c) 2023 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#import "MTRDemuxingStorage.h"

#import "MTRDeviceControllerFactory_Internal.h"
#import "MTRDeviceController_Internal.h"
#import "MTRLogging_Internal.h"

#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <platform/LockTracker.h>

using namespace chip;

static bool IsGlobalKey(NSString * key) { return [key hasPrefix:@"g/"]; }

/**
 * Checks for a key that is scoped to a specific fabric index.
 */
static bool IsIndexSpecificKey(NSString * key) { return [key hasPrefix:@"f/"]; }

/**
 * Extracts the fabric index from an index-specific key.  Fails if the key
 * is not index-specific or if a numeric FabricIndex could not be extracted
 * from it.
 */
static CHIP_ERROR ExtractIndexFromKey(NSString * key, FabricIndex * index)
{
    if (!IsIndexSpecificKey(key)) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    auto * components = [key componentsSeparatedByString:@"/"];
    if (components.count < 3) {
        // Unexpected "f/something" without any actual data.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    auto * indexString = components[1];
    auto * scanner = [NSScanner scannerWithString:indexString];

    auto * charset = [NSCharacterSet characterSetWithCharactersInString:@"0123456789abcdefABCDEF"];
    charset = [charset invertedSet];

    if ([scanner scanCharactersFromSet:charset intoString:nil] == YES) {
        // Leading non-hex chars.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    unsigned int value;
    if ([scanner scanHexInt:&value] == NO) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (scanner.atEnd == NO) {
        // Trailing garbage chars.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (!CanCastTo<FabricIndex>(value)) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    *index = static_cast<FabricIndex>(value);
    return CHIP_NO_ERROR;
}

/**
 * Extracts the "index-specific" part of an index-specific key (i.e. the
 * part after "f/index/").
 */
static CHIP_ERROR ExtractIndexSpecificKey(NSString * key, NSString * __autoreleasing * extractedKey)
{
    if (!IsIndexSpecificKey(key)) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    auto * components = [key componentsSeparatedByString:@"/"];
    if (components.count < 3) {
        // Unexpected "f/something" without any actual data.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    components = [components subarrayWithRange:NSMakeRange(2, components.count - 2)];
    *extractedKey = [components componentsJoinedByString:@"/"];
    return CHIP_NO_ERROR;
}

/**
 * Method to test whether a global key should be stored in memory only, as
 * opposed to being passed on to the actual storage related to controllers.
 */
static bool IsMemoryOnlyGlobalKey(NSString * key)
{
    if ([key isEqualToString:@"g/fidx"]) {
        // Fabric index list only needs to be stored in-memory, not persisted,
        // because we do not tie controllers to specific fabric indices.
        return true;
    }

    if ([key isEqualToString:@"g/fs/c"] || [key isEqualToString:@"g/fs/n"]) {
        // Just store the fail-safe markers in memory as well.  We could
        // plausibly not store them at all, since we never actually need to
        // clean up anything by fabric index, but this is safer in case
        // consumers try to read back right after writing.
        return true;
    }

    if ([key isEqualToString:@"g/lkgt"]) {
        // Store Last Known Good Time in memory only.  We never need this in
        // general, because we can always provide the wall-clock time.
        return true;
    }

    // We do not expect to see the "g/sri" or "g/s/*" keys for session
    // resumption, because we implement SessionResumptionStorage ourselves.

    // For now, put group global counters in memory.
    // TODO: we should inject a group counter manager that makes these counters
    // per-controller, not globally handled via storage.
    // See https://github.com/project-chip/connectedhomeip/issues/28510
    if ([key isEqualToString:@"g/gdc"] || [key isEqualToString:@"g/gcc"]) {
        return true;
    }

    // We do not expect to see "g/userlbl/*" User Label keys.

    // We do not expect to see the "g/gfl" key for endpoint-to-group
    // associations.

    // We do not expect to see the "g/a/*" keys for attribute values.

    // We do not expect to see the "g/bt" and "g/bt/*" keys for the binding
    // table.

    // We do not expect to see the "g/o/*" OTA Requestor keys.

    // We do not expect to see the "g/im/ec" event number counter key.

    // We do not expect to see the "g/su/*" and "g/sum" keys for server-side
    // subscription resumption storage.

    // We do not expect to see the "g/scc/*" scenes keys.

    // We do not expect to see the "g/ts/tts", "g/ts/dntp", "g/ts/tz",
    // "g/ts/dsto" Time Synchronization keys.

    // We do not expect to see the "g/icd/cic" key; that's only used for an ICD
    // that sends check-in messages.

    return false;
}

/**
 * Method to test whether an index-specific key should be stored in memory only, as
 * opposed to being passed on to the actual storage related to controllers.
 * The key string will have the "f/index/" bit already stripped off the
 * front of the key.
 */
static bool IsMemoryOnlyIndexSpecificKey(NSString * key)
{
    // Store all the fabric table bits in memory only.  This is done because the
    // fabric table expects none of these things to be stored in the case of a
    // "new fabric addition", which is what we always do when using
    // per-controller storage.
    //
    // TODO: Figure out which, if any, of these things we should also store for
    // later recall when starting a controller with storage we have used before.
    //
    // For future reference:
    //
    // n == NOC
    // i == ICAC
    // r == RCAC
    // m == Fabric metadata (TLV containing the vendor ID)
    // o == operational key, only written if internally generated.
    if ([key isEqualToString:@"n"] || [key isEqualToString:@"i"] || [key isEqualToString:@"r"] || [key isEqualToString:@"m"] ||
        [key isEqualToString:@"o"]) {
        return true;
    }

    // We do not expect to see the "s/*" keys for session resumption, because we
    // implement SessionResumptionStorage ourselves.

    // We do not expect to see the "ac/*" keys for ACL entries.

    // We do not expect to see the "g" or "g/*" keys for which endpoints are in which
    // group.

    // For now, just store group keysets and group keys in memory.
    // TODO: We want to start persisting these, per-controller, if we're going
    // to support group keys.  Or inject a GroupDataProvider of our own instead
    // of using Credentials::GroupDataProviderImp and then
    // not be tied to whatever storage format that uses.
    // https://github.com/project-chip/connectedhomeip/issues/28511
    if ([key hasPrefix:@"gk/"] || [key hasPrefix:@"k/"]) {
        return true;
    }

    // We do not expect to see the "icd/*" keys for the ICD Management table.

    // We do not expect to see the "e/*" scenes keys.

    return false;
}

/**
 * Method to convert an index-specific key into a fully qualified key.
 */
static NSString * FullyQualifiedKey(FabricIndex index, NSString * key)
{
    return [NSString stringWithFormat:@"f/%x/%s", index, key.UTF8String];
}

MTRDemuxingStorage::MTRDemuxingStorage(MTRDeviceControllerFactory * factory)
    : mFactory(factory)
{
    mInMemoryStore = [[NSMutableDictionary alloc] init];
}

CHIP_ERROR MTRDemuxingStorage::SyncGetKeyValue(const char * key, void * buffer, uint16_t & size)
{
    assertChipStackLockedByCurrentThread();
    if (buffer == nullptr && size != 0) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    NSString * keyString = [NSString stringWithUTF8String:key];

#if LOG_DEBUG_PERSISTENT_STORAGE_DELEGATE
    MTR_LOG_DEBUG("MTRDemuxingStorage Sync Get Value for Key: %@", keyString);
#endif

    NSData * value;
    if (IsGlobalKey(keyString)) {
        value = GetGlobalValue(keyString);
    } else if (IsIndexSpecificKey(keyString)) {
        FabricIndex index;
        ReturnErrorOnFailure(ExtractIndexFromKey(keyString, &index));
        ReturnErrorOnFailure(ExtractIndexSpecificKey(keyString, &keyString));
        value = GetIndexSpecificValue(index, keyString);
    } else {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (value == nil) {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    if (value.length > UINT16_MAX) {
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }

    uint16_t valueSize = static_cast<uint16_t>(value.length);
    if (valueSize > size) {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    size = valueSize;

    if (size != 0) {
        // buffer is known to be non-null here.
        memcpy(buffer, value.bytes, size);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MTRDemuxingStorage::SyncSetKeyValue(const char * key, const void * value, uint16_t size)
{
    if (value == nullptr && size != 0) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    NSString * keyString = [NSString stringWithUTF8String:key];
    NSData * valueData = (value == nullptr) ? [NSData data] : [NSData dataWithBytes:value length:size];

#if LOG_DEBUG_PERSISTENT_STORAGE_DELEGATE
    MTR_LOG_DEBUG("MTRDemuxingStorage Set Key %@", keyString);
#endif

    if (IsGlobalKey(keyString)) {
        return SetGlobalValue(keyString, valueData);
    }

    if (IsIndexSpecificKey(keyString)) {
        FabricIndex index;
        ReturnErrorOnFailure(ExtractIndexFromKey(keyString, &index));
        ReturnErrorOnFailure(ExtractIndexSpecificKey(keyString, &keyString));
        return SetIndexSpecificValue(index, keyString, valueData);
    }

    return CHIP_ERROR_INVALID_ARGUMENT;
}

CHIP_ERROR MTRDemuxingStorage::SyncDeleteKeyValue(const char * key)
{
    NSString * keyString = [NSString stringWithUTF8String:key];

#if LOG_DEBUG_PERSISTENT_STORAGE_DELEGATE
    MTR_LOG_DEBUG("MTRDemuxingStorage Delete Key: %@", keyString);
#endif

    if (IsGlobalKey(keyString)) {
        return DeleteGlobalValue(keyString);
    }

    if (IsIndexSpecificKey(keyString)) {
        FabricIndex index;
        ReturnErrorOnFailure(ExtractIndexFromKey(keyString, &index));
        ReturnErrorOnFailure(ExtractIndexSpecificKey(keyString, &keyString));
        return DeleteIndexSpecificValue(index, keyString);
    }

    return CHIP_ERROR_INVALID_ARGUMENT;
}

NSData * _Nullable MTRDemuxingStorage::GetGlobalValue(NSString * key)
{
    if (IsMemoryOnlyGlobalKey(key)) {
        return GetInMemoryValue(key);
    }

    MTR_LOG_ERROR("MTRDemuxingStorage reading unknown global key: %@", key);

    return nil;
}

NSData * _Nullable MTRDemuxingStorage::GetIndexSpecificValue(FabricIndex index, NSString * key)
{
    if (IsMemoryOnlyIndexSpecificKey(key)) {
        return GetInMemoryValue(FullyQualifiedKey(index, key));
    }

    return nil;
}

CHIP_ERROR MTRDemuxingStorage::SetGlobalValue(NSString * key, NSData * data)
{
    if (IsMemoryOnlyGlobalKey(key)) {
        // Fabric index list only needs to be stored in-memory, not persisted,
        // because we do not tie controllers to specific fabric indices.
        return SetInMemoryValue(key, data);
    }

    MTR_LOG_ERROR("MTRDemuxingStorage setting unknown global key: %@", key);

    return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
}

CHIP_ERROR MTRDemuxingStorage::SetIndexSpecificValue(FabricIndex index, NSString * key, NSData * data)
{
    if ([key isEqualToString:@"n"]) {
        // Index-scoped "n" is NOC.
        auto * controller = [mFactory runningControllerForFabricIndex:index];
        if (controller == nil) {
            return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
        }

        ReturnErrorOnFailure([controller.controllerDataStore storeLastLocallyUsedNOC:data]);
    }

    if (IsMemoryOnlyIndexSpecificKey(key)) {
        return SetInMemoryValue(FullyQualifiedKey(index, key), data);
    }

    return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
}

CHIP_ERROR MTRDemuxingStorage::DeleteGlobalValue(NSString * key)
{
    if (IsMemoryOnlyGlobalKey(key)) {
        return DeleteInMemoryValue(key);
    }

    MTR_LOG_ERROR("MTRDemuxingStorage deleting unknown global key: %@", key);

    return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
}

CHIP_ERROR MTRDemuxingStorage::DeleteIndexSpecificValue(FabricIndex index, NSString * key)
{
    if (IsMemoryOnlyIndexSpecificKey(key)) {
        return DeleteInMemoryValue(FullyQualifiedKey(index, key));
    }

    return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
}

NSData * _Nullable MTRDemuxingStorage::GetInMemoryValue(NSString * key) { return mInMemoryStore[key]; }

CHIP_ERROR MTRDemuxingStorage::SetInMemoryValue(NSString * key, NSData * data)
{
    mInMemoryStore[key] = data;
    return CHIP_NO_ERROR;
}

CHIP_ERROR MTRDemuxingStorage::DeleteInMemoryValue(NSString * key)
{
    BOOL present = (mInMemoryStore[key] != nil);
    if (present) {
        [mInMemoryStore removeObjectForKey:key];
    }
    return present ? CHIP_NO_ERROR : CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
}
