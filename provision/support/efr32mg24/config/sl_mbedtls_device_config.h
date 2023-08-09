#ifndef SL_MBEDTLS_DEVICE_CONFIG_H
#define SL_MBEDTLS_DEVICE_CONFIG_H

// -----------------------------------------------------------------------------
// User exposed config options

// <<< Use Configuration Wizard in Context Menu >>>

// <h> Secure Engine (SE) version configuration

// <o SL_SE_SUPPORT_FW_PRIOR_TO_1_2_2> Support SE firmware versions older than 1.2.2 <0-1>
// <i> Enable software fallback for ECDH and ECC public key validation on xG21
// <i> devices running SE firmware versions lower than 1.2.2.
// <i>
// <i> Due to other stability concerns, it is strongly recommended to upgrade
// <i> these devices to the latest firmware revision instead of turning on
// <i> software fallback support.
// <i>
// <i> Not having fallback support will make ECDH operations, as well as PSA
// <i> Crypto public key import, return an error code on affected devices.
// <i>
// <i> Default: 0
#define SL_SE_SUPPORT_FW_PRIOR_TO_1_2_2 0

// <o SL_SE_ASSUME_FW_AT_LEAST_1_2_2> Assume an SE firmware version newer than 1.2.2 <0-1>
// <i> For enhanced performance: if it is guaranteed that all devices on which
// <i> this library will run are updated to at least SE FW 1.2.2, then turning
// <i> on this option will remove certain fallback checks, thereby reducing the
// <i> amount of processing required for ECDH and public key verification
// <i> operations.
// <i> Default: 0
#define SL_SE_ASSUME_FW_AT_LEAST_1_2_2  0

// <o SL_SE_ASSUME_FW_UNAFFECTED_BY_ED25519_ERRATA> Assume an SE firmware version that is unaffected by Ed25519 errata <0-1>
// <i> For minimal code size and performance savings: if it is guaranteed that
// <i> none of the devices running this library has SE FWs in the range
// <i> [1.2.2, 1.2.8], then enabling this option will disable runtime version
// <i> checks.
// <i> Default: 0
#define SL_SE_ASSUME_FW_UNAFFECTED_BY_ED25519_ERRATA  0

// </h>

// <<< end of configuration section >>>

// -----------------------------------------------------------------------------
// Additional SE version related logic (DO NOT MODIFY)

// SL_SE_ASSUME_FW_AT_LEAST_1_2_10 is no longer in use, however, it is kept here
// for backwards compatibility. */
#if defined(SL_SE_ASSUME_FW_AT_LEAST_1_2_10)
  #undef SL_SE_ASSUME_FW_AT_LEAST_1_2_2
  #define SL_SE_ASSUME_FW_AT_LEAST_1_2_2  1
  #undef SL_SE_ASSUME_FW_UNAFFECTED_BY_ED25519_ERRATA
  #define SL_SE_ASSUME_FW_UNAFFECTED_BY_ED25519_ERRATA  1
#endif

// SLI_SE_SUPPORT_FW_PRIOR_TO_1_2_2 is no longer in use, however, it is kept
// here for backwards compatibility. */
#if defined(SLI_SE_SUPPORT_FW_PRIOR_TO_1_2_2)
  #undef SL_SE_SUPPORT_FW_PRIOR_TO_1_2_2
  #define SL_SE_SUPPORT_FW_PRIOR_TO_1_2_2  1
#endif

// SLI_SE_ASSUME_FW_AT_LEAST_1_2_2 is no longer in use, however, it is kept
// here for backwards compatibility. */
#if defined(SLI_SE_ASSUME_FW_AT_LEAST_1_2_2)
  #undef SL_SE_ASSUME_FW_AT_LEAST_1_2_2
  #define SL_SE_ASSUME_FW_AT_LEAST_1_2_2  1
#endif

#endif // SL_MBEDTLS_DEVICE_CONFIG_H
