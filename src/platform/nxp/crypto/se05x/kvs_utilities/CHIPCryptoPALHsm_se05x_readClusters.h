/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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

/**
 *    @file
 *          KVS Implementation for SE05x
 *          The functions are used to read the NFC commissioned data from SE05x.
 */

#include <cstdint>
#include <lib/core/CHIPError.h>
#include <string>
#include <vector>

/**
 * @brief The function is used to get the fabric id.
 * @return Fabric id (0 if no NFC commissioning done)
 */
uint32_t se05x_get_fabric_id();

/**
 * @brief The function is used to check if the NFC commissioning is done or not.
 * @return CHIP_ERROR_INTERNAL on no nfc commissioning, CHIP_NO_ERROR on nfc commissioning.
 */
CHIP_ERROR se05x_is_nfc_commissioning_done();

/**
 * @brief Read node operational key pair in the form of refeence key from SE05x.
 * @param[out] noc_buf - input buffer for node operational key pair in reference key format
 * @param[in,out] noc_buf_len - input buffer length when input and reference key length when output
 * @return CHIP_ERROR_INTERNAL on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR se05x_read_node_operational_keypair(uint8_t * op_key_ref_key, size_t * op_key_ref_key_len);

/**
 * @brief Read node operational certificate from SE05x.
 * @param[out] noc_buf - input buffer for node operational certificate
 * @param[in,out] noc_buf_len - input buffer length when input and noc length when output
 * @return CHIP_ERROR_INTERNAL on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR se05x_read_node_oper_cert(uint8_t * noc_buf, size_t * noc_buf_len);

/**
 * @brief Read node operational certificate from SE05x.
 * @param[out] root_cert_buf - input buffer for root certificate
 * @param[in,out] root_cert_buf_len - input buffer length when input and root cert length when output
 * @return CHIP_ERROR_INTERNAL on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR se05x_read_root_cert(uint8_t * root_cert_buf, size_t * root_cert_buf_len);

/**
 * @brief Read ICA certificate from SE05x.
 * @param[out] ica_buf - input buffer for ICA certificate
 * @param[in,out] ica_buf_len - input buffer length when input and ICA cert length when output
 * @return CHIP_ERROR_INTERNAL on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR se05x_read_ICA(uint8_t * ica_buf, size_t * ica_buf_len);

/**
 * @brief Read IPK from SE05x.
 * @param[out] ipk_buf - input buffer for IPK
 * @param[in,out] ipk_buf_len - input buffer length when input and IPK length when output
 * @return CHIP_ERROR_INTERNAL on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR se05x_read_ipk(uint8_t * ipk_buf, size_t * ipk_buf_len);

/**
 * @brief Read access control list from SE05x.
 * @param[out] acl - input buffer for ACL data
 * @param[in,out] acl_len - input buffer length when input and ACL data length when output
 * @return CHIP_ERROR_INTERNAL on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR se05x_read_acl_data(uint8_t * acl, size_t * acl_len);

/**
 * @brief Read fabric group info from SE05x.
 * @param[out] fabgrp_data - input buffer for fabric group data
 * @param[in,out] fabgrp_data_len - input buffer length when input and fabric data length when output
 * @return CHIP_ERROR_INTERNAL on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR se05x_read_fabric_groups(uint8_t * fabgrp_data, size_t * fabgrp_data_len);

/**
 * @brief Read meta data info from SE05x.
 * @param[out] meta_data - input buffer for meta data
 * @param[in,out] meta_data_len - input buffer length when input and meta data length when output
 * @return CHIP_ERROR_INTERNAL on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR se05x_read_meta_data(uint8_t * meta_data, size_t * meta_data_len);

/**
 * @brief Read fabric index from SE05x.
 * @param[out] fab_info_data - input buffer for fabric index
 * @param[in,out] fab_info_data_len - input buffer length when input and fabric index length when output
 * @return CHIP_ERROR_INTERNAL on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR se05x_read_fabric_index_info_data(uint8_t * fab_info_data, size_t * fab_info_data_len);

/**
 * @brief Read wifi credentials info from SE05x.
 * @param[in] buf - input buffer for reading binary file
 * @param[in] buflen - input buffer length
 * @param[in,out] ssid - input buffer for ssid
 * @param[in,out] ssid_len - ssid input buffer length when input and ssid length when output
 * @param[in,out] password - input buffer for password
 * @param[in,out] password_len - password input buffer length when input and password length when output
 * @return CHIP_ERROR_INTERNAL on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR se05x_read_wifi_credentials(uint8_t * buf, size_t buflen, char * ssid, size_t * ssid_len, char * password,
                                       size_t * password_len);
