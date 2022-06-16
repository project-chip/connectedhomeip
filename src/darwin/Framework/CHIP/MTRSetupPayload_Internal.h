//
//  MTRSetupPayload_Internal.h
//  MTR
//
//  Copyright © 2021 CHIP. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "MTRSetupPayload.h"

#ifdef __cplusplus
#import <setup_payload/SetupPayload.h>
#endif

@interface MTRSetupPayload ()

#ifdef __cplusplus
- (id)initWithSetupPayload:(chip::SetupPayload)setupPayload;
- (MTRRendezvousInformationFlags)convertRendezvousFlags:(chip::RendezvousInformationFlags)value;
- (MTRCommissioningFlow)convertCommissioningFlow:(chip::CommissioningFlow)value;
#endif

@end
