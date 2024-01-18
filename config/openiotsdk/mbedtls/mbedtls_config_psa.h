
#define MBEDTLS_SHA1_C
#define PSA_WANT_ALG_SHA_1
#define MBEDTLS_ECP_WINDOW_SIZE 6 /**< Maximum window size used */
#undef MBEDTLS_ECP_FIXED_POINT_OPTIM
#define MBEDTLS_ECP_FIXED_POINT_OPTIM 1 /**< Enable fixed-point speed-up */
