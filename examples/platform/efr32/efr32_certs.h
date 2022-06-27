#ifndef MATTER_MFG_TOKENS_EFR32
#define MATTER_MFG_TOKENS_EFR32

#include "psa/crypto.h"

#define EFR32_CERTS_DAC_ID                  PSA_KEY_ID_USER_MIN + 1

#define CREATOR_MFG_MATTER_CD               (USERDATA_TOKENS | 0x200)  //   4 bytes
#define CREATOR_MFG_MATTER_PAI              (USERDATA_TOKENS | 0x420)  //   4 bytes
#define CREATOR_MFG_MATTER_DAC              (USERDATA_TOKENS | 0x5F0)  //   4 bytes

#define MFG_MATTER_CD_SIZE                  541
#define MFG_MATTER_PAI_SIZE                 463
#define MFG_MATTER_DAC_SIZE                 492
#define MFG_MATTER_DAC_KEY_ID               PSA_KEY_ID_USER_MIN + 1


#endif // MATTER_MFG_TOKENS_EFR32