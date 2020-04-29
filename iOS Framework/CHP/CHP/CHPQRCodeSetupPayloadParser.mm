//
//  CHPQRCodeSetupPayloadParser.m
//  CHIPQRCodeReader
//
//  Created by Shana Azria on 20/04/2020.
//  Copyright © 2020 CHIP. All rights reserved.
//

#import "CHPQRCodeSetupPayloadParser.h"
#import "CHPError.h"

#import <setup_payload/QRCodeSetupPayloadParser.h>
#import <setup_payload/SetupPayload.h>

@implementation CHPQRCodeSetupPayloadParser {
    NSString *_base45Representation;
    chip::QRCodeSetupPayloadParser *_chipQRCodeSetupPayloadParser;
}

- (id)initWithBase45Representation:(NSString *)base45Representation
{
    if(self = [super init]) {
        _base45Representation = base45Representation;
        _chipQRCodeSetupPayloadParser = new chip::QRCodeSetupPayloadParser(std::string([base45Representation UTF8String]));
    }
    return self;
}

- (CHPSetupPayload *)populatePayload:(NSError *__autoreleasing *)error
{
    chip::SetupPayload cPlusPluspayload;
    CHIP_ERROR chipError = _chipQRCodeSetupPayloadParser->populatePayload(cPlusPluspayload);

    CHPSetupPayload *payload;
    if (chipError == 0) {
        payload = [[CHPSetupPayload alloc] initWithSetupPayload:cPlusPluspayload];
    } else if (error) {
         *error = [CHPError errorForCHIPErrorCode:chipError];
    }
    return payload;
}
@end
