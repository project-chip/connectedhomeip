//
//  CHPSetupPayload.m
//  CHIPQRCodeReader
//
//  Created by Shana Azria on 22/04/2020.
//  Copyright © 2020 CHIP. All rights reserved.
//

#import "CHPSetupPayload.h"

@implementation CHPSetupPayload

- (id)initWithSetupPayload:(chip::SetupPayload)setupPayload
{
    if(self = [super init]) {
        _version = [NSNumber numberWithUnsignedChar:setupPayload.version];
        _vendorID = [NSNumber numberWithUnsignedShort:setupPayload.vendorID];
        _productID = [NSNumber numberWithUnsignedShort:setupPayload.productID];
        _requiresCustomFlow = setupPayload.requiresCustomFlow == 1;
        _rendezvousInformation = [NSNumber numberWithUnsignedShort:setupPayload.rendezvousInformation];
        _discriminator = [NSNumber numberWithUnsignedShort:setupPayload.discriminator];
        _setUpPINCode = [NSNumber numberWithUnsignedLong:setupPayload.setUpPINCode];
    }
    return self;
}

@end
