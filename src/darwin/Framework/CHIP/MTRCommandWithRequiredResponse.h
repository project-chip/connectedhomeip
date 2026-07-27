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

#import <Foundation/Foundation.h>
#import <Matter/MTRBaseDevice.h> // For MTRCommandPath

NS_ASSUME_NONNULL_BEGIN

/**
 * An object representing a single command to be invoked and the response
 * required for the invoke to be considered successful.
 */
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRCommandWithRequiredResponse : NSObject <NSCopying, NSSecureCoding>

/**
 * The path of the command being invoked.
 */
@property (nonatomic, retain) MTRCommandPath * path;

/**
 * The command fields to pass for the command invoke.  nil if this command does
 * not have any fields.  If not nil, this should be a data-value dictionary of
 * MTRStructureValueType.
 */
@property (nonatomic, retain, nullable) NSDictionary<NSString *, id> * commandFields;

/**
 * The response that represents this command succeeding.
 *
 * If this is nil, that indicates that the invoke is considered successful if it
 * does not result in an error status response.
 *
 * If this is is not nil, then the invoke is considered successful if
 * the response is a data response and for each entry in the provided
 * requiredResponse the field whose field ID matches the key of the entry has a
 * value that equals the value of the entry.  Values of entries are data-value
 * dictionaries.
 */
@property (nonatomic, copy, nullable) NSDictionary<NSNumber *, NSDictionary<NSString *, id> *> * requiredResponse;

- (instancetype)initWithPath:(MTRCommandPath *)path
               commandFields:(nullable NSDictionary<NSString *, id> *)commandFields
            requiredResponse:(nullable NSDictionary<NSNumber *, NSDictionary<NSString *, id> *> *)requiredResponse;

@end

NS_ASSUME_NONNULL_END
