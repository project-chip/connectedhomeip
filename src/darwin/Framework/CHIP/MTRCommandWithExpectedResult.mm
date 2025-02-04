/**
 *    Copyright (c) 2025 Project CHIP Authors
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

#import "MTRDeviceDataValidation.h"
#import "MTRLogging_Internal.h"
#import <Matter/Matter.h>

@implementation MTRCommandWithExpectedResult
- (instancetype)initWithPath:(MTRCommandPath *)path
               commandFields:(nullable NSDictionary<NSString *, id> *)commandFields
              expectedResult:(nullable NSDictionary<NSNumber *, NSDictionary<NSString *, id> *> *)expectedResult
{
    if (self = [super init]) {
        self.path = path;
        self.commandFields = commandFields;
        self.expectedResult = expectedResult;
    }

    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    return [[MTRCommandWithExpectedResult alloc] initWithPath:self.path commandFields:self.commandFields expectedResult:self.expectedResult];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: %p, path: %@, fields: %@, expectedResult: %@", NSStringFromClass(self.class), self, self.path, self.commandFields, self.expectedResult];
}

#pragma mark - MTRCommandWithExpectedResult NSSecureCoding implementation

static NSString * const sPathKey = @"pathKey";
static NSString * const sFieldsKey = @"fieldsKey";
static NSString * const sExpectedResultKey = @"expectedResultKey";

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (nullable instancetype)initWithCoder:(NSCoder *)decoder
{
    self = [super init];
    if (self == nil) {
        return nil;
    }

    _path = [decoder decodeObjectOfClass:MTRCommandPath.class forKey:sPathKey];
    if (!_path || ![_path isKindOfClass:MTRCommandPath.class]) {
        MTR_LOG_ERROR("MTRCommandWithExpectedResult decoded %@ for endpoint, not MTRCommandPath.", _path);
        return nil;
    }

    _commandFields = [decoder decodeObjectOfClass:NSDictionary.class forKey:sFieldsKey];
    if (_commandFields) {
        if (![_commandFields isKindOfClass:NSDictionary.class]) {
            MTR_LOG_ERROR("MTRCommandWithExpectedResult decoded %@ for commandFields, not NSDictionary.", _commandFields);
            return nil;
        }

        if (!MTRDataValueDictionaryIsWellFormed(_commandFields) || ![MTRStructureValueType isEqual:_commandFields[MTRTypeKey]]) {
            MTR_LOG_ERROR("MTRCommandWithExpectedResult decoded %@ for commandFields, not a structure-typed data-value dictionary.", _commandFields);
            return nil;
        }
    }

    _expectedResult = [decoder decodeObjectOfClass:NSDictionary.class forKey:sExpectedResultKey];
    if (_expectedResult) {
        if (![_expectedResult isKindOfClass:NSDictionary.class]) {
            MTR_LOG_ERROR("MTRCommandWithExpectedResult decoded %@ for expectedResult, not NSDictionary.", _expectedResult);
            return nil;
        }

        for (id key in _expectedResult) {
            if (![key isKindOfClass:NSNumber.class]) {
                MTR_LOG_ERROR("MTRCommandWithExpectedResult decoded key %@ in expectedResult", key);
                return nil;
            }

            if (![_expectedResult[key] isKindOfClass:NSDictionary.class] || !MTRDataValueDictionaryIsWellFormed(_expectedResult[key])) {
                MTR_LOG_ERROR("MTRCommandWithExpectedResult decoded value %@ for key %@ in expectedResult", _expectedResult[key], key);
                return nil;
            }
        }
    }

    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    // In theory path is not nullable, but we don't really enforce that in init.
    if (self.path) {
        [coder encodeObject:self.path forKey:sPathKey];
    }
    if (self.commandFields) {
        [coder encodeObject:self.commandFields forKey:sFieldsKey];
    }
    if (self.expectedResult) {
        [coder encodeObject:self.expectedResult forKey:sExpectedResultKey];
    }
}

@end
