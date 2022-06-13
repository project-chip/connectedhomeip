#include "CHIPCommandStorageDelegate.h"
#import <CHIP/CHIP.h>
#include <crypto/CHIPCryptoPAL.h>

@interface CHIPToolKeypair : NSObject <CHIPKeypair>
- (BOOL)initialize;
- (NSData *)ECDSA_sign_message_raw:(NSData *)message;
- (SecKeyRef)pubkey;
- (CHIP_ERROR)Serialize:(chip::Crypto::P256SerializedKeypair &)output;
- (CHIP_ERROR)Deserialize:(chip::Crypto::P256SerializedKeypair &)input;
- (CHIP_ERROR)createOrLoadKeys:(CHIPToolPersistentStorageDelegate *)storage;
- (NSData *)getIPK;

@end
