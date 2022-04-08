//
//  CHIPSetupPayload_Internal.h
//  CHIP
//
//  Copyright © 2021 CHIP. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "CHIPSetupPayload.h"

#ifdef __cplusplus
#import <setup_payload/SetupPayload.h>
#endif

@interface CHIPSetupPayload ()

#ifdef __cplusplus
- (id)initWithSetupPayload:(chip::SetupPayload)setupPayload;
- (CHIPRendezvousInformationFlags)convertRendezvousFlags:(chip::RendezvousInformationFlags)value;
- (CHIPCommissioningFlow)convertCommissioningFlow:(chip::CommissioningFlow)value;
#endif

@end
