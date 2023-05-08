/**
 * This is a boilerplat header to define the SILABS authentication credentials.
 * Applications must provide their own version of this header, and include:
 *  - The content of the CSA-provided Certification Declaration
 *  - The location and size of the PAI, and DAC
 *  - The key ID of the key-pair associated with the DAC
 *
 *  These credentials MUST be provided if the build variable "chip_build_device_attestation_credentials" is set to true.
 */
#ifndef SILABS_DEVICE_CREDENTIALS
#define SILABS_DEVICE_CREDENTIALS

#ifndef SILABS_CREDENTIALS_DAC_KEY_ID
#define SILABS_CREDENTIALS_DAC_KEY_ID 0x0002
#endif

#ifndef SILABS_CREDENTIALS_DAC_OFFSET
#define SILABS_CREDENTIALS_DAC_OFFSET 0x0000
#endif

#ifndef SILABS_CREDENTIALS_DAC_SIZE
#define SILABS_CREDENTIALS_DAC_SIZE 0
#endif

#ifndef SILABS_CREDENTIALS_PAI_OFFSET
#define SILABS_CREDENTIALS_PAI_OFFSET 0x0200
#endif

#ifndef SILABS_CREDENTIALS_PAI_SIZE
#define SILABS_CREDENTIALS_PAI_SIZE 0
#endif

#ifndef SILABS_CREDENTIALS_CD_OFFSET
#define SILABS_CREDENTIALS_CD_OFFSET 0x0400
#endif

#ifndef SILABS_CREDENTIALS_CD_SIZE
#define SILABS_CREDENTIALS_CD_SIZE 0
#endif

#endif // SILABS_DEVICE_CREDENTIALS
