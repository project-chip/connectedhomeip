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

#import "MTRQRCodeSetupPayloadParser.h"

#import "MTRError_Internal.h"
#import "MTRSetupPayload_Internal.h"

@implementation MTRQRCodeSetupPayloadParser {
    NSString * _base38Representation;
}

- (id)initWithBase38Representation:(NSString *)base38Representation
{
    self = [super init];
    _base38Representation = base38Representation;
    return self;
}

- (MTRSetupPayload *)populatePayload:(NSError * __autoreleasing *)error
{
    MTRSetupPayload * payload = [[MTRSetupPayload alloc] initWithQRCode:_base38Representation];
    if (!payload && error) {
        *error = [MTRError errorWithCode:MTRErrorCodeInvalidArgument];
    }
    return payload;
}

@end
