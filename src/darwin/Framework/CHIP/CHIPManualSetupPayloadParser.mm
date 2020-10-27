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

#import "CHIPError.h"
#import "CHIPLogging.h"

#import <setup_payload/ManualSetupPayloadParser.h>
#import <setup_payload/SetupPayload.h>
#import <support/CHIPMem.h>

@implementation CHIPManualSetupPayloadParser {
    NSString * _decimalStringRepresentation;
    chip::ManualSetupPayloadParser * _chipManualSetupPayloadParser;
}

- (id)initWithDecimalStringRepresentation:(NSString *)decimalStringRepresentation
{
    if (self = [super init]) {
        _decimalStringRepresentation = decimalStringRepresentation;
        _chipManualSetupPayloadParser = new chip::ManualSetupPayloadParser(std::string([decimalStringRepresentation UTF8String]));
        if (CHIP_NO_ERROR != chip::Platform::MemoryInit()) {
               CHIP_LOG_ERROR("Error: couldn't initialize platform memory");
               delete _chipManualSetupPayloadParser;
        }
    }
    return self;
}

- (CHIPSetupPayload *)populatePayload:(NSError * __autoreleasing *)error
{
    chip::SetupPayload cPlusPluspayload;
    CHIP_ERROR chipError = _chipManualSetupPayloadParser->populatePayload(cPlusPluspayload);

    CHIPSetupPayload * payload;
    if (chipError == 0) {
        payload = [[CHIPSetupPayload alloc] initWithSetupPayload:cPlusPluspayload];
    } else if (error) {
        *error = [CHIPError errorForCHIPErrorCode:chipError];
    }

    return payload;
}

-(void)dealloc {
    delete _chipManualSetupPayloadParser;
    chip::Platform::MemoryShutdown();
}

@end
