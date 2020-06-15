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
 * @brief CBOR definitions for the Gateway Relay COAP plugin.
 *******************************************************************************
   ******************************************************************************/

// CBOR API
EmberStatus emAfPluginGatewayRelayCoapCborReadInit(uint8_t *buffer,
                                                   uint16_t bufferLength);
EmberStatus emAfPluginGatewayRelayCoapCborGetUnsignedInteger(uint32_t *value,
                                                             uint8_t *size);
EmberStatus emAfPluginGatewayRelayCoapCborGetSignedInteger(int32_t *value,
                                                           uint8_t *size);
EmberStatus emAfPluginGatewayRelayCoapCborWriteInit(uint8_t *buffer,
                                                    uint16_t bufferLength);
EmberStatus emAfPluginGatewayRelayCoapCborGetByteStringDefiniteLength(uint8_t *buffer,
                                                                      uint16_t bufferLength);
EmberStatus emAfPluginGatewayRelayCoapCborGetBooleanValue(bool *value);

uint16_t emAfPluginGatewayRelayCoapCborCurrentWriteDataLength(void);
EmberStatus emAfPluginGatewayRelayCoapCborWriteInt8(int8_t signedData8);
EmberStatus emAfPluginGatewayRelayCoapCborWriteInt16(int16_t signedData16);
EmberStatus emAfPluginGatewayRelayCoapCborWriteInt32(int32_t signedData32);
EmberStatus emAfPluginGatewayRelayCoapCborWriteInt64(int64_t signedData64);
EmberStatus emAfPluginGatewayRelayCoapCborWriteUint8(uint8_t data8);
EmberStatus emAfPluginGatewayRelayCoapCborWriteUint16(uint16_t data16);
EmberStatus emAfPluginGatewayRelayCoapCborWriteUint32(uint32_t data32);
EmberStatus emAfPluginGatewayRelayCoapCborWriteUint64(uint64_t data64);
EmberStatus emAfPluginGatewayRelayCoapCborWriteMapTag(void);
EmberStatus emAfPluginGatewayRelayCoapCborWriteArrayTag(void);
EmberStatus emAfPluginGatewayRelayCoapCborWriteBreak(void);
EmberStatus emAfPluginGatewayRelayCoapCborWriteTrue(void);
EmberStatus emAfPluginGatewayRelayCoapCborWriteFalse(void);
EmberStatus emAfPluginGatewayRelayCoapCborWriteString(char *string);
EmberStatus emAfPluginGatewayRelayCoapCborWriteStringOfLength(const uint8_t *string,
                                                              uint16_t length,
                                                              bool isText);
EmberStatus emAfPluginGatewayRelayCoapCborWriteFloatHalf(uint16_t data16);
EmberStatus emAfPluginGatewayRelayCoapCborWriteFloatSingle(uint32_t data32);
EmberStatus emAfPluginGatewayRelayCoapCborWriteFloatDouble(uint64_t data64);
