/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#import "MTRManualSetupPayloadParser.h"

#import "MTRError_Internal.h"
#import "MTRFramework.h"
#import "MTRLogging_Internal.h"
#import "MTRSetupPayload_Internal.h"

#import <setup_payload/ManualSetupPayloadParser.h>
#import <setup_payload/SetupPayload.h>

@implementation MTRManualSetupPayloadParser {
    NSString * _decimalStringRepresentation;
    chip::ManualSetupPayloadParser * _chipManualSetupPayloadParser;
}

+ (void)initialize
{
    MTRFrameworkInit();
}

- (id)initWithDecimalStringRepresentation:(NSString *)decimalStringRepresentation
{
    if (self = [super init]) {
        _decimalStringRepresentation = decimalStringRepresentation;
        _chipManualSetupPayloadParser = new chip::ManualSetupPayloadParser(std::string([decimalStringRepresentation UTF8String]));
    }
    return self;
}

- (MTRSetupPayload *)populatePayload:(NSError * __autoreleasing *)error
{
    chip::SetupPayload cPlusPluspayload;
    MTRSetupPayload * payload;

    if (_chipManualSetupPayloadParser) {
        CHIP_ERROR chipError = _chipManualSetupPayloadParser->populatePayload(cPlusPluspayload);

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
    delete _chipManualSetupPayloadParser;
    _chipManualSetupPayloadParser = nullptr;
}

@end
