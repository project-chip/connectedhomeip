//
//  CHIPSetupPayload.m
//  CHIPQRCodeReader
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

#import "CHIPSetupPayload.h"

@implementation CHIPSetupPayload

- (void)populatePayloadFields:(chip::SetupPayload)setupPayload
{
    
}

//- (id)initWithPayloadAtPath:(NSString *)filePath
//{
//    if(self = [super init]) {
//        chip::SetupPayload cPlusPluspayload;
//        CHIP_ERROR chipError = cPlusPluspayload.loadPayloadFromFile([filePath UTF8String]);
//        if (chipError == CHIP_NO_ERROR) {
//            [self populatePayloadFields:cPlusPluspayload];
//        } else {
//        }
//    }
//    return self;
//}

-(NSString *)description
{
    return [NSString stringWithFormat:@"<CHIPSetupPayload: %p>\n version: %@\n vendorID: %@\n productID: %@\n requiresCustomFlow: %@\n rendezvouInfo: %@\n discriminator: %@\n setUpPinCode: %@\n", self,
            _version, _vendorID, _productID,
            _requiresCustomFlow ? @"YES" : @"NO", _rendezvousInformation,
            _discriminator, _setUpPINCode];
}

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
