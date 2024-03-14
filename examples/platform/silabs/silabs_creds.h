/**
 *  To enable these credentias, compile the app with option
 *  "chip_build_device_attestation_credentials=true".
 */

#ifndef SL_PROVISION_VERSION_1_0
#define SL_PROVISION_VERSION_1_0    0
#endif

#ifndef SL_CREDENTIALS_DAC_OFFSET
#define SL_CREDENTIALS_DAC_OFFSET   0x0000
#endif

#ifndef SL_CREDENTIALS_DAC_SIZE
#define SL_CREDENTIALS_DAC_SIZE     0
#endif

#ifndef SL_CREDENTIALS_PAI_OFFSET
#define SL_CREDENTIALS_PAI_OFFSET   0x0200
#endif

#ifndef SL_CREDENTIALS_PAI_SIZE
#define SL_CREDENTIALS_PAI_SIZE     0
#endif

#ifndef SL_CREDENTIALS_CD_OFFSET
#define SL_CREDENTIALS_CD_OFFSET    0x0400
#endif

#ifndef SL_CREDENTIALS_CD_SIZE
#define SL_CREDENTIALS_CD_SIZE      0
#endif
