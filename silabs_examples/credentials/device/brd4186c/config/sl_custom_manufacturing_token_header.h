/***************************************************************************//**
 * @file
 * @brief Custom manufacturing token header
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

/***************************************************************************//**
 * Custom Manufacturing Token Usage Examples
 *
 * The macro DEFINE_MFG_TOKEN() should be used when instantiating a
 * manufacturing token.  Refer to the list of *_LOCATION defines to
 * see what memory is allocated and what memory is unused/available.
 *
 * The _LOCATION is or'ed with either USERDATA_TOKENS or LOCKBITSDATA_TOKENS
 * to control which segment of memory the token is placed in.
 *
 * REMEMBER: By definition, manufacturing tokens exist at fixed addresses.
 *           Tokens should not overlap.
 *
 * Here is a basic example of a manufacturing token header file:
 *
 * Note that the address used here is just an example.  It places the 8 bytes
 * in the middle of the USERDATA space on an EFR32MG12P433F1024GM68.
 * To review addresses of existing tokens when choosing a new address, refer
 * to an961-custom-nodes-efr32.pdf and the files
 * platform/service/token_manager/inc/sl_token_manufacturing_series_1.h
 * platform/service/token_manager/inc/sl_token_manufacturing_series_2.h
 *
 * @code
 * #define CREATOR_MFG_EXAMPLE 0x4242
 * #ifdef DEFINETYPES
 * typedef uint8_t tokTypeMfgExample[8];
 * #endif
 * #ifdef DEFINETOKENS
 * #define MFG_EXAMPLE_LOCATION (USERDATA_TOKENS | 0x2000)
 * DEFINE_MFG_TOKEN(MFG_EXAMPLE,
 *                  tokTypeMfgExample,
 *                  MFG_EXAMPLE_LOCATION,
 *                  {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF})
 * #endif
 * @endcode
 *
 * Since this file contains both the typedefs and the token defs, there are
 * two \#defines used to select which one is needed when this file is included.
 * \#define DEFINETYPES is used to select the type definitions and
 * \#define DEFINETOKENS is used to select the token definitions.
 *
 * To use this example:
 *   Ensure the Token Manager's configuration in Studio
 *   has enabled Custom Manufacuturing Tokens.
 *
 *   Ensure your application has:
 *     #include "sl_token_api.h"
 *     #include "sl_token_manager.h"
 *
 *   This code will read the token data:
 *     uint8_t data[8];
 *     sl_token_get_data(TOKEN_MFG_EXAMPLE, 0, &data, sizeof(data));
 ******************************************************************************/

/*
 #define CREATOR_MFG_EXAMPLE 0x4242
 #ifdef DEFINETYPES
   typedef uint8_t tokTypeMfgExample[8];
 #endif
 #ifdef DEFINETOKENS
 #define MFG_EXAMPLE_LOCATION (USERDATA_TOKENS | 0x2000)
   DEFINE_MFG_TOKEN(MFG_EXAMPLE,
                 tokTypeMfgExample,
                 MFG_EXAMPLE_LOCATION,
                 {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF})
 #endif
 */
