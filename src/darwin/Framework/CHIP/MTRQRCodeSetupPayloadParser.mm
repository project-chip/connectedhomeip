/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
