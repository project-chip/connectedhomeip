/**
 *  To enable these credentias, compile the app with option
 *  "chip_build_device_attestation_credentials=true".
 */

#ifndef SILABS_PROVISION_VERSION_1_0
#define SILABS_PROVISION_VERSION_1_0    0
#endif

#ifndef SILABS_CREDENTIALS_DAC_OFFSET
#define SILABS_CREDENTIALS_DAC_OFFSET   0x0000
#endif

#ifndef SILABS_CREDENTIALS_DAC_SIZE
#define SILABS_CREDENTIALS_DAC_SIZE     481
#endif

#ifndef SILABS_CREDENTIALS_PAI_OFFSET
#define SILABS_CREDENTIALS_PAI_OFFSET   0x0200
#endif

#ifndef SILABS_CREDENTIALS_PAI_SIZE
#define SILABS_CREDENTIALS_PAI_SIZE     443
#endif

#ifndef SILABS_CREDENTIALS_CD_OFFSET
#define SILABS_CREDENTIALS_CD_OFFSET    0x0400
#endif

#ifndef SILABS_CREDENTIALS_CD_SIZE
#define SILABS_CREDENTIALS_CD_SIZE      244
#endif
