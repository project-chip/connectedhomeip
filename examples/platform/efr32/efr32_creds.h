/**
 * This is a boilerplat header to define the EFR32 authentication credentials.
 * Applications must provide their own version of this header, and include:
 *  - The content of the CSA-provided Certification Declaration
 *  - The location and size of the PAI, and DAC
 *  - The key ID of the key-pair associated with the DAC
 *
 *  These credentials MUST be provided if the build variable "chip_build_device_attestation_credentials" is set to true.
 */
#ifndef EFR32_DEVICE_CREDENTIALS
#define EFR32_DEVICE_CREDENTIALS

#ifndef EFR32_CREDENTIALS_CD_OFFSET
#define EFR32_CREDENTIALS_CD_OFFSET 0x0000
#endif

#ifndef EFR32_CREDENTIALS_CD_SIZE
#define EFR32_CREDENTIALS_CD_SIZE 541
#endif

#ifndef EFR32_CREDENTIALS_PAI_OFFSET
#define EFR32_CREDENTIALS_PAI_OFFSET 0x400
#endif

#ifndef EFR32_CREDENTIALS_PAI_SIZE
#define EFR32_CREDENTIALS_PAI_SIZE 463
#endif

#ifndef EFR32_CREDENTIALS_DAC_OFFSET
#define EFR32_CREDENTIALS_DAC_OFFSET 0x600
#endif

#ifndef EFR32_CREDENTIALS_DAC_SIZE
#define EFR32_CREDENTIALS_DAC_SIZE 492
#endif

#ifndef EFR32_CREDENTIALS_DAC_KEY_ID
#define EFR32_CREDENTIALS_DAC_KEY_ID PSA_KEY_ID_USER_MIN + 1
#endif

#endif // EFR32_DEVICE_CREDENTIALS
