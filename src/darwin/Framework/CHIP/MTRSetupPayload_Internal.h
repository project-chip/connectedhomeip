//
//  MTRSetupPayload_Internal.h
//  MTR
//
//  Copyright © 2021 CHIP. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "MTRSetupPayload.h"

#ifdef __cplusplus
#import <lib/core/Optional.h>
#import <setup_payload/SetupPayload.h>
#endif

@interface MTRSetupPayload () <NSSecureCoding>

#ifdef __cplusplus
- (id)initWithSetupPayload:(chip::SetupPayload)setupPayload;
- (NSNumber *)convertRendezvousFlags:(const chip::Optional<chip::RendezvousInformationFlags> &)value;
- (MTRCommissioningFlow)convertCommissioningFlow:(chip::CommissioningFlow)value;
#endif

@end
