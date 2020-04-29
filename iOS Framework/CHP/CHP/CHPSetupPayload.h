//
//  CHPSetupPayload.h
//  CHIPQRCodeReader
//
//  Created by Shana Azria on 22/04/2020.
//  Copyright © 2020 CHIP. All rights reserved.
//

#import <Foundation/Foundation.h>

#ifdef __cplusplus
#import <setup_payload/SetupPayload.h>
#endif

NS_ASSUME_NONNULL_BEGIN

@interface CHPSetupPayload : NSObject

@property (nonatomic, strong) NSNumber          *version;
@property (nonatomic, strong) NSNumber          *vendorID;
@property (nonatomic, strong) NSNumber          *productID;
@property (nonatomic, assign) BOOL              requiresCustomFlow;
@property (nonatomic, strong) NSNumber          *rendezvousInformation;
@property (nonatomic, strong) NSNumber          *discriminator;
@property (nonatomic, strong) NSNumber          *setUpPINCode;

#ifdef __cplusplus
- (id)initWithSetupPayload:(chip::SetupPayload)setupPayload;
#endif

@end

NS_ASSUME_NONNULL_END
