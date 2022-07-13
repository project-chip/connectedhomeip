#include "CHIPCommandStorageDelegate.h"
#import <Matter/Matter.h>
#include <crypto/CHIPCryptoPAL.h>

@interface CHIPToolKeypair : NSObject <MTRKeypair>
- (BOOL)initialize;
- (NSData *)signMessageECDSA_RAW:(NSData *)message;
- (SecKeyRef)publicKey;
- (CHIP_ERROR)Serialize:(chip::Crypto::P256SerializedKeypair &)output;
- (CHIP_ERROR)Deserialize:(chip::Crypto::P256SerializedKeypair &)input;
- (CHIP_ERROR)createOrLoadKeys:(CHIPToolPersistentStorageDelegate *)storage;
- (NSData *)getIPK;

@end
