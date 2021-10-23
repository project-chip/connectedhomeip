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

#import "CHIPQRCodeSetupPayloadParser.h"
#import "CHIPError_Internal.h"
#import "CHIPLogging.h"
#import "CHIPSetupPayload_Internal.h"

#import <lib/support/CHIPMem.h>
#import <setup_payload/QRCodeSetupPayloadParser.h>
#import <setup_payload/SetupPayload.h>

@implementation CHIPQRCodeSetupPayloadParser {
    NSString * _base38Representation;
    chip::QRCodeSetupPayloadParser * _chipQRCodeSetupPayloadParser;
}

- (id)initWithBase38Representation:(NSString *)base38Representation
{
    if (self = [super init]) {
        if (CHIP_NO_ERROR != chip::Platform::MemoryInit()) {
            CHIP_LOG_ERROR("Error: couldn't initialize platform memory");
            return self;
        }
        _base38Representation = base38Representation;
        _chipQRCodeSetupPayloadParser = new chip::QRCodeSetupPayloadParser(std::string([base38Representation UTF8String]));
    }
    return self;
}

- (CHIPSetupPayload *)populatePayload:(NSError * __autoreleasing *)error
{
    chip::SetupPayload cPlusPluspayload;
    CHIPSetupPayload * payload;

    if (_chipQRCodeSetupPayloadParser) {
        CHIP_ERROR chipError = _chipQRCodeSetupPayloadParser->populatePayload(cPlusPluspayload);

        if (chipError == CHIP_NO_ERROR) {
            payload = [[CHIPSetupPayload alloc] initWithSetupPayload:cPlusPluspayload];
        } else if (error) {
            *error = [CHIPError errorForCHIPErrorCode:chipError];
        }
    } else {
        // Memory init has failed
        if (error) {
            *error = [CHIPError errorForCHIPErrorCode:CHIP_ERROR_NO_MEMORY];
        }
    }

    return payload;
}

- (void)dealloc
{
    delete _chipQRCodeSetupPayloadParser;
    _chipQRCodeSetupPayloadParser = nullptr;
    chip::Platform::MemoryShutdown();
}

@end
