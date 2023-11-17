/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#import "MTRFramework.h"
#import "MTRLogging_Internal.h"
#import "MTRSetupPayload_Internal.h"

#import <setup_payload/QRCodeSetupPayloadParser.h>
#import <setup_payload/SetupPayload.h>

@implementation MTRQRCodeSetupPayloadParser {
    NSString * _base38Representation;
    chip::QRCodeSetupPayloadParser * _chipQRCodeSetupPayloadParser;
}

+ (void)initialize
{
    MTRFrameworkInit();
}

- (id)initWithBase38Representation:(NSString *)base38Representation
{
    if (self = [super init]) {
        _base38Representation = base38Representation;
        _chipQRCodeSetupPayloadParser = new chip::QRCodeSetupPayloadParser(std::string([base38Representation UTF8String]));
    }
    return self;
}

- (MTRSetupPayload *)populatePayload:(NSError * __autoreleasing *)error
{
    chip::SetupPayload cPlusPluspayload;
    MTRSetupPayload * payload;

    if (_chipQRCodeSetupPayloadParser) {
        CHIP_ERROR chipError = _chipQRCodeSetupPayloadParser->populatePayload(cPlusPluspayload);

        if (chipError == CHIP_NO_ERROR) {
            payload = [[MTRSetupPayload alloc] initWithSetupPayload:cPlusPluspayload];
        } else if (error) {
            *error = [MTRError errorForCHIPErrorCode:chipError];
        }
    } else {
        // Memory init has failed
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_NO_MEMORY];
        }
    }

    return payload;
}

- (void)dealloc
{
    delete _chipQRCodeSetupPayloadParser;
    _chipQRCodeSetupPayloadParser = nullptr;
}

@end
