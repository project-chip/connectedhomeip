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
 * @brief The function is used to check if the NFC commissioning is done or not.
 * @return CHIP_ERROR_INTERNAL on no nfc commissioning, CHIP_NO_ERROR on nfc commissioning.
 */
CHIP_ERROR se05x_is_nfc_commissioning_done();

/**
 * @brief Read operational credentials data from SE05x.
 * Read Node Operational Certificate, ICA Certificate, Identity Protection key.
 * @param[out] nocKey - key name for Node Operational Certificate in chip_kvs file
 * @param[out] nocBuf - Node Operational Certificate buffer
 * @param[out] nocBuf - Node Operational Certificate buffer length
 * @param[out] rootCertKey - key name for Root Certificate in chip_kvs file
 * @param[out] rootCertBuf - Root Certificate buffer
 * @param[out] rootCertBufLen - Root Certificate buffer length
 * @param[out] icackey - key name for Intermediate certificate Authority in chip_kvs file
 * @param[out] icacBuf - Intermediate Certificate Authority buffer
 * @param[out] icacBufLen - Intermediate Certificate Authority buffer length
 * @param[out] ipKey - key name for Identity Protection Key in chip_kvs file
 * @param[out] ipkBuf - Identity Protection Key buffer
 * @param[out] ipkBufLen - Identity Protection Key buffer length
 * @return CHIP_ERROR_INTERNAL on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR se05x_read_operational_credentials_cluster(char * nocKey, uint8_t * nocBuf, size_t * nocBufLen, char * rootCertKey,
                                                      uint8_t * rootCertBuf, size_t * rootCertBufLen, char * icackey,
                                                      uint8_t * icacBuf, size_t * icacBufLen, char * ipKey, uint8_t * ipkBuf,
                                                      size_t * ipkBufLen);

/**
 * @brief Read node operational key pair in the form of refeence key from SE05x.
 * @param[out] op_key_name - node operational key pair name in chip_kvs file
 * @param[out] op_key_ref_key - node operational key pair in reference key format
 * @param[out] op_key_ref_key_len - node operational reference key pair length
 * @return CHIP_ERROR_INTERNAL on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR se05x_read_node_operational_keypair(char * op_key_name, uint8_t * op_key_ref_key, size_t * op_key_ref_key_len);

/**
 * @brief Read access control list from SE05x.
 * @param[out] acl_key - ACL name in chip_kvs file
 * @param[out] acl - ACL data buffer
 * @param[out] acl_len - ACL data length
 * @return CHIP_ERROR_INTERNAL on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR se05x_read_acl_data(char * acl_key, uint8_t * acl, size_t * acl_len);

/**
 * @brief Read fabric group info from SE05x.
 * @param[out] fgrp_key - Fabric group name in chip_kvs file
 * @param[out] fabgrp_data - Fabric group data buffer
 * @param[out] fabgrp_data_len - Fabric group data length
 * @return CHIP_ERROR_INTERNAL on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR se05x_read_fabric_groups(char * fgrp_key, uint8_t * fabgrp_data, size_t * fabgrp_data_len);

/**
 * @brief Read meta data info from SE05x.
 * @param[out] meta_data_key - meta data name in chip_kvs file
 * @param[out] meta_data - meta data buffer
 * @param[out] meta_data_len - meta data buffer length
 * @return CHIP_ERROR_INTERNAL on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR se05x_read_meta_data(char * meta_data_key, uint8_t * meta_data, size_t * meta_data_len);

/**
 * @brief Read fabric index info info from SE05x.
 * @param[out] fab_index_info_key - fabric index info name in chip_kvs file
 * @param[out] fab_info_data - fabric index info data buffer
 * @param[out] fab_info_data_len - fabric index info data buffer length
 * @return CHIP_ERROR_INTERNAL on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR se05x_read_fabric_index_info_data(char * fab_index_info_key, uint8_t * fab_info_data, size_t * fab_info_data_len);

/* Length of buffer used for key in KVS utilities functions */
#define SE05X_KEY_BUFFER_LEN_KVS_FUNCTIONS 12
