/**
 *
 *    Copyright (c) 2020 Silicon Labs
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/***************************************************************************//**
 * @file
 * @brief This file defines various security parameters based on the security
 * profile setup via App. Builder.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_SECURITY_CONFIG_H
#define SILABS_SECURITY_CONFIG_H

#if defined(EMBER_AF_HAS_SECURITY_PROFILE_SE_TEST) || defined(EMBER_AF_HAS_SECURITY_PROFILE_SE_FULL)
  #define EMBER_AF_HAS_SECURITY_PROFILE_SE
#endif

// This key is "ZigBeeAlliance09"
#define ZIGBEE_PROFILE_INTEROPERABILITY_LINK_KEY       \
  {                                                    \
    { 0x5A, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41, 0x6C,  \
      0x6C, 0x69, 0x61, 0x6E, 0x63, 0x65, 0x30, 0x39 } \
  }

#define SE_SECURITY_TEST_LINK_KEY                      \
  {                                                    \
    { 0x56, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,  \
      0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77 } \
  }

#define DUMMY_KEY                                      \
  {                                                    \
    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  \
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } \
  }

// None.
#define EMBER_AF_SECURITY_PROFILE_NONE_TC_SECURITY_BITMASK            0
#define EMBER_AF_SECURITY_PROFILE_NONE_TC_EXTENDED_SECURITY_BITMASK   0
#define EMBER_AF_SECURITY_PROFILE_NONE_NODE_SECURITY_BITMASK          0
#define EMBER_AF_SECURITY_PROFILE_NONE_NODE_EXTENDED_SECURITY_BITMASK 0
#define EMBER_AF_SECURITY_PROFILE_NONE_TC_LINK_KEY_REQUEST_POLICY     EMBER_AF_DENY_TC_KEY_REQUESTS
#define EMBER_AF_SECURITY_PROFILE_NONE_APP_LINK_KEY_REQUEST_POLICY    EMBER_AF_DENY_APP_KEY_REQUESTS
#define EMBER_AF_SECURITY_PROFILE_NONE_PRECONFIGURED_KEY              DUMMY_KEY

// HA.
#define EMBER_AF_SECURITY_PROFILE_HA_TC_SECURITY_BITMASK            (EMBER_TRUST_CENTER_GLOBAL_LINK_KEY \
                                                                     | EMBER_HAVE_PRECONFIGURED_KEY     \
                                                                     | EMBER_HAVE_NETWORK_KEY           \
                                                                     | EMBER_NO_FRAME_COUNTER_RESET     \
                                                                     | EMBER_REQUIRE_ENCRYPTED_KEY)
#define EMBER_AF_SECURITY_PROFILE_HA_TC_EXTENDED_SECURITY_BITMASK   EMBER_JOINER_GLOBAL_LINK_KEY
#define EMBER_AF_SECURITY_PROFILE_HA_NODE_SECURITY_BITMASK          (EMBER_TRUST_CENTER_GLOBAL_LINK_KEY \
                                                                     | EMBER_HAVE_PRECONFIGURED_KEY     \
                                                                     | EMBER_NO_FRAME_COUNTER_RESET     \
                                                                     | EMBER_REQUIRE_ENCRYPTED_KEY)
#define EMBER_AF_SECURITY_PROFILE_HA_NODE_EXTENDED_SECURITY_BITMASK EMBER_JOINER_GLOBAL_LINK_KEY
#define EMBER_AF_SECURITY_PROFILE_HA_TC_LINK_KEY_REQUEST_POLICY     EMBER_AF_DENY_TC_KEY_REQUESTS
#define EMBER_AF_SECURITY_PROFILE_HA_APP_LINK_KEY_REQUEST_POLICY    EMBER_AF_ALLOW_APP_KEY_REQUESTS
#define EMBER_AF_SECURITY_PROFILE_HA_PRECONFIGURED_KEY              ZIGBEE_PROFILE_INTEROPERABILITY_LINK_KEY

// HA1.2.
#define EMBER_AF_SECURITY_PROFILE_HA12_TC_SECURITY_BITMASK            (EMBER_HAVE_NETWORK_KEY \
                                                                       | EMBER_REQUIRE_ENCRYPTED_KEY)
#define EMBER_AF_SECURITY_PROFILE_HA12_TC_EXTENDED_SECURITY_BITMASK   0
#define EMBER_AF_SECURITY_PROFILE_HA12_NODE_SECURITY_BITMASK          (EMBER_GET_PRECONFIGURED_KEY_FROM_INSTALL_CODE \
                                                                       | EMBER_REQUIRE_ENCRYPTED_KEY)
#define EMBER_AF_SECURITY_PROFILE_HA12_NODE_EXTENDED_SECURITY_BITMASK 0
#define EMBER_AF_SECURITY_PROFILE_HA12_TC_LINK_KEY_REQUEST_POLICY     EMBER_AF_DENY_TC_KEY_REQUESTS
#define EMBER_AF_SECURITY_PROFILE_HA12_APP_LINK_KEY_REQUEST_POLICY    EMBER_AF_ALLOW_APP_KEY_REQUESTS
#define EMBER_AF_SECURITY_PROFILE_HA12_PRECONFIGURED_KEY              DUMMY_KEY

// SE Security Test.
#define EMBER_AF_SECURITY_PROFILE_SE_TEST_TC_SECURITY_BITMASK            (EMBER_TRUST_CENTER_GLOBAL_LINK_KEY \
                                                                          | EMBER_HAVE_NETWORK_KEY           \
                                                                          | EMBER_HAVE_PRECONFIGURED_KEY     \
                                                                          | EMBER_REQUIRE_ENCRYPTED_KEY)
#define EMBER_AF_SECURITY_PROFILE_SE_TEST_TC_EXTENDED_SECURITY_BITMASK   EMBER_NWK_LEAVE_REQUEST_NOT_ALLOWED
#define EMBER_AF_SECURITY_PROFILE_SE_TEST_NODE_SECURITY_BITMASK          (EMBER_TRUST_CENTER_GLOBAL_LINK_KEY \
                                                                          | EMBER_HAVE_PRECONFIGURED_KEY     \
                                                                          | EMBER_REQUIRE_ENCRYPTED_KEY)
#define EMBER_AF_SECURITY_PROFILE_SE_TEST_NODE_EXTENDED_SECURITY_BITMASK EMBER_NWK_LEAVE_REQUEST_NOT_ALLOWED
#define EMBER_AF_SECURITY_PROFILE_SE_TEST_TC_LINK_KEY_REQUEST_POLICY     EMBER_AF_DENY_TC_KEY_REQUESTS
#define EMBER_AF_SECURITY_PROFILE_SE_TEST_APP_LINK_KEY_REQUEST_POLICY    EMBER_AF_ALLOW_APP_KEY_REQUESTS
#define EMBER_AF_SECURITY_PROFILE_SE_TEST_PRECONFIGURED_KEY              SE_SECURITY_TEST_LINK_KEY

// SE Security Full.
#define EMBER_AF_SECURITY_PROFILE_SE_FULL_TC_SECURITY_BITMASK            (EMBER_HAVE_NETWORK_KEY \
                                                                          | EMBER_REQUIRE_ENCRYPTED_KEY)
#define EMBER_AF_SECURITY_PROFILE_SE_FULL_TC_EXTENDED_SECURITY_BITMASK   EMBER_NWK_LEAVE_REQUEST_NOT_ALLOWED
#define EMBER_AF_SECURITY_PROFILE_SE_FULL_NODE_SECURITY_BITMASK          (EMBER_GET_PRECONFIGURED_KEY_FROM_INSTALL_CODE \
                                                                          | EMBER_REQUIRE_ENCRYPTED_KEY)
#define EMBER_AF_SECURITY_PROFILE_SE_FULL_NODE_EXTENDED_SECURITY_BITMASK EMBER_NWK_LEAVE_REQUEST_NOT_ALLOWED
#define EMBER_AF_SECURITY_PROFILE_SE_FULL_TC_LINK_KEY_REQUEST_POLICY     EMBER_AF_DENY_TC_KEY_REQUESTS
#define EMBER_AF_SECURITY_PROFILE_SE_FULL_APP_LINK_KEY_REQUEST_POLICY    EMBER_AF_ALLOW_APP_KEY_REQUESTS
#define EMBER_AF_SECURITY_PROFILE_SE_FULL_PRECONFIGURED_KEY              DUMMY_KEY

// Z3 security.
#define EMBER_AF_SECURITY_PROFILE_Z3_TC_SECURITY_BITMASK            (EMBER_TRUST_CENTER_GLOBAL_LINK_KEY        \
                                                                     | EMBER_HAVE_PRECONFIGURED_KEY            \
                                                                     | EMBER_HAVE_NETWORK_KEY                  \
                                                                     | EMBER_NO_FRAME_COUNTER_RESET            \
                                                                     | EMBER_TRUST_CENTER_USES_HASHED_LINK_KEY \
                                                                     | EMBER_REQUIRE_ENCRYPTED_KEY)
#define EMBER_AF_SECURITY_PROFILE_Z3_TC_EXTENDED_SECURITY_BITMASK   EMBER_NWK_LEAVE_REQUEST_NOT_ALLOWED
#define EMBER_AF_SECURITY_PROFILE_Z3_NODE_SECURITY_BITMASK          (EMBER_TRUST_CENTER_GLOBAL_LINK_KEY \
                                                                     | EMBER_HAVE_PRECONFIGURED_KEY     \
                                                                     | EMBER_REQUIRE_ENCRYPTED_KEY      \
                                                                     | EMBER_NO_FRAME_COUNTER_RESET)
#define EMBER_AF_SECURITY_PROFILE_Z3_NODE_EXTENDED_SECURITY_BITMASK (EMBER_JOINER_GLOBAL_LINK_KEY \
                                                                     | EMBER_EXT_NO_FRAME_COUNTER_RESET)
#define EMBER_AF_SECURITY_PROFILE_Z3_TC_LINK_KEY_REQUEST_POLICY     EMBER_AF_DENY_TC_KEY_REQUESTS
#define EMBER_AF_SECURITY_PROFILE_Z3_APP_LINK_KEY_REQUEST_POLICY    EMBER_AF_DENY_APP_KEY_REQUESTS
#define EMBER_AF_SECURITY_PROFILE_Z3_PRECONFIGURED_KEY              ZIGBEE_PROFILE_INTEROPERABILITY_LINK_KEY

// Custom security.
#define EMBER_AF_SECURITY_PROFILE_CUSTOM_TC_SECURITY_BITMASK            0
#define EMBER_AF_SECURITY_PROFILE_CUSTOM_TC_EXTENDED_SECURITY_BITMASK   0
#define EMBER_AF_SECURITY_PROFILE_CUSTOM_NODE_SECURITY_BITMASK          0
#define EMBER_AF_SECURITY_PROFILE_CUSTOM_NODE_EXTENDED_SECURITY_BITMASK 0
#define EMBER_AF_SECURITY_PROFILE_CUSTOM_TC_LINK_KEY_REQUEST_POLICY     EMBER_AF_DENY_TC_KEY_REQUESTS
#define EMBER_AF_SECURITY_PROFILE_CUSTOM_APP_LINK_KEY_REQUEST_POLICY    EMBER_AF_DENY_APP_KEY_REQUESTS
#define EMBER_AF_SECURITY_PROFILE_CUSTOM_PRECONFIGURED_KEY              DUMMY_KEY

#endif // SILABS_SECURITY_CONFIG_H
