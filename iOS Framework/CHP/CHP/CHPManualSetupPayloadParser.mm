//
//  CHPManualSetupPayloadParser.m
//  CHIPQRCodeReader
//
//  Created by Shana Azria on 22/04/2020.
//  Copyright © 2020 CHIP. All rights reserved.
//

#import "CHPManualSetupPayloadParser.h"

#import <setup_payload/ManualSetupPayloadParser.h>
#import <setup_payload/SetupPayload.h>

@implementation CHPManualSetupPayloadParser {
    NSString *_decimalStringRepresentation;
    chip::ManualSetupPayloadParser *_chipManualSetupPayloadParser;
}

- (id)initWithDecimalStringRepresentation:(NSString *)decimalStringRepresentation
{
    if(self = [super init]) {
        _decimalStringRepresentation = decimalStringRepresentation;
        _chipManualSetupPayloadParser = new chip::ManualSetupPayloadParser(std::string([decimalStringRepresentation UTF8String]));
    }
    return self;
}

- (CHPSetupPayload *)populatePayload:(NSError *__autoreleasing *)error
{
    chip::SetupPayload cPlusPluspayload;
    CHIP_ERROR chipError = _chipManualSetupPayloadParser->populatePayload(cPlusPluspayload);

    CHPSetupPayload *payload;
    if (chipError == 0) {
        payload = [[CHPSetupPayload alloc] initWithSetupPayload:cPlusPluspayload];
    } else if (error) {
        *error = [CHPError errorForCHIPErrorCode:chipError];
    }
    
    return payload;
}

@end
