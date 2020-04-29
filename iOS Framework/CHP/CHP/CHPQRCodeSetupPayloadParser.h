//
//  CHPQRCodeSetupPayloadParser.h
//  CHIPQRCodeReader
//
//  Created by Shana Azria on 20/04/2020.
//  Copyright © 2020 CHIP. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "CHPSetupPayload.h"
#import "CHPError.h"

NS_ASSUME_NONNULL_BEGIN

@interface CHPQRCodeSetupPayloadParser : NSObject
- (id)initWithBase45Representation:(NSString *)base45Representation;
- (CHPSetupPayload *)populatePayload:(NSError *__autoreleasing *)error;
@end
;NS_ASSUME_NONNULL_END
