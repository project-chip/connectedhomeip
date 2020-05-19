#import <Foundation/Foundation.h>

#ifdef __cplusplus
#import "SetupPayload.h"
#endif

NS_ASSUME_NONNULL_BEGIN

@interface CHIPSetupPayload : NSObject

@property (nonatomic, strong) NSNumber * version;
@property (nonatomic, strong) NSNumber * vendorID;
@property (nonatomic, strong) NSNumber * productID;
@property (nonatomic, assign) BOOL requiresCustomFlow;
@property (nonatomic, strong) NSNumber * rendezvousInformation;
@property (nonatomic, strong) NSNumber * discriminator;
@property (nonatomic, strong) NSNumber * setUpPINCode;

- (id)initWithPayloadAtPath:(NSString *)filePath;

#ifdef __cplusplus
- (id)initWithSetupPayload:(chip::SetupPayload)setupPayload;
#endif

@end

NS_ASSUME_NONNULL_END
