/***************************************************************************//**
 * @file
 * @brief Custom token header
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
 * Custom Tokens Usage Examples
 *
 * #define BASICTOKEN1_DEFAULT                                            0xA5F0
 * #define BASICTOKEN2_DEFAULT                  { { 0xAA, 0xBB, 0xCC }, 0xDDDD }
 * #define BASICTOKEN3_DEFAULT                                         { { 0 } }
 * #define COUNTERTOKEN1_DEFAULT                                               0
 * #define COUNTERTOKEN2_DEFAULT                                      0xCCCCCCCC
 * #define INDEXEDTOKEN1_DEFAULT                                               0
 * #define INDEXEDTOKEN2_DEFAULT                                      { 0xDDDD }
 * #define INDEXEDTOKEN3_DEFAULT    { 0xAA, 0xBBBB, { 0x00, 0x11, 0x22 }, 0xCC }
 *
 * #ifdef DEFINETYPES
 * typedef uint16_t tokTypeBasicToken1;
 * typedef struct {
 *   uint8_t basicToken2Array[3];
 *   uint16_t basicToken2VarA;
 * } tokTypeBasicToken2;
 *
 * typedef struct {
 *   uint8_t basicToken3Array[254];
 * } tokTypeBasicToken3;
 *
 * typedef uint32_t tokTypeCounterToken1;
 * typedef uint32_t tokTypeCounterToken2;
 *
 * typedef uint8_t  tokTypeIndexedToken1Element;
 * typedef uint16_t tokTypeIndexedToken2Element;
 *
 * typedef struct {
 *   uint8_t indexedToken3VarA;
 *   uint16_t indexedToken3VarB;
 *   uint8_t indexedToken3Array[3];
 *   int8_t indexedToken3VarC;
 * } tokTypeIndexedToken3Element;
 *
 * #endif
 *
 * #ifdef DEFINETOKENS
 * DEFINE_BASIC_TOKEN(BASICTOKEN1,
 *                    tokTypeBasicToken1,
 *                    BASICTOKEN1_DEFAULT)
 * DEFINE_BASIC_TOKEN(BASICTOKEN2,
 *                    tokTypeBasicToken2,
 *                    BASICTOKEN2_DEFAULT)
 * DEFINE_BASIC_TOKEN(BASICTOKEN3,
 *                    tokTypeBasicToken3,
 *                    BASICTOKEN3_DEFAULT)
 * DEFINE_COUNTER_TOKEN(COUNTERTOKEN1,
 *                      tokTypeCounterToken1,
 *                      COUNTERTOKEN1_DEFAULT)
 * DEFINE_COUNTER_TOKEN(COUNTERTOKEN2,
 *                      tokTypeCounterToken2,
 *                      COUNTERTOKEN2_DEFAULT)
 * DEFINE_INDEXED_TOKEN(INDEXEDTOKEN1,
 *                      tokTypeIndexedToken1Element,
 *                      INDEXEDTOKEN1_ELEMENTS,
 *                      INDEXEDTOKEN1_DEFAULT)
 * DEFINE_INDEXED_TOKEN(INDEXEDTOKEN2,
 *                      tokTypeIndexedToken2Element,
 *                      INDEXEDTOKEN2_ELEMENTS,
 *                      INDEXEDTOKEN2_DEFAULT)
 * DEFINE_INDEXED_TOKEN(INDEXEDTOKEN3,
 *                      tokTypeIndexedToken3Element,
 *                      INDEXEDTOKEN3_ELEMENTS,
 *                      INDEXEDTOKEN3_DEFAULT)
 * #endif
 *
 ******************************************************************************/
