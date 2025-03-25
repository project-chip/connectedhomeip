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

#import "MTRDefines_Internal.h"

NS_ASSUME_NONNULL_BEGIN

// Returns whether a data-value dictionary is well-formed (in the sense that all
// the types of the objects inside are as expected, so it's actually a valid
// representation of some TLV).  Implemented in MTRBaseDevice.mm because that's
// where the pieces needed to implement it are, but declared here so our tests
// can see it.
MTR_EXTERN MTR_TESTABLE BOOL MTRDataValueDictionaryIsWellFormed(MTRDeviceDataValueDictionary value);

// Returns whether the provided attribute report actually has the right sorts of
// objects in the right places.
MTR_EXTERN MTR_TESTABLE BOOL MTRAttributeReportIsWellFormed(NSArray<MTRDeviceResponseValueDictionary> * report);

// Returns whether the provided event report actually has the right sorts of
// objects in the right places.
MTR_EXTERN MTR_TESTABLE BOOL MTREventReportIsWellFormed(NSArray<MTRDeviceResponseValueDictionary> * report);

// Returns whether the provided invoke response actually has the right sorts of
// objects in the right places.
MTR_EXTERN MTR_TESTABLE BOOL MTRInvokeResponseIsWellFormed(NSArray<MTRDeviceResponseValueDictionary> * response);

// Returns whether the provided invoke responses actually have the right sorts of objects in the
// right places.  This differs from MTRInvokeResponseIsWellFormed in not enforcing that there is
// only one response.
MTR_EXTERN MTR_TESTABLE BOOL MTRInvokeResponsesAreWellFormed(NSArray<MTRDeviceResponseValueDictionary> * responses);

NS_ASSUME_NONNULL_END
