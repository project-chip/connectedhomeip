/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

function atomicType(arg)
{
  switch (arg.name) {
  case 'action_id':
  case 'cluster_id':
  case 'command_id':
  case 'event_id':
  case 'fabric_id':
  case 'field_id':
  case 'group_id':
  case 'node_id':
  case 'transaction_id':
  case 'vendor_id':
    return 'chip::' + arg.name.split('_').map(part => part[0].toUpperCase() + part.substring(1)).join('');
  case 'attrib_id':
    return 'chip::AttributeId';
  case 'data_ver':
    return 'chip::DataVersion';
  case 'devtype_id':
    return 'chip::DeviceTypeId';
  case 'endpoint_no':
    return 'chip::EndpointId';
  case 'event_no':
    return 'chip::EventNumber';
  case 'fabric_idx':
    return 'chip::FabricIndex';
  case 'status':
    return 'chip::StatusCode';
  case 'octet_string':
  case 'long_octet_string':
    return 'chip::ByteSpan';
  default:
    throw 'not overriding';
  }
}

exports.atomicType = atomicType
