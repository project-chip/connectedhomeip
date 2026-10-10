/**
 *
 *    Copyright (c) 2020-2024 Project CHIP Authors
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

#import "MTRManualSetupPayloadParser.h"

#import "MTRSetupPayload_Internal.h"

@implementation MTRManualSetupPayloadParser {
    NSString * _decimalStringRepresentation;
}

- (id)initWithDecimalStringRepresentation:(NSString *)decimalStringRepresentation
{
    self = [super init];
    _decimalStringRepresentation = decimalStringRepresentation;
    return self;
}

- (MTRSetupPayload *)populatePayload:(NSError * __autoreleasing *)error
{
    return [[MTRSetupPayload alloc] initWithManualPairingCode:_decimalStringRepresentation error:error];
}

@end
