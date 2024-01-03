/**
 *    Copyright (c) 2024 Project CHIP Authors
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
#import <Matter/MTRDefines.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * A representation of an attribute implemented on a server cluster by an
 * MTRDeviceController.  An attribute has an identifier and a value, and may or
 * may not be writable.
 */
MTR_NEWLY_AVAILABLE
@interface MTRAttributeDescription : NSObject <NSCopying>

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Initialize as a readonly attribute.  The value is a data-value as documented
 * in MTRBaseDevice.h.
 *
 * Will fail if the attribute ID is not valid or the attribute value is not a
 * valid data-value.
 */
- (nullable instancetype)initReadonlyAttributeWithID:(NSNumber *)attributeID initialValue:(NSDictionary<NSString *, id> *)value error:(NSError * __autoreleasing *)error;

/**
 * Initialize as a writable attribute.  The value is a data-value as documented
 * in MTRBaseDevice.h.
 *
 * Will fail if the attribute ID is not valid or the attribute value is not a
 * valid data-value.
 */
- (nullable instancetype)initWritableAttributeWithID:(NSNumber *)attributeID initialValue:(NSDictionary<NSString *, id> *)value error:(NSError * __autoreleasing *)error;

@property (nonatomic, copy, readonly) NSNumber * attributeID;
@property (nonatomic, copy, readonly) NSDictionary<NSString *, id> * value;
@property (nonatomic, readonly, getter=isWritable) BOOL writable;

@end

NS_ASSUME_NONNULL_END
