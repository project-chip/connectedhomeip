/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/*
 * This file declares test suite utility methods for discovery.
 *
 * Each method declared in this file needs to be implemented on a per-language
 * basis and allows exposing  methods to the test suites that are not part
 * of the regular cluster set of APIs.
 *
 */

const kTypeArgument = {
  name : 'type',
  type : 'CHAR_STRING',
};

const kNumberValueArgument = {
  name : 'value',
  type : 'INT64U',
};

const kStringValueArgument = {
  name : 'value',
  type : 'CHAR_STRING',
};

const kDefaultResponse = {
  arguments : [
    { name : 'hostName', type : 'CHAR_STRING', chipType : 'chip::CharSpan' }, //
    { name : 'instanceName', type : 'CHAR_STRING', chipType : 'chip::CharSpan' }, //
    { name : 'longDiscriminator', type : 'INT16U', chipType : 'uint16_t' }, //
    { name : 'shortDiscriminator', type : 'INT8U', chipType : 'uint8_t' }, //
    { name : 'vendorId', type : 'INT16U', chipType : 'uint16_t' }, //
    { name : 'productId', type : 'INT16U', chipType : 'uint16_t' }, //
    { name : 'commissioningMode', type : 'INT8U', chipType : 'uint8_t' }, //
    { name : 'deviceType', type : 'INT16U', chipType : 'uint16_t' }, //
    { name : 'deviceName', type : 'CHAR_STRING' }, //
    { name : 'rotatingId', type : 'OCTET_STRING', chipType : 'chip::ByteSpan' }, //
    { name : 'rotatingIdLen', type : 'INT64U', chipType : 'uint64_t' }, //
    { name : 'pairingHint', type : 'INT16U', chipType : 'uint16_t' }, //
    { name : 'pairingInstruction', type : 'CHAR_STRING' }, //
    { name : 'supportsTcp', type : 'BOOLEAN', chipType : 'bool' }, //
    { name : 'numIPs', type : 'INT8U', chipType : 'uint8_t' }, //
    { name : 'port', type : 'INT16U', chipType : 'uint16_t' }, //
    { name : 'mrpRetryIntervalIdle', type : 'INT32U', chipType : 'uint32_t', isOptional : true }, //
    { name : 'mrpRetryIntervalActive', type : 'INT32U', chipType : 'uint32_t', isOptional : true }, //
  ]
};

//
// Commissionable
//

const FindCommissionable = {
  name : 'FindCommissionable',
  arguments : [],
  hasSpecificResponse : true,
  responseName : 'DiscoveryCommandResponse',
  response : kDefaultResponse
};

const FindCommissionableByShortDiscriminator = {
  name : 'FindCommissionableByShortDiscriminator',
  arguments : [ kNumberValueArgument ],
  hasSpecificResponse : true,
  responseName : 'DiscoveryCommandResponse',
  response : kDefaultResponse
};

const FindCommissionableByLongDiscriminator = {
  name : 'FindCommissionableByLongDiscriminator',
  arguments : [ kNumberValueArgument ],
  hasSpecificResponse : true,
  responseName : 'DiscoveryCommandResponse',
  response : kDefaultResponse
};

const FindCommissionableByCompressedFabricId = {
  name : 'FindOperationalByCompressedFabricId',
  arguments : [ kNumberValueArgument ],
  hasSpecificResponse : true,
  responseName : 'DiscoveryCommandResponse',
  response : kDefaultResponse
};

const FindCommissionableByCommissioningMode = {
  name : 'FindCommissionableByCommissioningMode',
  arguments : [],
  hasSpecificResponse : true,
  responseName : 'DiscoveryCommandResponse',
  response : kDefaultResponse
};

const FindCommissionableByVendorId = {
  name : 'FindCommissionableByVendorId',
  arguments : [ kNumberValueArgument ],
  hasSpecificResponse : true,
  responseName : 'DiscoveryCommandResponse',
  response : kDefaultResponse
};

const FindCommissionableByDeviceType = {
  name : 'FindCommissionableByDeviceType',
  arguments : [ kNumberValueArgument ],
  hasSpecificResponse : true,
  responseName : 'DiscoveryCommandResponse',
  response : kDefaultResponse
};

const FindCommissionableByName = {
  name : 'FindCommissionableByName',
  arguments : [ kStringValueArgument ],
  hasSpecificResponse : true,
  responseName : 'DiscoveryCommandResponse',
  response : kDefaultResponse
};

//
// Commissioner
//

const FindCommissioner = {
  name : 'FindCommissioner',
  arguments : [],
  hasSpecificResponse : true,
  responseName : 'DiscoveryCommandResponse',
  response : kDefaultResponse
};

const FindCommissionerByVendorId = {
  name : 'FindCommissionableByVendorId',
  arguments : [ kNumberValueArgument ],
  hasSpecificResponse : true,
  responseName : 'DiscoveryCommandResponse',
  response : kDefaultResponse
};

const FindCommissionerByDeviceType = {
  name : 'FindCommissionerByDeviceType',
  arguments : [ kNumberValueArgument ],
  hasSpecificResponse : true,
  responseName : 'DiscoveryCommandResponse',
  response : kDefaultResponse
};

const commands = [
  // Commissionable
  FindCommissionable,
  FindCommissionableByShortDiscriminator,
  FindCommissionableByLongDiscriminator,
  FindCommissionableByCommissioningMode,
  FindCommissionableByVendorId,
  FindCommissionableByDeviceType,
  FindCommissionableByName,
  // Commissioner
  FindCommissioner,
  FindCommissionerByVendorId,
  FindCommissionerByDeviceType,
];

const DiscoveryCommands = {
  name : 'DiscoveryCommands',
  commands : commands,
};

//
// Module exports
//
exports.cluster = DiscoveryCommands;
