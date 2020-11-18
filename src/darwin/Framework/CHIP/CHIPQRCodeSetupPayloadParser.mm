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
#import "CHIPError.h"
#import "CHIPLogging.h"
#import "CHIPSetupPayload.h"

#import <setup_payload/QRCodeSetupPayloadParser.h>
#import <setup_payload/SetupPayload.h>
#import <support/CHIPMem.h>

@implementation CHIPQRCodeSetupPayloadParser {
    NSString * _base41Representation;
    chip::QRCodeSetupPayloadParser * _chipQRCodeSetupPayloadParser;
}

- (id)initWithBase41Representation:(NSString *)base41Representation
{
    if (self = [super init]) {
        if (CHIP_NO_ERROR != chip::Platform::MemoryInit()) {
            CHIP_LOG_ERROR("Error: couldn't initialize platform memory");
            return self;
        }
        _base41Representation = base41Representation;
        _chipQRCodeSetupPayloadParser = new chip::QRCodeSetupPayloadParser(std::string([base41Representation UTF8String]));
    }
    return self;
}

- (CHIPSetupPayload *)populatePayload:(NSError * __autoreleasing *)error
{
    chip::SetupPayload cPlusPluspayload;
    CHIPSetupPayload * payload;

    if (_chipQRCodeSetupPayloadParser) {
        CHIP_ERROR chipError = _chipQRCodeSetupPayloadParser->populatePayload(cPlusPluspayload);

        if (chipError == 0) {
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
