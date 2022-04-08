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
#import "CHIPManualSetupPayloadParser.h"

#import "CHIPError_Internal.h"
#import "CHIPLogging.h"
#import "CHIPSetupPayload_Internal.h"

#import <lib/support/CHIPMem.h>
#import <setup_payload/ManualSetupPayloadParser.h>
#import <setup_payload/SetupPayload.h>

@implementation CHIPManualSetupPayloadParser {
    NSString * _decimalStringRepresentation;
    chip::ManualSetupPayloadParser * _chipManualSetupPayloadParser;
}

- (id)initWithDecimalStringRepresentation:(NSString *)decimalStringRepresentation
{
    if (self = [super init]) {
        if (CHIP_NO_ERROR != chip::Platform::MemoryInit()) {
            CHIP_LOG_ERROR("Error: couldn't initialize platform memory");
            return self;
        }
        _decimalStringRepresentation = decimalStringRepresentation;
        _chipManualSetupPayloadParser = new chip::ManualSetupPayloadParser(std::string([decimalStringRepresentation UTF8String]));
    }
    return self;
}

- (CHIPSetupPayload *)populatePayload:(NSError * __autoreleasing *)error
{
    chip::SetupPayload cPlusPluspayload;
    CHIPSetupPayload * payload;

    if (_chipManualSetupPayloadParser) {
        CHIP_ERROR chipError = _chipManualSetupPayloadParser->populatePayload(cPlusPluspayload);

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
    delete _chipManualSetupPayloadParser;
    _chipManualSetupPayloadParser = nullptr;
    chip::Platform::MemoryShutdown();
}

@end
